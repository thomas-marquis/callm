# Tasks — 002_safeparser_csv

- [x] Extend query option model and parsing for output mode selection.
    - [x] Update `src/safeparser/query_options.h` to add an output-mode enum/type (`text`, `csv`) and store it in
      `QueryOptions`.
    - [x] Initialize output mode default to `text` in `QueryOptions_init` to preserve backward compatibility.
    - [x] Extend `QueryOptions_parse` in `src/safeparser/query_options.c` to parse `--output <text|csv>`.
    - [x] Validate supported output values and return `ERROR` for unsupported values with a clear message listing `text`
      and `csv`.
    - [x] Update `QueryOptions_print_help()` to include `--output <text|csv>` and default behavior.

- [x] Adapt safeparser output API and CLI wiring for mode-based rendering.
    - [x] Update `src/safeparser/output.h` and `src/safeparser/output.c` API so printing receives selected output mode.
    - [x] Keep parse → build → filter → sort flow unchanged in `src/safeparser/main.c` and only switch formatting at
      print time.
    - [x] Pass parsed `options.output_mode` from `main.c` into the output printer.

- [x] Implement CSV formatter and deterministic field escaping in output layer.
    - [x] Keep existing text formatter behavior unchanged for `text` mode.
    - [x] Add CSV mode output with header row `name,shape,dtype`.
    - [x] Serialize shape dimensions as `D1,D2,...` into a single CSV field.
    - [x] Add a shared CSV field writer that quotes fields containing comma/quote/newline and doubles embedded quotes.
    - [x] Ensure CSV formatter preserves all characters and produces deterministic output.

- [x] Extend unit tests for query options output-mode behavior.
    - [x] Update `tests/unit/safeparser/test_query_options.c` to assert default output mode is `text` when `--output` is
      omitted.
    - [x] Add successful parsing cases for `--output text` and `--output csv`.
    - [x] Add failure case for invalid `--output` value and assert parse returns `ERROR`.
    - [x] Ensure existing filter/sort parsing coverage remains passing as regression protection.

- [x] Add dedicated output formatter tests and register them in CMake.
    - [x] Create `tests/unit/safeparser/test_output.c` for direct output formatter validation.
    - [x] Cover text mode regression (existing human-readable line format unchanged).
    - [x] Cover CSV header and row emission for representative tensor metadata.
    - [x] Cover CSV escaping edge cases for commas, double quotes, and newline characters in fields.
    - [x] Cover shape column serialization as one CSV field with comma-separated dimensions.
    - [x] Update `tests/unit/safeparser/CMakeLists.txt` to add executable/`add_test` registration for the new output
      test target.

- [x] Run targeted verification for safeparser CSV feature.
    - [x] Build and run all relevant safeparser unit tests (`query_options`, `filtering`, `sorting`, and new `output`
      tests).
    - [x] Run CLI checks for default/`--output text` behavior to confirm backward-compatible output.
    - [x] Run CLI checks for `--output csv` behavior to confirm machine-readable output and same filtered/sorted result
      set.
    - [x] Run CLI check with invalid `--output` value to confirm clear error and non-zero exit status.