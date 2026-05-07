# 004_bf16 - Technical Plan

## Architecture

The bf16 library implements a complete software emulation of the bfloat16 floating-point type for CPU inference. The
library is designed as a standalone static library with scalar and SIMD/AVX(2) backends.

### Core Design Principles

- **No type conversion**: Operate directly on bfloat16 bit patterns without converting to float32
- **Dual backend**: Scalar fallback + AVX(2) vectorized operations
- **Bit-level manipulation**: All operations implemented via integer arithmetic on uint16_t
- **IEEE 754 compliance**: Proper handling of special values (zero, inf, NaN), denormals, and rounding

### Component Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                        bf16 Library                              │
│  ┌─────────────────────────────────────────────────────────┐│
│  │                    Public API (bf16.h)                      ││
│  │  - bf16_t type definition                                   ││
│  │  - Arithmetic: add, sub, mul, div                           ││
│  │  - Comparison: eq, ne, lt, le, gt, ge                        ││
│  │  - Special value checks: is_zero, is_inf, is_nan, is_neg    ││
│  │  - Bit manipulation: to_bits, from_bits, get_sign/exp/mant ││
│  └─────────────────────────────────────────────────────────┘│
│                                     │                            │
│         ┌───────────────────────────┼───────────────────────┐  │
│         ▼                           ▼                           ▼  │
│  ┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐  │
│  │  bf16_scalar.c  │    │  bf16_simd.c    │    │  bf16.c (main)  │  │
│  │  Scalar ops     │    │  AVX(2) vector  │    │  Dispatch +     │  │
│  │  (fallback)     │    │  operations      │    │  common helpers │  │
│  └─────────────────┘    └─────────────────┘    └─────────────────┘  │
│         ▲                           ▲                           ▲  │
│         └───────────────────────────┴───────────────────────┘  │
│                              │                                  │
│                    Runtime CPU Feature Detection                │
│                    (Compile-time + Runtime flags)                │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                    Safetensors Parser                           │
│  - Loads bf16 data from .safetensors files                     │
│  - Uses bf16_t type for tensor element representation            │
│  - No modification to safetensors library                      │
└─────────────────────────────────────────────────────────────┘
```

### Data Structures

**Primary Type (bf16.h)**

```c
typedef uint16_t bf16_t;
```

**Internal Representation**

All bfloat16 values are stored as 16-bit unsigned integers with the following bit layout:

```
Bit 15: Sign bit (0 = positive, 1 = negative)
Bits 14-7: Exponent (8 bits, bias = 127)
Bits 6-0: Mantissa (7 bits, implicit leading 1 for normalized)
```

**SIMD Vector Type**

For AVX(2) operations, we use 256-bit registers holding 16 bfloat16 values:

```c
typedef __m256i bf16_vec_t;  // 16 x bf16 packed in 256 bits
```

### Operation Flow

```
Input: bf16_t a, b
       │
       ▼
┌─────────────────┐
│  Runtime Check   │◄── If AVX2 available, use vector path
│  CPU Features    │   (compile-time flag + runtime detection)
└────────┬────────┘
         │
    ┌────┴────┐
    ▼         ▼
┌───────┐ ┌────────┐
│ Scalar │ │ AVX(2) │
│ Path   │ │ Path   │
└───┬───┘ └────┬───┘
    │         │
    └─────────┘
         │
         ▼
┌─────────────────┐
│  Result (bf16_t) │
└─────────────────┘
```

## Technical Requirements

### Dependencies

- C99 standard or later
- Standard library headers: `stdint.h`, `stdbool.h`, `string.h`, `stdlib.h`
- AVX(2) intrinsics: `immintrin.h` (when available)
- No external dependencies

### Compiler Support

- GCC >= 4.9 or Clang >= 3.5 for AVX(2) intrinsics
- CMake build system for conditional compilation

### CPU Assumptions

- AVX(2) instruction set available (detected at runtime)
- No AVX-512-BF16 support assumed
- Fallback to scalar operations on unsupported CPUs

### Build Configuration

The library must support:

- Compile-time AVX(2) enable/disable
- Runtime CPU feature detection
- Static library output

## Expected Folder Structure

```
src/bf16/
├── include/
│   └── bf16.h                  # Public header with type and API
├── bf16.c                      # Main implementation + dispatch
├── bf16_scalar.c               # Scalar operation implementations
├── bf16_simd.c                 # AVX(2) SIMD operation implementations
├── bf16_math.c                 # Mathematical helper functions
└── CMakeLists.txt              # Build configuration

tests/unit/bf16/
├── CMakeLists.txt              # Test build configuration
├── test_bf16_basic.c           # Basic type and bit manipulation tests
├── test_bf16_arithmetic.c      # Arithmetic operation tests
├── test_bf16_comparison.c      # Comparison operation tests
├── test_bf16_special.c          # Special value handling tests
├── test_bf16_simd.c            # SIMD operation tests
└── test_bf16_rounding.c         # Rounding behavior tests
```

## Existing Code Updates

### Files to Modify

1. **src/CMakeLists.txt**
    - Add: `add_subdirectory(bf16)`

### Files to NOT Modify

- **src/core/bf16.h** and **src/core/bf16.c**: Explicitly out of scope per requirements
- **src/safeparser/**: No modifications needed (already handles bf16 file I/O)

## Public API

### Header File: bf16.h

```c
#ifndef CALLM_BF16_H
#define CALLM_BF16_H

#include <stdint.h>
#include <stdbool.h>

// ============================================================================
// TYPE DEFINITION
// ============================================================================

/**
 * bfloat16 floating-point type.
 * 16-bit format: 1 sign bit, 8 exponent bits, 7 mantissa bits.
 * Stored as uint16_t for bit-level manipulation.
 */
typedef uint16_t bf16_t;

// ============================================================================
// SPECIAL VALUES
// ============================================================================

/** Zero (positive) */
#define BF16_ZERO ((bf16_t)0x0000)

/** Zero (negative) */
#define BF16_NEG_ZERO ((bf16_t)0x8000)

/** Positive infinity */
#define BF16_INF ((bf16_t)0x7F80)

/** Negative infinity */
#define BF16_NEG_INF ((bf16_t)0xFF80)

/** Quiet NaN (most common) */
#define BF16_NAN ((bf16_t)0x7FC0)

/** Minimum positive normalized value */
#define BF16_MIN_NORMAL ((bf16_t)0x0080)

/** Maximum finite value */
#define BF16_MAX ((bf16_t)0x7F7F)

/** Minimum positive denormal value */
#define BF16_MIN_DENORMAL ((bf16_t)0x0001)

// ============================================================================
// BIT MANIPULATION
// ============================================================================

/**
 * Returns the raw 16-bit representation of a bfloat16 value.
 *
 * @param a The bfloat16 value.
 * @return The raw bits as uint16_t.
 */
uint16_t bf16_to_bits(bf16_t a);

/**
 * Creates a bfloat16 value from raw bits.
 *
 * @param bits The raw 16-bit pattern.
 * @return The bfloat16 value.
 */
bf16_t bf16_from_bits(uint16_t bits);

/**
 * Extracts the sign bit (bit 15) of a bfloat16 value.
 *
 * @param a The bfloat16 value.
 * @return The sign bit as uint16_t (0 or 1).
 */
uint16_t bf16_get_sign(bf16_t a);

/**
 * Extracts the exponent field (bits 14-7) of a bfloat16 value.
 *
 * @param a The bfloat16 value.
 * @return The exponent field as uint16_t (0-255).
 */
uint16_t bf16_get_exponent(bf16_t a);

/**
 * Extracts the mantissa field (bits 6-0) of a bfloat16 value.
 *
 * @param a The bfloat16 value.
 * @return The mantissa field as uint16_t (0-127).
 */
uint16_t bf16_get_mantissa(bf16_t a);

// ============================================================================
// SPECIAL VALUE CHECKS
// ============================================================================

/**
 * Checks if a bfloat16 value is zero (positive or negative).
 *
 * @param a The bfloat16 value to check.
 * @return true if zero, false otherwise.
 */
bool bf16_is_zero(bf16_t a);

/**
 * Checks if a bfloat16 value is infinity (positive or negative).
 *
 * @param a The bfloat16 value to check.
 * @return true if infinity, false otherwise.
 */
bool bf16_is_inf(bf16_t a);

/**
 * Checks if a bfloat16 value is NaN (Quiet or Signaling).
 *
 * @param a The bfloat16 value to check.
 * @return true if NaN, false otherwise.
 */
bool bf16_is_nan(bf16_t a);

/**
 * Checks if a bfloat16 value is negative.
 *
 * @param a The bfloat16 value to check.
 * @return true if negative (sign bit set), false otherwise.
 */
bool bf16_is_neg(bf16_t a);

// ============================================================================
// ARITHMETIC OPERATIONS
// ============================================================================

/**
 * Adds two bfloat16 values.
 *
 * @param a First operand.
 * @param b Second operand.
 * @return The sum as bfloat16.
 */
bf16_t bf16_add(bf16_t a, bf16_t b);

/**
 * Subtracts two bfloat16 values.
 *
 * @param a First operand (minuend).
 * @param b Second operand (subtrahend).
 * @return The difference as bfloat16.
 */
bf16_t bf16_sub(bf16_t a, bf16_t b);

/**
 * Multiplies two bfloat16 values.
 *
 * @param a First operand.
 * @param b Second operand.
 * @return The product as bfloat16.
 */
bf16_t bf16_mul(bf16_t a, bf16_t b);

/**
 * Divides two bfloat16 values.
 *
 * @param a Dividend.
 * @param b Divisor.
 * @return The quotient as bfloat16. Returns NaN if b is zero.
 */
bf16_t bf16_div(bf16_t a, bf16_t b);

// ============================================================================
// COMPARISON OPERATIONS
// ============================================================================

/**
 * Checks if two bfloat16 values are equal.
 * Note: NaN != NaN, 0 == -0.
 *
 * @param a First operand.
 * @param b Second operand.
 * @return true if equal, false otherwise.
 */
bool bf16_eq(bf16_t a, bf16_t b);

/**
 * Checks if two bfloat16 values are not equal.
 *
 * @param a First operand.
 * @param b Second operand.
 * @return true if not equal, false otherwise.
 */
bool bf16_ne(bf16_t a, bf16_t b);

/**
 * Checks if a < b.
 *
 * @param a First operand.
 * @param b Second operand.
 * @return true if a < b, false otherwise.
 */
bool bf16_lt(bf16_t a, bf16_t b);

/**
 * Checks if a <= b.
 *
 * @param a First operand.
 * @param b Second operand.
 * @return true if a <= b, false otherwise.
 */
bool bf16_le(bf16_t a, bf16_t b);

/**
 * Checks if a > b.
 *
 * @param a First operand.
 * @param b Second operand.
 * @return true if a > b, false otherwise.
 */
bool bf16_gt(bf16_t a, bf16_t b);

/**
 * Checks if a >= b.
 *
 * @param a First operand.
 * @param b Second operand.
 * @return true if a >= b, false otherwise.
 */
bool bf16_ge(bf16_t a, bf16_t b);

// ============================================================================
// CPU FEATURE DETECTION
// ============================================================================

/**
 * Checks if AVX(2) is available at runtime.
 *
 * @return true if AVX(2) is supported, false otherwise.
 */
bool bf16_cpu_has_avx2(void);

/**
 * Returns the SIMD width in elements for the current CPU.
 *
 * @return Number of bf16 elements processed in parallel (1 for scalar, 16 for AVX2).
 */
size_t bf16_simd_width(void);

#endif // CALLM_BF16_H
```

## Performance and Benchmarking

### Performance Goals

| Operation | Scalar Goal (cycles) | AVX(2) Goal (cycles/16 elem) | Target Speedup |
|-----------|----------------------|------------------------------|----------------|
| add/sub   | < 100                | < 200                        | 8-16x          |
| mul       | < 100                | < 200                        | 8-16x          |
| div       | < 200                | < 400                        | 8-16x          |
| cmp       | < 50                 | < 100                        | 8-16x          |

### Benchmarking Strategy

**Benchmark Suite**

A dedicated benchmark program will be created to measure performance:

```
tests/bench/bf16/
├── CMakeLists.txt
├── bench_bf16.c              # Main benchmark program
└── bench_bf16.h              # Benchmark utilities
```

**Benchmark Metrics**

1. **Latency**: Time for single operation (scalar path)
2. **Throughput**: Operations per second (both paths)
3. **Speedup**: AVX(2) vs scalar ratio
4. **Memory**: No dynamic allocation during operations

**Test Cases**

- Random normalized values
- Special values (zero, inf, NaN)
- Denormalized values
- Edge cases (min, max, subnormal)

**Benchmark Categories**

1. **Micro-benchmarks**: Single operation, repeated millions of times
    - Measure raw operation speed
    - Warm-up phase to account for CPU frequency scaling
    - Multiple iterations for statistical significance

2. **Vector benchmarks**: Operations on arrays
    - Measure throughput on contiguous data
    - Test memory bandwidth effects
    - Array sizes: 16, 64, 256, 1024, 65536 elements

3. **Realistic workloads**: Simulated inference patterns
    - Matrix-vector multiplication patterns
    - Accumulation operations
    - Mixed operation sequences

### Implementation Approach for SIMD

**AVX(2) Strategy**

Since AVX(2) doesn't natively support 16-bit integer operations, we use the following approach:

1. **Load**: Use `_mm256_loadu_si256` to load 16 bf16 values (32 bytes) as __m256i
2. **Process**: Unpack to 32-bit integers for computation
3. **Compute**: Perform operations using 32-bit integer arithmetic
4. **Pack**: Re-pack results to 16-bit
5. **Store**: Use `_mm256_storeu_si256` to store results

**Example: Addition**

```c
// Scalar path (bf16_scalar.c)
bf16_t bf16_add_scalar(bf16_t a, bf16_t b) {
    // Extract components
    uint16_t sign_a = (a >> 15) & 0x1;
    uint16_t exp_a = (a >> 7) & 0xFF;
    uint16_t mant_a = a & 0x7F;
    
    uint16_t sign_b = (b >> 15) & 0x1;
    uint16_t exp_b = (b >> 7) & 0xFF;
    uint16_t mant_b = b & 0x7F;
    
    // Handle special cases (zero, inf, NaN)
    if (exp_a == 0xFF && mant_a != 0) return BF16_NAN; // a is NaN
    if (exp_b == 0xFF && mant_b != 0) return BF16_NAN; // b is NaN
    if (exp_a == 0xFF && exp_b == 0xFF) {
        if (sign_a != sign_b) return BF16_NAN; // inf - inf = NaN
        return a; // inf + inf = inf (same sign)
    }
    if (exp_a == 0xFF) return a; // a is inf
    if (exp_b == 0xFF) return b; // b is inf
    
    // Align exponents and add mantissas
    // ... (full IEEE 754 addition algorithm)
}

// AVX(2) path (bf16_simd.c)
void bf16_add_vec(bf16_t *result, const bf16_t *a, const bf16_t *b, size_t count) {
    size_t i = 0;
    
    // Process 16 elements at a time
    for (; i + 16 <= count; i += 16) {
        __m256i va = _mm256_loadu_si256((__m256i *)(a + i));
        __m256i vb = _mm256_loadu_si256((__m256i *)(b + i));
        
        // Unpack to 32-bit for computation
        __m256i va_lo = _mm256_cvtepu16_epi32(_mm256_extracti128_si256(va, 0));
        __m256i va_hi = _mm256_cvtepu16_epi32(_mm256_extracti128_si256(va, 1));
        __m256i vb_lo = _mm256_cvtepu16_epi32(_mm256_extracti128_si256(vb, 0));
        __m256i vb_hi = _mm256_cvtepu16_epi32(_mm256_extracti128_si256(vb, 1));
        
        // Process low and high 128-bit lanes separately
        __m256i vr_lo = bf16_add_packed_epi32(va_lo, vb_lo);
        __m256i vr_hi = bf16_add_packed_epi32(va_hi, vb_hi);
        
        // Pack results back to 16-bit
        __m128i vr_128 = _mm_packus_epi32(vr_lo, vr_hi);
        __m256i vr = _mm256_cvtepu16_epi32(vr_128); // Need proper packing
        
        _mm256_storeu_si256((__m256i *)(result + i), vr);
    }
    
    // Handle remaining elements with scalar path
    for (; i < count; i++) {
        result[i] = bf16_add_scalar(a[i], b[i]);
    }
}
```

**Dispatch Mechanism**

```c
// In bf16.c
static bool use_avx2 = false;
static size_t simd_width = 1;

void bf16_init(void) {
    use_avx2 = bf16_cpu_has_avx2();
    simd_width = use_avx2 ? 16 : 1;
}

bf16_t bf16_add(bf16_t a, bf16_t b) {
    if (use_avx2) {
        // For single element, scalar might be faster due to overhead
        return bf16_add_scalar(a, b);
    }
    return bf16_add_scalar(a, b);
}

// Vector version for multiple elements
void bf16_add_n(bf16_t *result, const bf16_t *a, const bf16_t *b, size_t n) {
    if (use_avx2 && n >= 16) {
        bf16_add_vec(result, a, b, n);
    } else {
        for (size_t i = 0; i < n; i++) {
            result[i] = bf16_add_scalar(a[i], b[i]);
        }
    }
}
```

### Optimization Techniques

1. **Denormal Flush-to-Zero**: For performance, denormalized results can be flushed to zero rather than implementing
   gradual underflow. This trades off minimal accuracy for significant speed improvement.

2. **Rounding Mode**: Use round-to-nearest, ties-to-even as the default for consistent behavior.

3. **Branchless Code**: Minimize branches in hot paths using conditional moves and bit manipulation.

4. **Loop Unrolling**: Manual unrolling of scalar loops for better instruction scheduling.

5. **Alignment**: Ensure memory accesses are 32-byte aligned for optimal AVX(2) performance.

6. **Inline Functions**: Mark all hot-path functions as inline for better optimization.

### Performance Monitoring

**Compiler Flags**

```cmake
# For benchmark builds
if (BUILD_BENCHMARKS)
    add_compile_options(-O3 -march=native -mtune=native)
    add_compile_options(-fno-math-errno)
    add_compile_options(-ffast-math)  # For benchmarking only, not default
endif ()
```

**Runtime CPU Detection**

```c
#include <cpuid.h>

bool bf16_cpu_has_avx2(void) {
    unsigned int eax, ebx, ecx, edx;
    
    // Check AVX2 bit (bit 5 of EBX after CPUID with EAX=7, ECX=0)
    if (__get_cpuid_count(7, 0, &eax, &ebx, &ecx, &edx)) {
        return (ebx & (1 << 5)) != 0;
    }
    
    // Fallback: try to use AVX2 instructions and catch illegal instruction
    // This is platform-specific and may not be portable
    
    return false;
}
```

### Benchmark Results Reporting

Benchmark results will be documented in a performance report:

```
tests/bench/bf16/PERFORMANCE.md
```

Format:

```markdown
# BF16 Library Performance Report

## Environment

- CPU: [Model]
- Architecture: [x86_64/ARM]
- Compiler: [GCC/Clang] [Version]
- Flags: [Optimization flags]
- Date: [YYYY-MM-DD]

## Results

### Scalar Operations (cycles/op)

| Operation | Min | Max | Mean | StdDev |
|-----------|-----|-----|------|--------|
| add       | X   | X   | X    | X      |
| sub       | X   | X   | X    | X      |
| mul       | X   | X   | X    | X      |
| div       | X   | X   | X    | X      |

### AVX(2) Operations (cycles/16 op)

| Operation | Min | Max | Mean | StdDev | Speedup |
|-----------|-----|-----|------|--------|---------|
| add       | X   | X   | X    | X      | Xx      |
| sub       | X   | X   | X    | X      | Xx      |
| mul       | X   | X   | X    | X      | Xx      |
| div       | X   | X   | X    | X      | Xx      |

### Throughput (GOp/s)

| Mode | add | sub | mul | div |
|------|-----|-----|-----|-----|
| Scalar | X | X | X | X |
| AVX(2) | X | X | X | X |

## Analysis

- [Performance observations]
- [Bottlenecks identified]
- [Optimization opportunities]
```

## Testing Strategy

### Test Framework

- Use Unity test framework (already integrated via CPM)
- Tests in `tests/unit/bf16/` directory
- 100% code coverage target for all operations

### Test Categories

1. **Bit Manipulation Tests** (`test_bf16_basic.c`)
    - `bf16_to_bits` / `bf16_from_bits` roundtrip
    - Sign/exponent/mantissa extraction
    - All special value bit patterns

2. **Special Value Tests** (`test_bf16_special.c`)
    - Zero (positive and negative)
    - Infinity (positive and negative)
    - NaN (quiet and signaling)
    - Denormalized numbers
    - Minimum and maximum values

3. **Arithmetic Tests** (`test_bf16_arithmetic.c`)
    - Addition: normal + normal, normal + special, special + special
    - Subtraction: same as addition plus sign handling
    - Multiplication: normal * normal, by zero, by inf
    - Division: normal / normal, by zero, inf / inf, 0 / 0
    - Rounding behavior verification

4. **Comparison Tests** (`test_bf16_comparison.c`)
    - All 6 comparison operators
    - NaN comparison behavior (NaN compares false for all comparisons)
    - Zero equality (-0 == +0)
    - Signed zero ordering

5. **SIMD Tests** (`test_bf16_simd.c`)
    - Vector operations produce same results as scalar
    - Edge cases with non-multiple-of-16 counts
    - Unaligned memory access

6. **Rounding Tests** (`test_bf16_rounding.c`)
    - Round-to-nearest, ties-to-even
    - Overflow and underflow handling
    - Denormal results

7. **CPU Detection Tests** (`test_bf16_cpu.c`)
    - AVX(2) detection accuracy
    - Fallback behavior on unsupported CPUs

### Test Data Generation

- Use known bit patterns for reproducible tests
- Include edge cases from IEEE 754 bfloat16 specification
- Random value testing with property-based checks

### Test Validation

- Compare against reference implementation (Python or known-good C)
- Verify IEEE 754 compliance for all operations
- Cross-validate scalar and SIMD paths

## Makefile Targets

No new top-level Makefile targets needed. The CMake build system will:

1. Build `callm_bf16` static library
2. Build unit tests (when RELEASE_TYPE=DEV)
3. Build benchmarks (when BUILD_BENCHMARKS=ON)

New CMake options:

```cmake
option(BUILD_BF16_BENCHMARKS "Build bf16 benchmark programs" OFF)
option(BF16_ENABLE_AVX2 "Enable AVX(2) support" ON)
```

## Technical Risks and Limitations

### Risks

1. **AVX(2) Implementation Complexity**
    - Risk: 16-bit operations in 256-bit registers require unpacking/packing
    - Mitigation: Use well-tested patterns from existing SIMD libraries
    - Fallback: Scalar path always available

2. **Precision Loss**
    - Risk: Software emulation may accumulate more error than hardware
    - Mitigation: Careful rounding at each operation, test against reference
    - Note: bfloat16 inherently has limited precision (7-bit mantissa)

3. **Performance Gap**
    - Risk: Software emulation may be too slow for practical use
    - Mitigation: Aggressive optimization, AVX(2) vectorization
    - Target: < 10x slower than native float32 operations

4. **CPU Compatibility**
    - Risk: AVX(2) not available on older CPUs (pre-2013)
    - Mitigation: Runtime detection with scalar fallback
    - Note: Project targets "any machine", including older CPUs

5. **Denormal Handling**
    - Risk: Flush-to-zero may cause accuracy issues in some workloads
    - Mitigation: Document behavior, provide option to enable gradual underflow
    - Default: Flush-to-zero for performance

6. **Rounding Consistency**
    - Risk: Different rounding modes produce different results
    - Mitigation: Document and consistently apply round-to-nearest, ties-to-even
    - Test: Verify rounding against IEEE 754 specification

7. **Special Value Propagation**
    - Risk: Incorrect handling of NaN, inf, zero in operations
    - Mitigation: Explicit checks in all operations, comprehensive testing
    - Reference: IEEE 754-2008 floating-point standard

### Limitations

1. **No Type Conversion**
    - Cannot convert between bfloat16 and float32/float64
    - Safetensors parser must provide data in bfloat16 format
    - All computations must be performed in bfloat16

2. **No Fused Operations**
    - No fused multiply-add (FMA) support
    - Each operation performed separately with rounding

3. **No AVX-512-BF16**
    - Cannot use hardware bfloat16 instructions
    - All operations software-emulated

4. **No ARM Support Initially**
    - Focus on x86_64 with AVX(2)
    - ARM NEON support can be added later

5. **Memory Alignment Requirements**
    - AVX(2) operations require 32-byte alignment for optimal performance
    - Unaligned access supported but may have performance penalty

6. **No Exception Handling**
    - No IEEE 754 exception flags (invalid, divide-by-zero, overflow, etc.)
    - Results follow IEEE 754 semantics but don't signal exceptions

## Implementation Steps

### Phase 1: Core Type and Bit Manipulation

1. Create `src/bf16/include/bf16.h` with type definition and declarations
2. Create `src/bf16/CMakeLists.txt` with library configuration
3. Implement bit manipulation functions in `bf16_scalar.c`
4. Implement special value constants and checks
5. Create basic unit tests for bit manipulation

### Phase 2: Scalar Arithmetic

1. Implement addition with proper exponent alignment and rounding
2. Implement subtraction (reuse addition with sign flip)
3. Implement multiplication with exponent addition and mantissa multiplication
4. Implement division with exponent subtraction and mantissa division
5. Add comprehensive arithmetic tests

### Phase 3: Scalar Comparison

1. Implement all 6 comparison operators
2. Handle NaN comparisons (always false)
3. Handle signed zero equality
4. Add comparison tests

### Phase 4: CPU Detection

1. Implement runtime AVX(2) detection
2. Add compile-time AVX(2) enable/disable
3. Create dispatch mechanism for scalar vs SIMD
4. Test CPU detection on various machines

### Phase 5: SIMD Implementation

1. Create `bf16_simd.c` with AVX(2) intrinsics
2. Implement vectorized addition
3. Implement vectorized subtraction
4. Implement vectorized multiplication
5. Implement vectorized division
6. Implement vectorized comparisons
7. Add SIMD-specific tests

### Phase 6: Performance Optimization

1. Profile scalar operations
2. Optimize hot paths (branchless code, inline functions)
3. Profile SIMD operations
4. Optimize unpacking/packing logic
5. Add loop unrolling where beneficial

### Phase 7: Benchmarking

1. Create benchmark infrastructure
2. Implement micro-benchmarks for all operations
3. Implement vector benchmarks
4. Implement realistic workload benchmarks
5. Document baseline performance

### Phase 8: Testing and Validation

1. Run all unit tests
2. Validate against reference implementation
3. Test on CPUs with and without AVX(2)
4. Verify IEEE 754 compliance
5. Achieve 100% code coverage

### Phase 9: Integration

1. Add bf16 to `src/CMakeLists.txt`
2. Verify library builds correctly
3. Verify all tests pass
4. Verify benchmarks run successfully
5. Update project documentation if needed
