# Plan — 001_saftensors_parser_cli

<!-- HUMAN-START -->

- The safeparser CLI must be separated from the safetensors engine itself
- Use a map to navigate to the safetensors file because this file can be extensively large

<!-- HUMAN-END -->

## 1. Architecture

### 1.1 High-level design

Implement a dedicated executable `safeparser` that reads only the Safetensors header metadata and prints a
filtered/sorted
list of tensors.

Planned components:

- **CLI entrypoint** (`safeparser` executable)
    - Parse arguments (`--help`, `--filter-name`, `--filter-shape`, `--filter-dtype`, `--sort-by`, `<FILE>`).
    - Validate arguments and convert filters/sort to internal query options.
    - Orchestrate parse → filter → sort → print flow.
    - Keep all CLI UX/query/output concerns inside `src/safeparser` (no CLI policy logic in the Safetensors engine).
- **Metadata extraction layer** (core-side API)
    - Reuse existing Safetensors header parsing (`Safetensors_new`) and JSON root traversal.
    - Expose only a narrow metadata-read path from core (name/shape/dtype access), not CLI behavior.
    - Extract only tensor metadata entries (name, shape, dtype), skipping `__metadata__`.
    - Never call matrix loading routines.
- **Query engine** (CLI-side or shared utility)
    - Apply exact-match filters for name, dtype, and full shape.
    - Apply deterministic ascending sorting by one key (`name`, `shape`, `dtype`).
- **Text output formatter**
    - Emit one human-readable line per tensor with `name`, `shape`, and `dtype`.

### 1.2 Integration in current project

- Add a new executable target `safeparser` in CMake.
- Link against existing libraries needed for Safetensors parsing (`callm_core`, `callm_shared`, `jansson`).
- Keep `src/core/safetensors.*` focused on parsing/data access primitives; filtering/sorting/formatting remain CLI-side.
- Keep runtime/inference code paths untouched (`callm` executable remains unchanged).

### 1.3 Data flow

1. Parse CLI args into a `QueryOptions` structure.
2. Open and memory-map (`mmap`) file, then parse header JSON via existing Safetensors parser.
3. Build an in-memory list of lightweight tensor metadata records from header entries only.
4. Apply combined filters (logical AND).
5. Sort filtered results according to selected key.
6. Print final list and exit.

## 2. Technical requirements

### 2.1 CLI contract

- Executable name: `safeparser`.
- Required positional argument: `<FILE>`.
- Options:
    - `-h`, `--help`
    - `--filter-name <NAME>`
    - `--filter-shape <D1,D2,...>`
    - `--filter-dtype <DTYPE>`
    - `--sort-by <name|shape|dtype>`
- Missing/invalid arguments must return non-zero and clear error messages.

### 2.2 Metadata-only behavior

- Use header parsing only; tensor payload bytes must not be converted to matrices.
- Do not invoke `Safetensors_load_matrix` in CLI path.
- Use memory mapping for file access in the parsing path to avoid full payload copies for large files.
- Output must include only: tensor name, tensor shape, tensor dtype.

### 2.3 Filtering semantics

- Name: exact full-string match.
- Dtype: exact enum/string match (initially `F32`, `BF16`).
- Shape: exact full-shape match from comma-separated dimensions.
- When multiple filters are provided, all must match.

### 2.4 Sorting semantics

- Ascending only.
- Single active sort key (`name` or `shape` or `dtype`).
- Deterministic tie-breakers to keep output stable:
    - `name` sort: tie-break by shape, then dtype.
    - `dtype` sort: tie-break by name, then shape.
    - `shape` sort: compare rank first, then lexicographic dimensions, then name.
- If `--sort-by` is absent, preserve deterministic parser iteration order (or explicit default `name` sort).

### 2.5 Error handling

- File path missing → usage/help + non-zero exit.
- File cannot be opened/read/mapped → explicit system error + non-zero exit.
- Corrupted/invalid Safetensors header or JSON → parsing error details + non-zero exit.
- Unsupported dtype or malformed filter/sort values → explicit validation error + non-zero exit.

### 2.6 Expected folder structure

- `src/safeparser/`
    - `CMakeLists.txt` (defines the `safeparser` executable target)
    - `main.c` (CLI entrypoint and end-to-end flow)
    - `query_options.h/.c` (argument parsing + filter/sort option validation)
    - `metadata_view.h/.c` (lightweight tensor metadata extraction/representation)
    - `output.h/.c` (human-readable formatter)
- `tests/unit/safeparser/`
    - `CMakeLists.txt` (unit test targets + `add_test` registrations)
    - `test_query_options.c` (argument parsing and validation)
    - `test_sorting.c` (deterministic ordering and tie-breakers)
    - `test_filtering.c` (name/shape/dtype exact-match behavior)

### 2.7 Existing code to update

- `src/CMakeLists.txt`
    - Add `add_subdirectory(safeparser)` to integrate the new CLI target.
- `src/core/safetensors.h` and `src/core/safetensors.c`
    - Add a minimal metadata-iteration/extraction API so the CLI can access tensor name/shape/dtype without calling
      `Safetensors_load_matrix` and without moving CLI filtering/sorting concerns into core.
    - Keep existing runtime paths compatible (no behavior change for `callm`).
- `tests/unit/CMakeLists.txt` and/or `tests/unit/core/CMakeLists.txt`
    - Register the new `safeparser` unit test subdirectory/targets using the current Unity + CTest pattern.
- Optional: shared error catalog (`src/shared/errors.h` + related source)
    - Add explicit CLI-facing validation/parsing error codes/messages only if current error surface is not precise
      enough.

## 3. Technical risks and limitations

- **Current dtype support is narrow**: parser currently recognizes `F32` and `BF16`; other dtypes must fail explicitly
  in v1.
- **Header-driven memory usage still scales with metadata size**: payload is avoided, but very large headers still
  consume memory.
- **Shape parsing edge cases**: malformed strings, empty dims, or integer overflow need strict validation.
- **Deterministic ordering complexity**: shape comparisons need a clear canonical rule to avoid unstable outputs.
- **Current parser API orientation**: existing public API is oriented toward loading/printing; metadata iteration
  helpers may be needed for clean CLI integration.

## 4. Main implementation steps (high-level)

1. **Add CLI target and entrypoint**
    - Create `safeparser` executable and register it in CMake.
    - Implement argument parsing and help text exactly as specified.

2. **Expose metadata extraction path**
    - Reuse Safetensors header parsing and add/adjust API to iterate tensor metadata safely.
    - Ensure non-tensor metadata keys are excluded.

3. **Implement query processing**
    - Parse and validate filter options.
    - Apply exact-match filtering and deterministic sorting.

4. **Implement output and errors**
    - Print human-readable metadata rows (`name`, `shape`, `dtype`) only.
    - Standardize non-zero exits and explicit error messages.

5. **Testing and verification**
    - Execute unit tests for option parsing, filtering, and sorting.
    - Execute CLI integration checks on sample Safetensors files.
    - Confirm payload data is never loaded into matrices during CLI execution.

## 5. Testing strategy

- **Unit tests (fast, deterministic)**
    - Test argument parsing (`--help`, missing file, invalid `--sort-by`, malformed `--filter-shape`).
    - Test filter logic independently (exact name, exact dtype, exact full-shape, combined filters with logical AND).
    - Test sorting comparators and tie-breakers (`name`, `shape`, `dtype`) to guarantee deterministic output.
- **Integration tests (CLI-level behavior)**
    - Run `safeparser` on small valid `.safetensors` fixtures and assert expected output lines.
    - Validate error paths (missing file, unreadable file, corrupted header) and non-zero exit codes.
- **Non-functional memory behavior checks**
    - Validate that CLI metadata listing path does not call matrix loading routines.
    - Validate that the CLI path uses file mapping (no full file read into heap buffers).
    - Use representative files to ensure runtime memory scales with metadata/header, not tensor payload size.
- **Regression scope**
    - Re-run existing Safetensors/core tests to ensure parser API changes do not break current runtime behavior.