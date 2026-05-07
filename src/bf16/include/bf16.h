#ifndef CALLM_BF16_H
#define CALLM_BF16_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

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
#define BF16_ZERO ((bf16_t) 0x0000)

/** Zero (negative) */
#define BF16_NEG_ZERO ((bf16_t) 0x8000)

/** Positive infinity */
#define BF16_INF ((bf16_t) 0x7F80)

/** Negative infinity */
#define BF16_NEG_INF ((bf16_t) 0xFF80)

/** Quiet NaN (most common) */
#define BF16_NAN ((bf16_t) 0x7FC0)

/** Minimum positive normalized value */
#define BF16_MIN_NORMAL ((bf16_t) 0x0080)

/** Maximum finite value */
#define BF16_MAX ((bf16_t) 0x7F7F)

/** Minimum positive denormal value */
#define BF16_MIN_DENORMAL ((bf16_t) 0x0001)

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
static inline bool
bf16_is_zero(bf16_t a)
{
    return (a & 0x7FFF) == 0;
}

/**
 * Checks if a bfloat16 value is infinity (positive or negative).
 *
 * @param a The bfloat16 value to check.
 * @return true if infinity, false otherwise.
 */
static inline bool
bf16_is_inf(bf16_t a)
{
    return (a & 0x7FFF) == 0x7F80;
}

/**
 * Checks if a bfloat16 value is NaN (Quiet or Signaling).
 *
 * @param a The bfloat16 value to check.
 * @return true if NaN, false otherwise.
 */
static inline bool
bf16_is_nan(bf16_t a)
{
    return (a & 0x7F80) == 0x7F80 && (a & 0x007F) != 0;
}

/**
 * Checks if a bfloat16 value is negative.
 *
 * @param a The bfloat16 value to check.
 * @return true if negative (sign bit set), false otherwise.
 */
static inline bool
bf16_is_neg(bf16_t a)
{
    return (a >> 15) & 0x1;
}

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
// ARRAY OPERATIONS (Vectorized)
// ============================================================================

/**
 * Adds two arrays of bfloat16 values.
 *
 * @param out Output array.
 * @param a First input array.
 * @param b Second input array.
 * @param n Number of elements.
 */
void bf16_add_arr(bf16_t *out, const bf16_t *a, const bf16_t *b, size_t n);

/**
 * Subtracts two arrays of bfloat16 values.
 *
 * @param out Output array.
 * @param a First input array.
 * @param b Second input array.
 * @param n Number of elements.
 */
void bf16_sub_arr(bf16_t *out, const bf16_t *a, const bf16_t *b, size_t n);

/**
 * Multiplies two arrays of bfloat16 values.
 *
 * @param out Output array.
 * @param a First input array.
 * @param b Second input array.
 * @param n Number of elements.
 */
void bf16_mul_arr(bf16_t *out, const bf16_t *a, const bf16_t *b, size_t n);

/**
 * Divides two arrays of bfloat16 values.
 *
 * @param out Output array.
 * @param a First input array.
 * @param b Second input array.
 * @param n Number of elements.
 */
void bf16_div_arr(bf16_t *out, const bf16_t *a, const bf16_t *b, size_t n);

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

#endif  // CALLM_BF16_H
