# 001_saftensors_parser_cli

<!-- HUMAN-START -->
Create a CLI in C to pase a Safetensors file and display the content structure.
Expectations:

- the tool does not display the matrices, but only the structure
- the expected output is the list of the matrices with their name, shape and dtype
- the tool must allow the user to filter and sort the result by name, shape and dtype
- The tool must use as less memory as possible. The matrices MUST NOT be loaded, only their metadata

<!-- HUMAN-END -->

## Functional specification

### Feature goal

Provide a dedicated command-line tool, written in C, that reads a Safetensors file and prints only tensor metadata
without loading or printing full tensor values.

### Scope

- Input: one Safetensors file path provided by the user.
- Output: a list of tensors, each containing:
    - tensor name
    - tensor shape
    - tensor dtype
- Excluded from output:
    - raw matrix/tensor numerical values
    - heavy data dumps

### Expected CLI behavior

- The CLI must validate that the provided file can be opened and parsed as Safetensors.
- The CLI must parse metadata from the Safetensors header.
- The CLI must list all tensors from the file (except technical metadata entries that are not tensors).
- The CLI must support filtering results by:
    - name
    - shape
    - dtype
- The CLI must support sorting results by:
    - name
    - shape
    - dtype

### Filtering requirements

- Name filtering must use exact full-name match (`--filter-name <NAME>`).
- Dtype filtering must allow selecting tensors by dtype value (for example `F32`, `BF16`).
- Shape filtering must use exact full-shape match with a comma-separated descriptor (`--filter-shape 4096,11008`).
- Multiple filters can be combined; only tensors matching all selected filters are displayed.

### Sorting requirements

- Sorting must be deterministic.
- Sorting must be configurable per supported key (`name`, `shape`, `dtype`).
- Sorting direction is ascending only in this first version.
- If no sort option is provided, default order should remain predictable (stable parser iteration order or explicit
  default sort).

### CLI options and expected `--help` output

- Tool name: `safeparser`
- Output mode: human-readable text only (no machine-readable mode in this first version).
- Supported options:
    - `-h`, `--help`
    - `--filter-name <NAME>`
    - `--filter-shape <D1,D2,...>`
    - `--filter-dtype <DTYPE>`
    - `--sort-by <name|shape|dtype>`

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
  -h, --help                   Show this help message and exit
```

### Error handling requirements

- Missing file path: return non-zero exit code with usage/help message.
- Unreadable/non-existing file: return non-zero exit code with explicit error message.
- Invalid/corrupted Safetensors header: return non-zero exit code with parsing error details.
- Invalid filter/sort argument: return non-zero exit code with a clear explanation.

## Risks and limitations

- Safetensors files can be large; implementation must avoid loading full tensor payloads when only metadata is required.
- Shape-based filtering/sorting may become costly on models with many tensors; clear and simple comparison rules are
  needed.
- Current runtime dtype handling appears focused on `F32` and `BF16`; unsupported or unexpected dtypes should fail
  explicitly.
- JSON/header parsing robustness is critical: malformed metadata should fail safely and predictably.
- CLI UX ambiguity (exact option names and shape filter grammar) can cause inconsistent usage unless standardized early.

## Clarified decisions for this feature

- Name filtering is exact-match only in this first version.
- Shape filtering is exact full-shape only using `D1,D2,...` syntax.
- Sorting supports ascending order only.
- Output format is human-readable text only.