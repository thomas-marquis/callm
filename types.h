#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

typedef struct
{
    uint16_t value;
} bf16;

bf16 float_to_bf16(float f);

float bf16_to_float(bf16 b);

#endif
