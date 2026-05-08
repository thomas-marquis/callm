#ifndef CALLM_BF16_BENCH_H
#define CALLM_BF16_BENCH_H

#include "bf16.h"

// Benchmarking API - exposes internal implementations for direct comparison
// These functions bypass the runtime dispatch mechanism

/**
 * Scalar implementation of bf16 array addition.
 * Always uses scalar operations, regardless of CPU features.
 */
void bf16_add_arr_scalar(bf16_t *out, const bf16_t *a, const bf16_t *b, size_t n);

/**
 * Scalar implementation of bf16 array subtraction.
 */
void bf16_sub_arr_scalar(bf16_t *out, const bf16_t *a, const bf16_t *b, size_t n);

/**
 * Scalar implementation of bf16 array multiplication.
 */
void bf16_mul_arr_scalar(bf16_t *out, const bf16_t *a, const bf16_t *b, size_t n);

/**
 * Scalar implementation of bf16 array division.
 */
void bf16_div_arr_scalar(bf16_t *out, const bf16_t *a, const bf16_t *b, size_t n);

#ifdef CALLM_ENABLE_AVX2
/**
 * AVX2 SIMD implementation of bf16 array addition.
 * Requires AVX2 support.
 */
void bf16_add_arr_avx2(bf16_t *out, const bf16_t *a, const bf16_t *b, size_t n);

/**
 * AVX2 SIMD implementation of bf16 array subtraction.
 */
void bf16_sub_arr_avx2(bf16_t *out, const bf16_t *a, const bf16_t *b, size_t n);

/**
 * AVX2 SIMD implementation of bf16 array multiplication.
 */
void bf16_mul_arr_avx2(bf16_t *out, const bf16_t *a, const bf16_t *b, size_t n);

/**
 * AVX2 SIMD implementation of bf16 array division.
 */
void bf16_div_arr_avx2(bf16_t *out, const bf16_t *a, const bf16_t *b, size_t n);
#endif

#endif  // CALLM_BF16_BENCH_H
