#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

typedef uint16_t bf16_t;

/**
 * Converts a bfloat16 (bf16) number to a float32 (float) number.
 *
 * @param b The bfloat16 number to be converted.
 * @return The corresponding float32 number.
 *
 * This function performs the following steps:
 * 1. Extracts the sign, exponent, and mantissa from the bfloat16 number.
 * 2. Handles special cases:
 *    - If the exponent is zero and the mantissa is zero, the result is zero
 * (positive or negative).
 *    - If the exponent is zero and the mantissa is non-zero, the result is
 * treated as zero (denormalized numbers are not handled).
 *    - If the exponent is maximal (0x1F), the result is either infinity (if
 * mantissa is zero) or NaN (if mantissa is non-zero).
 * 3. Adjusts the exponent and mantissa to fit the float32 format.
 * 4. Constructs the float32 number from the adjusted sign, exponent, and
 * mantissa.
 * 5. Returns the resulting float32 number.
 */
float bf16_to_float (bf16_t b);

#endif
