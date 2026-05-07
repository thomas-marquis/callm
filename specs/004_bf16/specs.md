# 004_bf16

<!-- HUMAN-START -->

Bfloat16 (bf16) is not handled by CPU hardware. The goal of this feature is to enable its supports for CPU in an
efficient,
safe and performant way.

Create a new `bf16` static library that implements the `bfloat16` type in C.

[bfloat16](https://en.wikipedia.org/wiki/Bfloat16_floating-point_format) is a 16-bit floating-point format:

- 1 sign bit
- 8 exponent bits (same as float32)
- 7 fraction bits (truncated from float32’s 23 bits)

Requirements:

- We don't want to simply convert bfloat in an existing numeric format (such as float or double). Instead, we want to
  load bfloats and perform computations on them.
- Bfloat16 numbers are stored on a safetensors file. This file is already handled by the `safetensors` library. Don't
  modify it.
- Only focus on implementing the `bfloat16` type, nothing else.
- This type must support SIMD and AVX(2) operations when supported by the CPU.
- We assume the CPU supports AVX(2) instructions but not AVX-512-BF16. So we need to implement a software emulation for
  all operations.
- We don't want to use an existing library that support this type, we want to develop our own.
- Safetensors files contain bfloat16 numbers, no need to convert a float to bfloat or vice versa.

Notes:

- An existing implementation of bf16 already exists in the project under the library `core`. This is an older version
  and it's no longer relevant, don't relie on it, design your own version.

<!-- HUMAN-END -->

## Functional specification

### Feature goal

Create a new standalone static library named `bf16` that provides a native C implementation of the bfloat16
floating-point type, enabling the CaLLM runtime to load and perform computations directly on bfloat16 values without
converting them to other numeric formats.
 
### Scope

- **Included**: A complete bfloat16 type implementation with arithmetic, comparison, and bit manipulation operations.
- **Excluded**:
    - Conversion functions between bfloat16 and float/double (explicitly out of scope per requirements).
    - Integration with any component other than the safetensors parser (which already handles file I/O).
    - Any other numeric type implementations.
    - Matrix or tensor operations (these belong to the linear algebra library).

### bfloat16 type definition

The bfloat16 type is a 16-bit floating-point format with the following layout:

| Bits | Field    | Description                          |
|------|----------|--------------------------------------|
| 15   | Sign     | 1 bit, 0 = positive, 1 = negative    |
| 14-7 | Exponent | 8 bits, bias = 127 (same as float32) |
| 6-0  | Mantissa | 7 bits, implicit leading 1           |

The type must be defined as a C type alias (e.g., `typedef uint16_t bf16_t;`).

### Required operations

The library must provide the following categories of operations on bfloat16 values:

#### Arithmetic operations

- Addition: `bf16_t bf16_add(bf16_t a, bf16_t b)`
- Subtraction: `bf16_t bf16_sub(bf16_t a, bf16_t b)`
- Multiplication: `bf16_t bf16_mul(bf16_t a, bf16_t b)`
- Division: `bf16_t bf16_div(bf16_t a, bf16_t b)`

#### Comparison operations

- Equal: `bool bf16_eq(bf16_t a, bf16_t b)`
- Not equal: `bool bf16_ne(bf16_t a, bf16_t b)`
- Less than: `bool bf16_lt(bf16_t a, bf16_t b)`
- Less than or equal: `bool bf16_le(bf16_t a, bf16_t b)`
- Greater than: `bool bf16_gt(bf16_t a, bf16_t b)`
- Greater than or equal: `bool bf16_ge(bf16_t a, bf16_t b)`

#### Special value handling

- Check for zero: `bool bf16_is_zero(bf16_t a)`
- Check for infinity: `bool bf16_is_inf(bf16_t a)`
- Check for NaN: `bool bf16_is_nan(bf16_t a)`
- Check for negative: `bool bf16_is_neg(bf16_t a)`

#### Bit manipulation and inspection

- Get raw bits: `uint16_t bf16_to_bits(bf16_t a)`
- Create from raw bits: `bf16_t bf16_from_bits(uint16_t bits)`
- Get sign bit: `uint16_t bf16_get_sign(bf16_t a)`
- Get exponent: `uint16_t bf16_get_exponent(bf16_t a)`
- Get mantissa: `uint16_t bf16_get_mantissa(bf16_t a)`

### SIMD and AVX(2) support

All operations must have optimized implementations that leverage SIMD and AVX(2) instructions when available on the CPU.
The implementation must:

- Detect AVX(2) support at compile-time or runtime.
- Provide vectorized versions of arithmetic operations for processing multiple bfloat16 values in parallel.
- Use 256-bit AVX(2) registers to process 16 bfloat16 values simultaneously (16 x 16-bit = 256 bits).
- Fall back to scalar implementations when SIMD/AVX(2) is not available.

### Software emulation

Since AVX-512-BF16 is not assumed to be available, all bfloat16 operations must be implemented via software emulation
using integer arithmetic and bit manipulation. The implementation must:

- Perform all calculations using integer operations on the underlying 16-bit representation.
- Handle special cases (zero, infinity, NaN) correctly according to IEEE 754 floating-point semantics where applicable.
- Maintain consistent rounding behavior (round-to-nearest, ties-to-even recommended).
- Handle denormalized numbers appropriately (flush-to-zero or gradual underflow).

### Library structure

The library must be organized as a standalone static library:

```
src/bf16/
├── include/
│   └── bf16.h          # Public header with type definition and function declarations
├── bf16.c              # Main implementation
├── bf16_scalar.c       # Scalar implementations of operations
├── bf16_simd.c         # SIMD/AVX(2) implementations
└── CMakeLists.txt      # Build configuration
```

### Integration with safetensors

The bf16 library will be used by the safetensors parser to interpret bfloat16 data stored in safetensors files. The
safetensors library already handles:

- File I/O and parsing
- Memory mapping of tensor data
- Tensor metadata extraction

The bf16 library is only responsible for:

- Providing the type definition
- Performing computations on loaded bfloat16 values
- No conversion between bfloat16 and other types is required

### Header file requirements

The public header (`bf16.h`) must:

- Define the `bf16_t` type
- Declare all public functions with full documentation
- Use include guards
- Not expose any private implementation details
- Be self-contained (include only standard C headers it needs)

### Build requirements

The library must:

- Compile as a static library
- Have no external dependencies (only standard C library)
- Be compilable with C99 or later
- Provide appropriate compiler flags for AVX(2) support

## Risks and limitations

- **Performance**: Software emulation of bfloat16 operations may be significantly slower than hardware-accelerated
  implementations. Careful optimization is required to minimize overhead.
- **Precision**: bfloat16 has limited precision (7-bit mantissa). Accumulation of rounding errors across many operations
  may lead to significant accuracy loss. The implementation must be designed to minimize unnecessary rounding steps.
- **SIMD complexity**: Implementing SIMD operations for bfloat16 using AVX(2) requires careful handling of 16-bit
  integer operations in 256-bit registers. AVX(2) natively supports 32-bit integer operations, so 16-bit operations
  require special packing/unpacking logic.
- **Denormalized numbers**: The 7-bit mantissa means denormalized numbers have very limited precision. The
  implementation must decide whether to support gradual underflow or use flush-to-zero for denormalized results.
- **Rounding modes**: Different rounding modes (round-to-nearest, round-toward-zero, etc.) can produce different
  results. The implementation must document and consistently apply its chosen rounding strategy.
- **Special values**: Proper handling of NaN, infinity, and zero requires careful bit pattern management. All edge cases
  must be thoroughly tested.
- **Portability**: AVX(2) is not available on all CPUs. The implementation must gracefully fall back to scalar code on
  unsupported hardware.
- **Underflow/Overflow**: With only 8 exponent bits, bfloat16 values can easily underflow or overflow. The
  implementation must handle these cases correctly according to IEEE 754 semantics.

## Clarified decisions for this feature

- The bf16 library is a separate static library, not part of the existing `core` library.
- The old bf16 implementation in `src/core/bf16.*` is considered obsolete and will not be used as a reference.
- No type conversion functions (bf16 <-> float/double) are required or allowed in this feature.
- The library focuses solely on the bfloat16 type implementation; matrix/tensor operations are out of scope.
- SIMD support targets AVX(2) specifically (256-bit registers).
- Software emulation must be used for all operations since AVX-512-BF16 is not assumed available.
- The safetensors library remains unchanged; it will use the new bf16 type for reading bfloat16 data from files.