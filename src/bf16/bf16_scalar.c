#include "bf16_internal.h"
#include "include/callm/bf16.h"

uint16_t
bf16_to_bits(bf16_t a)
{
    return (uint16_t) a;
}

bf16_t
bf16_from_bits(uint16_t bits)
{
    return (bf16_t) bits;
}

uint16_t
bf16_get_sign(bf16_t a)
{
    return (a >> 15) & 0x1;
}

uint16_t
bf16_get_exponent(bf16_t a)
{
    return (a >> 7) & 0xFF;
}

uint16_t
bf16_get_mantissa(bf16_t a)
{
    return a & 0x7F;
}

void
bf16_add_arr_scalar(bf16_t *out, const bf16_t *a, const bf16_t *b, size_t n)
{
    for (size_t i = 0; i < n; i++)
    {
        out[i] = bf16_add_scalar(a[i], b[i]);
    }
}

void
bf16_sub_arr_scalar(bf16_t *out, const bf16_t *a, const bf16_t *b, size_t n)
{
    for (size_t i = 0; i < n; i++)
    {
        out[i] = bf16_sub_scalar(a[i], b[i]);
    }
}

void
bf16_mul_arr_scalar(bf16_t *out, const bf16_t *a, const bf16_t *b, size_t n)
{
    for (size_t i = 0; i < n; i++)
    {
        out[i] = bf16_mul_scalar(a[i], b[i]);
    }
}

void
bf16_div_arr_scalar(bf16_t *out, const bf16_t *a, const bf16_t *b, size_t n)
{
    for (size_t i = 0; i < n; i++)
    {
        out[i] = bf16_div_scalar(a[i], b[i]);
    }
}
