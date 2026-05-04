# Tasks — 001_saftensors_parser_cli

- [x] Create `src/safeparser/` and wire a new `safeparser` executable target into CMake.
    - [x] Add `src/safeparser/CMakeLists.txt` defining the `safeparser` target and linking required libs (`callm_core`,
      `callm_shared`, `jansson`).
    - [x] Update `src/CMakeLists.txt` to add `add_subdirectory(safeparser)`.

- [x] Implement CLI entrypoint and argument contract in `src/safeparser/main.c`.
    - [x] Support `-h` / `--help` and print usage/help text matching the feature spec.
    - [x] Support positional `<FILE>` argument and validate it is provided when not in help mode.
    - [x] Parse options `--filter-name`, `--filter-shape`, `--filter-dtype`, `--sort-by` and return non-zero on invalid
      combinations/values.
    - [x] Keep orchestration in CLI layer only (parse options → parse metadata → filter → sort → print).

- [x] Add query option parsing/validation module.
    - [x] Create `src/safeparser/query_options.h` with `QueryOptions` model and parse/validate API.
    - [x] Create `src/safeparser/query_options.c` to parse CLI args into `QueryOptions`.
    - [x] Validate dtype filter values (v1: `F32`, `BF16`) and reject unsupported dtypes with explicit errors.
    - [x] Validate shape filter grammar (`D1,D2,...`) including malformed/empty/overflow cases.
    - [x] Validate `--sort-by` allowed values (`name`, `shape`, `dtype`) and reject invalid values.

- [x] Expose metadata-only access path in core Safetensors parser (`src/core/safetensors.h/.c`).
    - [x] Add minimal API to iterate/read tensor metadata (name, shape, dtype) from parsed header JSON.
    - [x] Ensure `__metadata__` and other non-tensor entries are skipped by this API.
    - [x] Keep filtering/sorting decisions outside core (no CLI policy in core parser).
    - [x] Keep existing runtime behavior compatible for current `callm` code paths.

- [x] Implement safeparser metadata view layer.
    - [x] Create `src/safeparser/metadata_view.h` defining lightweight tensor metadata record(s).
    - [x] Create `src/safeparser/metadata_view.c` to build metadata list from core API output.
    - [x] Ensure implementation reads header metadata only and never calls `Safetensors_load_matrix`.

- [x] Implement filtering and sorting query engine in CLI layer.
    - [x] Apply exact name filter matching full tensor name.
    - [x] Apply exact dtype filter matching parsed tensor dtype.
    - [x] Apply exact full-shape filter against all dimensions.
    - [x] Combine multiple filters with logical AND.
    - [x] Implement ascending deterministic sort by `name`, `shape`, or `dtype`.
    - [x] Implement tie-breakers:
        - [x] `name`: tie-break by shape, then dtype.
        - [x] `dtype`: tie-break by name, then shape.
        - [x] `shape`: compare rank, then lexicographic dimensions, then name.
    - [x] Define deterministic default order when `--sort-by` is omitted.

- [x] Implement output formatter.
    - [x] Create `src/safeparser/output.h` with formatting/printing API.
    - [x] Create `src/safeparser/output.c` to print one line per tensor including only `name`, `shape`, and `dtype`.
    - [x] Ensure no matrix payload values are printed.

- [x] Implement CLI error handling and exit behavior.
    - [x] Return non-zero with usage/help for missing required file argument.
    - [x] Return non-zero with explicit system error for open/read/mmap failures.
    - [x] Return non-zero with explicit parsing error for invalid/corrupted Safetensors header/JSON.
    - [x] Return non-zero with explicit validation error for unsupported dtype or malformed filter/sort options.
    - [x] Add/adjust shared error codes/messages only if current error surface is insufficient.

- [x] Add safeparser unit test suite and register with CTest.
    - [x] Create `tests/unit/safeparser/CMakeLists.txt` with unit targets and `add_test` registrations.
    - [x] Update `tests/unit/CMakeLists.txt` to include safeparser unit subdirectory.
    - [x] Create `tests/unit/safeparser/test_query_options.c` covering help, missing file, invalid sort, malformed
      shape, and valid option parsing.
    - [x] Create `tests/unit/safeparser/test_filtering.c` covering exact-match name/dtype/shape filters and combined
      filters (AND semantics).
    - [x] Create `tests/unit/safeparser/test_sorting.c` covering deterministic ordering and tie-breakers for all sort
      keys.

- [x] Add CLI integration verification tasks.
    - [x] Run `safeparser` on valid Safetensors fixture(s) and verify expected output lines.
    - [x] Verify error-path behavior (missing file, unreadable path, corrupted header) and non-zero exit codes.

- [ ] Add non-functional behavior verification tasks for memory/parsing path.
    - [x] Verify safeparser path uses file mapping for input access.
    - [x] Verify safeparser metadata listing path never invokes matrix loading routines.
    - [ ] Verify runtime memory usage scales with metadata/header size rather than tensor payload size on representative
      files.

- [ ] Run regression checks for existing parser/runtime behavior.
    - [ ] Re-run relevant existing `tests/unit/core` safetensors-related tests after core API changes.
    - [x] Confirm existing `callm` runtime behavior is unchanged by metadata API additions.