# 004_bf16 - Tasks List

## Phase 1: Core Type and Bit Manipulation

- [ ] Create directory structure `src/bf16/include/`
- [ ] Create directory structure `src/bf16/`
- [ ] Create `src/bf16/include/bf16.h` with include guards and standard headers
- [ ] Define `bf16_t` type as `typedef uint16_t bf16_t`
- [ ] Define special value constants (BF16_ZERO, BF16_NEG_ZERO, BF16_INF, BF16_NEG_INF, BF16_NAN, BF16_MIN_NORMAL, BF16_MAX, BF16_MIN_DENORMAL)
- [ ] Declare bit manipulation functions (to_bits, from_bits, get_sign, get_exponent, get_mantissa)
- [ ] Declare special value check functions (is_zero, is_inf, is_nan, is_neg)
- [ ] Declare arithmetic operation functions (add, sub, mul, div)
- [ ] Declare comparison operation functions (eq, ne, lt, le, gt, ge)
- [ ] Declare CPU feature detection functions (cpu_has_avx2, simd_width)
- [ ] Add full documentation for all declared functions
- [ ] Create `src/bf16/bf16_scalar.c` with bit manipulation implementations
- [ ] Implement `bf16_to_bits` function
- [ ] Implement `bf16_from_bits` function
- [ ] Implement `bf16_get_sign` function
- [ ] Implement `bf16_get_exponent` function
- [ ] Implement `bf16_get_mantissa` function
- [ ] Create `tests/unit/bf16/` directory structure
- [ ] Create `tests/unit/bf16/CMakeLists.txt`
- [ ] Create `tests/unit/bf16/test_bf16_basic.c` with bit manipulation tests
- [ ] Create `src/bf16/CMakeLists.txt` for the library

## Phase 2: Scalar Arithmetic

- [ ] Implement `bf16_is_zero` function with handling for both positive and negative zero
- [ ] Implement `bf16_is_inf` function with handling for both positive and negative infinity
- [ ] Implement `bf16_is_nan` function for quiet and signaling NaN
- [ ] Implement `bf16_is_neg` function
- [ ] Implement scalar `bf16_add` with exponent alignment and proper rounding
  - [ ] Handle special cases (NaN, infinity, zero)
  - [ ] Implement exponent alignment logic
  - [ ] Implement mantissa addition with carry
  - [ ] Implement normalization of result
  - [ ] Implement rounding (round-to-nearest, ties-to-even)
  - [ ] Handle denormalized numbers (flush-to-zero)
  - [ ] Handle overflow and underflow
- [ ] Implement scalar `bf16_sub` reusing addition logic with sign flip
  - [ ] Handle special cases (NaN, infinity, zero)
  - [ ] Implement sign handling for subtraction
- [ ] Implement scalar `bf16_mul` with exponent addition and mantissa multiplication
  - [ ] Handle special cases (NaN, infinity, zero)
  - [ ] Implement exponent addition with bias adjustment
  - [ ] Implement mantissa multiplication (7-bit x 7-bit)
  - [ ] Implement normalization and rounding
  - [ ] Handle overflow and underflow
- [ ] Implement scalar `bf16_div` with exponent subtraction and mantissa division
  - [ ] Handle special cases (NaN, infinity, zero, division by zero)
  - [ ] Implement exponent subtraction with bias adjustment
  - [ ] Implement mantissa division
  - [ ] Implement normalization and rounding
  - [ ] Handle overflow and underflow
- [ ] Create `tests/unit/bf16/test_bf16_arithmetic.c` with arithmetic tests
  - [ ] Test addition with normal values
  - [ ] Test addition with special values (zero, inf, NaN)
  - [ ] Test subtraction with normal values
  - [ ] Test subtraction with special values
  - [ ] Test multiplication with normal values
  - [ ] Test multiplication by zero and infinity
  - [ ] Test division with normal values
  - [ ] Test division by zero
  - [ ] Test division of infinity
  - [ ] Test rounding behavior

## Phase 3: Scalar Comparison

- [ ] Implement `bf16_eq` with NaN handling (NaN != NaN) and signed zero equality
- [ ] Implement `bf16_ne` function
- [ ] Implement `bf16_lt` function
- [ ] Implement `bf16_le` function
- [ ] Implement `bf16_gt` function
- [ ] Implement `bf16_ge` function
- [ ] Create `tests/unit/bf16/test_bf16_comparison.c` with comparison tests
  - [ ] Test all comparison operators with normal values
  - [ ] Test NaN comparison behavior
  - [ ] Test zero equality (-0 == +0)
  - [ ] Test signed zero ordering
  - [ ] Test infinity comparisons

## Phase 4: CPU Detection

- [ ] Implement runtime AVX(2) detection in `bf16.c`
  - [ ] Use CPUID instruction to check for AVX(2) support
  - [ ] Handle cases where CPUID is not available
- [ ] Add compile-time AVX(2) enable/disable flag
- [ ] Implement `bf16_cpu_has_avx2` function
- [ ] Implement `bf16_simd_width` function
- [ ] Create dispatch mechanism in `bf16.c`
  - [ ] Initialize library with CPU feature detection
  - [ ] Set up function pointers for scalar vs SIMD implementations
- [ ] Create `tests/unit/bf16/test_bf16_cpu.c` with CPU detection tests

## Phase 5: SIMD Implementation

- [ ] Create `src/bf16/bf16_simd.c` with AVX(2) intrinsics
- [ ] Define `bf16_vec_t` type for SIMD operations
- [ ] Implement helper functions for unpacking 16-bit to 32-bit
- [ ] Implement helper functions for packing 32-bit to 16-bit
- [ ] Implement vectorized `bf16_add` for 16 elements
- [ ] Implement vectorized `bf16_sub` for 16 elements
- [ ] Implement vectorized `bf16_mul` for 16 elements
- [ ] Implement vectorized `bf16_div` for 16 elements
- [ ] Implement vectorized comparison operations
- [ ] Handle edge cases with non-multiple-of-16 counts
- [ ] Handle unaligned memory access
- [ ] Create `tests/unit/bf16/test_bf16_simd.c` with SIMD tests
  - [ ] Test vector operations produce same results as scalar
  - [ ] Test edge cases with non-multiple-of-16 counts
  - [ ] Test unaligned memory access

## Phase 6: Performance Optimization

- [ ] Profile scalar operations with benchmarks
- [ ] Optimize hot paths with branchless code
- [ ] Add inline function attributes where appropriate
- [ ] Add manual loop unrolling for scalar loops
- [ ] Optimize unpacking/packing logic in SIMD operations
- [ ] Ensure memory accesses are properly aligned
- [ ] Review and optimize rounding implementation
- [ ] Review and optimize special value handling

## Phase 7: Benchmarking

- [ ] Create `tests/bench/bf16/` directory structure
- [ ] Create `tests/bench/bf16/CMakeLists.txt`
- [ ] Create `tests/bench/bf16/bench_bf16.h` with benchmark utilities
- [ ] Create `tests/bench/bf16/bench_bf16.c` with main benchmark program
- [ ] Implement micro-benchmarks for scalar operations
- [ ] Implement micro-benchmarks for AVX(2) operations
- [ ] Implement vector benchmarks for arrays
- [ ] Implement realistic workload benchmarks
- [ ] Add warm-up phase to benchmarks
- [ ] Add statistical significance to benchmarks
- [ ] Create `tests/bench/bf16/PERFORMANCE.md` template

## Phase 8: Testing and Validation

- [ ] Run all unit tests for bit manipulation
- [ ] Run all unit tests for arithmetic operations
- [ ] Run all unit tests for comparison operations
- [ ] Run all unit tests for special value handling
- [ ] Run all unit tests for SIMD operations
- [ ] Run all unit tests for CPU detection
- [ ] Validate results against reference implementation
- [ ] Test on CPU with AVX(2) support
- [ ] Test on CPU without AVX(2) support (fallback to scalar)
- [ ] Verify IEEE 754 compliance for all operations
- [ ] Verify rounding behavior
- [ ] Achieve 100% code coverage

## Phase 9: Integration

- [ ] Add `add_subdirectory(bf16)` to `src/CMakeLists.txt`
- [ ] Verify library builds correctly
- [ ] Verify all tests pass
- [ ] Verify benchmarks run successfully

## Additional Tasks

- [ ] Create `tests/unit/bf16/test_bf16_special.c` for special value tests
- [ ] Create `tests/unit/bf16/test_bf16_rounding.c` for rounding behavior tests
