# 002_safeparser_csv

<!-- HUMAN-START -->
Update the safeparser CLI to support CSV export
<!-- HUMAN-END -->

## Functional specification

### Feature goal

Extend the `safeparser` CLI with a CSV export mode so tensor metadata can be consumed by scripts, spreadsheets, and
other tooling, while preserving the current human-readable output as the default behavior.

### Scope

- Input: one Safetensors file path provided by the user.
- Metadata source: Safetensors header only (no tensor payload loading for display/export).
- Existing query options remain supported and unchanged:
    - `--filter-name <NAME>`
    - `--filter-shape <D1,D2,...>`
    - `--filter-dtype <DTYPE>`
    - `--sort-by <name|shape|dtype>`
- New capability: CSV output format in addition to the existing human-readable format.

### Output format requirements

- Output mode must be user-selectable with:
    - `--output <text|csv>`
- Default output mode must remain `text` for backward compatibility.
- In `csv` mode:
    - First line must be a header row: `name,shape,dtype`
    - Each following line must represent one tensor metadata entry.
    - The exported rows must reflect the same filtered/sorted result set as `text` mode.

### CSV field representation rules

- `name` column: tensor name as stored in metadata.
- `shape` column: full shape serialized as `D1,D2,...` inside a single CSV field.
- `dtype` column: dtype string currently supported by the parser (`F32`, `BF16`).
- CSV escaping must be correct and deterministic:
    - Any field containing comma, quote, or newline must be wrapped in double quotes.
    - Double quotes inside a field must be escaped by doubling them (`""`).

### Expected CLI behavior

- Current parsing/filtering/sorting logic is reused before output formatting.
- If `--output csv` is selected, output must be printed to stdout in CSV format.
- If `--output text` is selected (or omitted), output must remain the current human-readable format.
- If an unsupported output mode is provided, the command must fail with a clear error and non-zero exit code.

### CLI options and expected `--help` output

Supported options become:

- `-h`, `--help`
- `--filter-name <NAME>`
- `--filter-shape <D1,D2,...>`
- `--filter-dtype <DTYPE>`
- `--sort-by <name|shape|dtype>`
- `--output <text|csv>`

Expected `--help` output:

```text
Usage: safeparser [OPTIONS] <FILE>

Parse a Safetensors file and print tensor metadata only (name, shape, dtype).
Tensor payload values are never loaded into matrices for display.

Arguments:
  <FILE>                       Path to the .safetensors file

Options:
  --filter-name <NAME>         Exact tensor name match
  --filter-shape <D1,D2,...>   Exact shape match, comma-separated dimensions
  --filter-dtype <DTYPE>       Dtype filter (supported: F32, BF16)
  --sort-by <name|shape|dtype> Sort output by a single key (ascending)
  --output <text|csv>          Output format (default: text)
  -h, --help                   Show this help message and exit
```

### Error handling requirements

- Missing file path: return non-zero exit code with usage/help message.
- Unreadable/non-existing file: return non-zero exit code with explicit error message.
- Invalid/corrupted Safetensors header: return non-zero exit code with parsing error details.
- Invalid filter/sort argument: return non-zero exit code with a clear explanation.
- Invalid output mode: return non-zero exit code with supported values in the message.

## Risks and limitations

- Tensor names may include CSV-sensitive characters (comma, quotes, newlines); incorrect escaping would break
  machine-readable output.
- `shape` is a list serialized into one CSV cell; this is easy to parse but still requires quoting rules due to commas.
- CSV export remains metadata-only; no tensor value export is included in this feature.
- Large models with many tensors may produce large CSV output; stdout piping/redirection behavior must remain stable.
- Current dtype coverage in parser/help is limited (`F32`, `BF16`); future dtype expansion will require keeping CSV
  output consistent.

## Clarified decisions for this feature

- CSV export is an output mode of the existing `safeparser` command, not a separate executable.
- CSV output is written to stdout; writing to a dedicated output file path is out of scope for this feature.
- Filtering and sorting semantics are identical in `text` and `csv` modes.