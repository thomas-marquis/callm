#ifndef TYPES_C
#define TYPES_C

#include "types.h"

bf16 float_to_bf16(float f)
{
    uint32_t *f_bits = (uint32_t *)&f;
    uint16_t bf16_bits = (*f_bits >> 16) & 0x8000; // Signe
    bf16_bits |= ((*f_bits >> 13) & 0x7FFF);       // Exposant et mantisse
    bf16 result = {bf16_bits};
    return result;
}

float bf16_to_float(bf16 b)
{
    uint32_t f_bits = (b.value & 0x8000) << 16; // Signe
    f_bits |= (b.value & 0x7FFF) << 13;         // Exposant et mantisse
    float *f = (float *)&f_bits;
    return *f;
}

#endif
