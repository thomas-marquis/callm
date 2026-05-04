# Tasks — 001_saftensors_parser_cli

- [ ] Create `src/safeparser/` and wire a new `safeparser` executable target into CMake.
    - [ ] Add `src/safeparser/CMakeLists.txt` defining the `safeparser` target and linking required libs (`callm_core`,
      `callm_shared`, `jansson`).
    - [ ] Update `src/CMakeLists.txt` to add `add_subdirectory(safeparser)`.

- [ ] Implement CLI entrypoint and argument contract in `src/safeparser/main.c`.
    - [ ] Support `-h` / `--help` and print usage/help text matching the feature spec.
    - [ ] Support positional `<FILE>` argument and validate it is provided when not in help mode.
    - [ ] Parse options `--filter-name`, `--filter-shape`, `--filter-dtype`, `--sort-by` and return non-zero on invalid
      combinations/values.
    - [ ] Keep orchestration in CLI layer only (parse options → parse metadata → filter → sort → print).

- [ ] Add query option parsing/validation module.
    - [ ] Create `src/safeparser/query_options.h` with `QueryOptions` model and parse/validate API.
    - [ ] Create `src/safeparser/query_options.c` to parse CLI args into `QueryOptions`.
    - [ ] Validate dtype filter values (v1: `F32`, `BF16`) and reject unsupported dtypes with explicit errors.
    - [ ] Validate shape filter grammar (`D1,D2,...`) including malformed/empty/overflow cases.
    - [ ] Validate `--sort-by` allowed values (`name`, `shape`, `dtype`) and reject invalid values.

- [ ] Expose metadata-only access path in core Safetensors parser (`src/core/safetensors.h/.c`).
    - [ ] Add minimal API to iterate/read tensor metadata (name, shape, dtype) from parsed header JSON.
    - [ ] Ensure `__metadata__` and other non-tensor entries are skipped by this API.
    - [ ] Keep filtering/sorting decisions outside core (no CLI policy in core parser).
    - [ ] Keep existing runtime behavior compatible for current `callm` code paths.

- [ ] Implement safeparser metadata view layer.
    - [ ] Create `src/safeparser/metadata_view.h` defining lightweight tensor metadata record(s).
    - [ ] Create `src/safeparser/metadata_view.c` to build metadata list from core API output.
    - [ ] Ensure implementation reads header metadata only and never calls `Safetensors_load_matrix`.

- [ ] Implement filtering and sorting query engine in CLI layer.
    - [ ] Apply exact name filter matching full tensor name.
    - [ ] Apply exact dtype filter matching parsed tensor dtype.
    - [ ] Apply exact full-shape filter against all dimensions.
    - [ ] Combine multiple filters with logical AND.
    - [ ] Implement ascending deterministic sort by `name`, `shape`, or `dtype`.
    - [ ] Implement tie-breakers:
        - [ ] `name`: tie-break by shape, then dtype.
        - [ ] `dtype`: tie-break by name, then shape.
        - [ ] `shape`: compare rank, then lexicographic dimensions, then name.
    - [ ] Define deterministic default order when `--sort-by` is omitted.

- [ ] Implement output formatter.
    - [ ] Create `src/safeparser/output.h` with formatting/printing API.
    - [ ] Create `src/safeparser/output.c` to print one line per tensor including only `name`, `shape`, and `dtype`.
    - [ ] Ensure no matrix payload values are printed.

- [ ] Implement CLI error handling and exit behavior.
    - [ ] Return non-zero with usage/help for missing required file argument.
    - [ ] Return non-zero with explicit system error for open/read/mmap failures.
    - [ ] Return non-zero with explicit parsing error for invalid/corrupted Safetensors header/JSON.
    - [ ] Return non-zero with explicit validation error for unsupported dtype or malformed filter/sort options.
    - [ ] Add/adjust shared error codes/messages only if current error surface is insufficient.

- [ ] Add safeparser unit test suite and register with CTest.
    - [ ] Create `tests/unit/safeparser/CMakeLists.txt` with unit targets and `add_test` registrations.
    - [ ] Update `tests/unit/CMakeLists.txt` to include safeparser unit subdirectory.
    - [ ] Create `tests/unit/safeparser/test_query_options.c` covering help, missing file, invalid sort, malformed
      shape, and valid option parsing.
    - [ ] Create `tests/unit/safeparser/test_filtering.c` covering exact-match name/dtype/shape filters and combined
      filters (AND semantics).
    - [ ] Create `tests/unit/safeparser/test_sorting.c` covering deterministic ordering and tie-breakers for all sort
      keys.

- [ ] Add CLI integration verification tasks.
    - [ ] Run `safeparser` on valid Safetensors fixture(s) and verify expected output lines.
    - [ ] Verify error-path behavior (missing file, unreadable path, corrupted header) and non-zero exit codes.

- [ ] Add non-functional behavior verification tasks for memory/parsing path.
    - [ ] Verify safeparser path uses file mapping for input access.
    - [ ] Verify safeparser metadata listing path never invokes matrix loading routines.
    - [ ] Verify runtime memory usage scales with metadata/header size rather than tensor payload size on representative
      files.

- [ ] Run regression checks for existing parser/runtime behavior.
    - [ ] Re-run relevant existing `tests/unit/core` safetensors-related tests after core API changes.
    - [ ] Confirm existing `callm` runtime behavior is unchanged by metadata API additions.