#ifndef CALLM_BF16_INTERNAL_H
#define CALLM_BF16_INTERNAL_H

#include "bf16.h"

// Scalar implementations
static inline bf16_t
bf16_add_scalar(bf16_t a, bf16_t b)
{
    if (bf16_is_nan(a))
        return BF16_NAN;
    if (bf16_is_nan(b))
        return BF16_NAN;

    if (bf16_is_inf(a))
    {
        if (bf16_is_inf(b) && ((a ^ b) >> 15))
            return BF16_NAN;
        return a;
    }
    if (bf16_is_inf(b))
        return b;

    if (bf16_is_zero(a))
        return b;
    if (bf16_is_zero(b))
        return a;

    uint32_t sign_a = (a >> 15);
    int32_t exp_a = (a >> 7) & 0xFF;
    uint32_t mant_a = (a & 0x7F) | 0x80;

    uint32_t sign_b = (b >> 15);
    int32_t exp_b = (b >> 7) & 0xFF;
    uint32_t mant_b = (b & 0x7F) | 0x80;

    if (exp_a < exp_b || (exp_a == exp_b && mant_a < mant_b))
    {
        uint32_t ts = sign_a;
        sign_a = sign_b;
        sign_b = ts;
        int32_t te = exp_a;
        exp_a = exp_b;
        exp_b = te;
        uint32_t tm = mant_a;
        mant_a = mant_b;
        mant_b = tm;
    }

    int32_t diff = exp_a - exp_b;
    uint32_t m_a = mant_a << 8;
    uint32_t m_b = mant_b << 8;

    if (diff > 15)
        m_b = 0;
    else
        m_b >>= diff;

    uint32_t res_m;
    uint32_t res_s = sign_a;
    int32_t res_e = exp_a;

    if (sign_a == sign_b)
    {
        res_m = m_a + m_b;
        if (res_m & 0x10000)
        {
            res_m >>= 1;
            res_e++;
        }
    }
    else
    {
        res_m = m_a - m_b;
        if (res_m == 0)
            return BF16_ZERO;
        while (!(res_m & 0x8000))
        {
            res_m <<= 1;
            res_e--;
            if (res_e <= 0)
                break;
        }
    }

    if (res_e >= 0xFF)
        return res_s ? BF16_NEG_INF : BF16_INF;
    if (res_e <= 0)
        return res_s ? BF16_NEG_ZERO : BF16_ZERO;

    uint32_t round = res_m & 0xFF;
    res_m >>= 8;
    if (round > 0x80 || (round == 0x80 && (res_m & 1)))
    {
        res_m++;
        if (res_m & 0x100)
        {
            res_m >>= 1;
            res_e++;
        }
    }

    if (res_e >= 0xFF)
        return res_s ? BF16_NEG_INF : BF16_INF;
    return (res_s << 15) | (res_e << 7) | (res_m & 0x7F);
}

static inline bf16_t
bf16_sub_scalar(bf16_t a, bf16_t b)
{
    if (bf16_is_nan(b))
        return BF16_NAN;
    return bf16_add_scalar(a, b ^ 0x8000);
}

static inline bf16_t
bf16_mul_scalar(bf16_t a, bf16_t b)
{
    if (bf16_is_nan(a) || bf16_is_nan(b))
        return BF16_NAN;
    bool res_s = ((a ^ b) >> 15) & 1;
    if (bf16_is_inf(a))
    {
        if (bf16_is_zero(b))
            return BF16_NAN;
        return res_s ? BF16_NEG_INF : BF16_INF;
    }
    if (bf16_is_inf(b))
    {
        if (bf16_is_zero(a))
            return BF16_NAN;
        return res_s ? BF16_NEG_INF : BF16_INF;
    }
    if (bf16_is_zero(a) || bf16_is_zero(b))
        return res_s ? BF16_NEG_ZERO : BF16_ZERO;
    int32_t exp_a = (a >> 7) & 0xFF;
    uint32_t mant_a = (a & 0x7F) | 0x80;
    int32_t exp_b = (b >> 7) & 0xFF;
    uint32_t mant_b = (b & 0x7F) | 0x80;
    int32_t res_e = exp_a + exp_b - 127;
    uint32_t res_m = mant_a * mant_b;
    if (res_m & 0x8000)
    {
        uint32_t round = res_m & 0xFF;
        res_m >>= 8;
        if (round > 0x80 || (round == 0x80 && (res_m & 1)))
            res_m++;
        res_e++;
    }
    else
    {
        uint32_t round = res_m & 0x7F;
        res_m >>= 7;
        if (round > 0x40 || (round == 0x40 && (res_m & 1)))
            res_m++;
    }
    if (res_m & 0x100)
    {
        res_m >>= 1;
        res_e++;
    }
    if (res_e >= 0xFF)
        return res_s ? BF16_NEG_INF : BF16_INF;
    if (res_e <= 0)
        return res_s ? BF16_NEG_ZERO : BF16_ZERO;
    return (res_s << 15) | (res_e << 7) | (res_m & 0x7F);
}

static inline bf16_t
bf16_div_scalar(bf16_t a, bf16_t b)
{
    if (bf16_is_nan(a) || bf16_is_nan(b))
        return BF16_NAN;
    bool res_s = ((a ^ b) >> 15) & 1;
    if (bf16_is_inf(a))
    {
        if (bf16_is_inf(b))
            return BF16_NAN;
        return res_s ? BF16_NEG_INF : BF16_INF;
    }
    if (bf16_is_inf(b))
        return res_s ? BF16_NEG_ZERO : BF16_ZERO;
    if (bf16_is_zero(b))
    {
        if (bf16_is_zero(a))
            return BF16_NAN;
        return res_s ? BF16_NEG_INF : BF16_INF;
    }
    if (bf16_is_zero(a))
        return res_s ? BF16_NEG_ZERO : BF16_ZERO;
    int32_t exp_a = (a >> 7) & 0xFF;
    uint32_t mant_a = (a & 0x7F) | 0x80;
    int32_t exp_b = (b >> 7) & 0xFF;
    uint32_t mant_b = (b & 0x7F) | 0x80;
    int32_t res_e = exp_a - exp_b + 127;
    uint32_t res_m = (mant_a << 15) / mant_b;
    if (res_m & 0x8000)
    {
        uint32_t round = res_m & 0xFF;
        res_m >>= 8;
        if (round > 0x80 || (round == 0x80 && (res_m & 1)))
            res_m++;
    }
    else
    {
        res_e--;
        uint32_t round = res_m & 0x7F;
        res_m >>= 7;
        if (round > 0x40 || (round == 0x40 && (res_m & 1)))
            res_m++;
    }
    if (res_m & 0x100)
    {
        res_m >>= 1;
        res_e++;
    }
    if (res_e >= 0xFF)
        return res_s ? BF16_NEG_INF : BF16_INF;
    if (res_e <= 0)
        return res_s ? BF16_NEG_ZERO : BF16_ZERO;
    return (res_s << 15) | (res_e << 7) | (res_m & 0x7F);
}

static inline bool
bf16_eq_scalar(bf16_t a, bf16_t b)
{
    if (bf16_is_nan(a) || bf16_is_nan(b))
        return false;
    if (bf16_is_zero(a) && bf16_is_zero(b))
        return true;
    return a == b;
}

static inline bool
bf16_ne_scalar(bf16_t a, bf16_t b)
{
    return !bf16_eq_scalar(a, b);
}

static inline bool
bf16_lt_scalar(bf16_t a, bf16_t b)
{
    if (bf16_is_nan(a) || bf16_is_nan(b))
        return false;
    if (bf16_is_zero(a) && bf16_is_zero(b))
        return false;
    bool sign_a = bf16_is_neg(a);
    bool sign_b = bf16_is_neg(b);
    if (sign_a != sign_b)
        return sign_a;
    if (sign_a)
        return (a & 0x7FFF) > (b & 0x7FFF);
    else
        return (a & 0x7FFF) < (b & 0x7FFF);
}

static inline bool
bf16_le_scalar(bf16_t a, bf16_t b)
{
    return bf16_eq_scalar(a, b) || bf16_lt_scalar(a, b);
}

static inline bool
bf16_gt_scalar(bf16_t a, bf16_t b)
{
    return bf16_lt_scalar(b, a);
}

static inline bool
bf16_ge_scalar(bf16_t a, bf16_t b)
{
    return bf16_eq_scalar(a, b) || bf16_gt_scalar(a, b);
}

// Vectorized implementations (Internal) - declared in bf16_bench.h for benchmarking
// These are defined in bf16_scalar.c and bf16_simd.c

#endif  // CALLM_BF16_INTERNAL_H
