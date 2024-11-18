#ifndef TYPES_C
#define TYPES_C

#include "types.h"
#include <math.h>
#include <stdint.h>
#include <string.h>

float bf16_to_float(bf16_t b)
{
    // Extraction of bfloat16 components
    bf16_t sign = (b >> 15) & 0x1;     // 1 bit for the sign      0000 0001
    bf16_t exponent = (b >> 7) & 0xFF; // 8 bits for the exponent 1111 1111
    bf16_t mantissa = b & 0x7F;        // 7 bits for the mantissa 0111 1111

    // If the exponent is zero, we have a zero or a denormalized number
    if (exponent == 0)
    {
        if (mantissa == 0)
        {
            // Zero number (positive or negative)
            return sign ? -0.0f : 0.0f;
        }
        else
        {
            // Denormalized number (not handled here for simplicity)
            return 0.0f;
        }
    }
    else if (exponent == 0x1F)
    {
        // Maximal exponent case: infinity or NaN
        if (mantissa == 0)
        {
            return sign ? -INFINITY : INFINITY;
        }
        else
        {
            return NAN; // NaN
        }
    }

    // Calculate the new exponent for float32
    uint32_t new_sign = sign << 31;         // Sign on bit 31
    uint32_t new_exponent = exponent << 23; // Shifted exponent
    uint32_t new_mantissa = mantissa << 16; // Extended mantissa

    // Construct the integer representing the float32
    uint32_t float_bits = new_sign | new_exponent | new_mantissa;

    // Interpret the number as a float
    float result;
    memcpy(&result, &float_bits, sizeof(result));

    return result;
}

#endif
