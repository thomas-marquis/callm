# Tasks — 002_safeparser_csv

- [ ] Extend query option model and parsing for output mode selection.
    - [ ] Update `src/safeparser/query_options.h` to add an output-mode enum/type (`text`, `csv`) and store it in
      `QueryOptions`.
    - [ ] Initialize output mode default to `text` in `QueryOptions_init` to preserve backward compatibility.
    - [ ] Extend `QueryOptions_parse` in `src/safeparser/query_options.c` to parse `--output <text|csv>`.
    - [ ] Validate supported output values and return `ERROR` for unsupported values with a clear message listing `text`
      and `csv`.
    - [ ] Update `QueryOptions_print_help()` to include `--output <text|csv>` and default behavior.

- [ ] Adapt safeparser output API and CLI wiring for mode-based rendering.
    - [ ] Update `src/safeparser/output.h` and `src/safeparser/output.c` API so printing receives selected output mode.
    - [ ] Keep parse → build → filter → sort flow unchanged in `src/safeparser/main.c` and only switch formatting at
      print time.
    - [ ] Pass parsed `options.output_mode` from `main.c` into the output printer.

- [ ] Implement CSV formatter and deterministic field escaping in output layer.
    - [ ] Keep existing text formatter behavior unchanged for `text` mode.
    - [ ] Add CSV mode output with header row `name,shape,dtype`.
    - [ ] Serialize shape dimensions as `D1,D2,...` into a single CSV field.
    - [ ] Add a shared CSV field writer that quotes fields containing comma/quote/newline and doubles embedded quotes.
    - [ ] Ensure CSV formatter preserves all characters and produces deterministic output.

- [ ] Extend unit tests for query options output-mode behavior.
    - [ ] Update `tests/unit/safeparser/test_query_options.c` to assert default output mode is `text` when `--output` is
      omitted.
    - [ ] Add successful parsing cases for `--output text` and `--output csv`.
    - [ ] Add failure case for invalid `--output` value and assert parse returns `ERROR`.
    - [ ] Ensure existing filter/sort parsing coverage remains passing as regression protection.

- [ ] Add dedicated output formatter tests and register them in CMake.
    - [ ] Create `tests/unit/safeparser/test_output.c` for direct output formatter validation.
    - [ ] Cover text mode regression (existing human-readable line format unchanged).
    - [ ] Cover CSV header and row emission for representative tensor metadata.
    - [ ] Cover CSV escaping edge cases for commas, double quotes, and newline characters in fields.
    - [ ] Cover shape column serialization as one CSV field with comma-separated dimensions.
    - [ ] Update `tests/unit/safeparser/CMakeLists.txt` to add executable/`add_test` registration for the new output
      test target.

- [ ] Run targeted verification for safeparser CSV feature.
    - [ ] Build and run all relevant safeparser unit tests (`query_options`, `filtering`, `sorting`, and new `output`
      tests).
    - [ ] Run CLI checks for default/`--output text` behavior to confirm backward-compatible output.
    - [ ] Run CLI checks for `--output csv` behavior to confirm machine-readable output and same filtered/sorted result
      set.
    - [ ] Run CLI check with invalid `--output` value to confirm clear error and non-zero exit status.