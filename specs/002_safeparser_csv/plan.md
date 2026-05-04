# Plan — 002_safeparser_csv

<!-- HUMAN-START -->

<!-- HUMAN-END -->

## 1. Architecture

### 1.1 High-level design

Extend the existing `safeparser` CLI flow by introducing an explicit output mode in query options, while keeping
filtering/sorting behavior unchanged and shared across formats.

Planned components:

- **CLI/query options layer** (`src/safeparser/query_options.h/.c`)
    - Add `--output <text|csv>` parsing and validation.
    - Keep default mode as `text` for backward compatibility.
    - Surface clear errors for unsupported output values.
- **Metadata processing layer** (`src/safeparser/metadata_view.h/.c`)
    - Reuse current metadata build/filter/sort pipeline without behavior change.
    - Keep output formatting concerns out of this layer.
- **Output formatting layer** (`src/safeparser/output.h/.c`)
    - Preserve existing human-readable formatter for `text` mode.
    - Add CSV formatter for `csv` mode with deterministic escaping/serialization rules.
- **CLI orchestration** (`src/safeparser/main.c`)
    - Continue parse → build → filter → sort sequence.
    - Select output formatter only at the final print step based on parsed output mode.

### 1.2 Data flow

1. Parse CLI args into `QueryOptions` (including `output_mode`, defaulting to `text`).
2. Load Safetensors header metadata using existing core parser path.
3. Build `MetadataView`, apply filters, and sort using current logic.
4. Dispatch to output formatter:
    - `text`: existing line format (`name=... shape=[...] dtype=...`).
    - `csv`: header + one CSV row per tensor metadata record.
5. Exit with non-zero code on any parsing/output/IO validation error.

### 1.3 Integration in current project

- No new executable is introduced; feature extends existing `safeparser` target only.
- No core Safetensors API changes are required for this feature scope.
- Most code changes remain isolated to `src/safeparser` and `tests/unit/safeparser`.

## 2. Technical requirements

### 2.1 CLI contract updates

- Support option: `--output <text|csv>`.
- Default when omitted: `text`.
- `--help` text must list `--output <text|csv>` and mention default behavior.
- Invalid output value must fail with clear supported values and non-zero exit code.

### 2.2 Output semantics

- **Result-set parity requirement**:
    - Both `text` and `csv` outputs must operate on the exact same filtered and sorted `MetadataView`.
    - Output mode affects rendering only.
- **CSV mode**:
    - First line: `name,shape,dtype`.
    - `shape` serialized as `D1,D2,...` inside a single CSV field.
    - `dtype` string remains consistent with current parser mapping (`F32`, `BF16`).

### 2.3 CSV escaping rules

Implement a dedicated field writer used by all CSV columns:

- If a field contains comma, quote, or newline, wrap field in double quotes.
- Escape embedded quotes by doubling (`""`).
- Preserve all characters (no trimming/transformation).
- Produce deterministic output for identical input.

### 2.4 Error handling requirements

- Keep current existing error behavior for missing file, unreadable path, and invalid filter/sort values.
- Add output-mode validation error path:
    - `Invalid output mode: '<value>' (supported: text, csv)` (or equivalent clear wording).
- Preserve non-zero exit on all invalid argument/data paths.

### 2.5 Expected folder/module touchpoints

- `src/safeparser/query_options.h`
    - Add output-mode enum/type and field in `QueryOptions`.
- `src/safeparser/query_options.c`
    - Initialize default output mode.
    - Parse `--output` value and validate accepted modes.
    - Extend `QueryOptions_print_help()` output.
- `src/safeparser/output.h`
    - Update output API signature to receive selected mode.
- `src/safeparser/output.c`
    - Keep text formatter logic.
    - Add CSV formatter and escaping helper(s).
- `src/safeparser/main.c`
    - Pass selected output mode into output printer.
- `tests/unit/safeparser/test_query_options.c`
    - Add coverage for default output, valid csv value, invalid output value.
- `tests/unit/safeparser/test_output.c` (new)
    - Add direct formatter tests for CSV header, shape serialization, and escaping edge cases.
- `tests/unit/safeparser/CMakeLists.txt`
    - Register new output-format test target (if new test file is added).

## 3. Testing strategy

### 3.1 Unit tests

- **Query options tests**:
    - `--output` omitted defaults to `text`.
    - `--output text` and `--output csv` parse successfully.
    - Invalid output value returns `ERROR`.
    - Existing filter/sort parsing tests remain passing (regression).
- **Output formatter tests**:
    - Text mode output remains unchanged.
    - CSV emits header plus expected rows.
    - CSV escaping for commas, quotes, and newline characters.
    - Shape field emitted as one CSV field containing comma-separated dimensions.

### 3.2 CLI-level verification

- Build `safeparser` and run with representative fixtures:
    - `--output text` (or omitted) to verify backward-compatible output.
    - `--output csv` to verify machine-readable output.
    - invalid `--output` value to verify non-zero exit and clear error.
- Confirm filtered/sorted set parity by comparing selected rows between `text` and `csv` modes.

### 3.3 Regression scope

- Re-run all `tests/unit/safeparser` tests (query options, filtering, sorting, and output tests).
- Ensure no behavior change in filtering/sorting logic due to output-mode addition.

## 4. Technical risks and limitations

- **CSV correctness risk**: incorrect escaping would break downstream parsing; dedicated edge-case tests are mandatory.
- **Shape formatting ambiguity**: shape includes commas internally, so always writing it as a CSV field is required to
  avoid column drift.
- **Future dtype expansion**: formatter currently mirrors existing dtype strings (`F32`, `BF16`); new dtypes will
  require
  extending both parsing/help and output tests.
- **No output-file option in scope**: output remains stdout-only as specified.

## 5. Main implementation steps (high-level)

1. **Extend query options with output mode**
    - Add enum/field/default and argument parsing for `--output`.
    - Update help text.

2. **Adapt output API and dispatch**
    - Pass selected mode from `main.c` to output layer.
    - Keep existing text formatter intact and add CSV formatter.

3. **Implement CSV writer with escaping**
    - Add helper(s) to quote/escape fields deterministically.
    - Render `name`, serialized `shape`, and `dtype` columns.

4. **Add/extend tests**
    - Extend query option tests for output mode.
    - Add output formatter tests for CSV and escaping edge cases.

5. **Run targeted verification**
    - Build and run all relevant `safeparser` unit tests.
    - Run `safeparser` CLI checks for text/csv parity and invalid mode errors.