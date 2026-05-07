#include "bf16.h"
#include "bf16_internal.h"

#ifdef __x86_64__
#include <cpuid.h>
#endif

bool
bf16_cpu_has_avx2(void)
{
#ifdef __x86_64__
    unsigned int eax, ebx, ecx, edx;
    if (__get_cpuid(1, &eax, &ebx, &ecx, &edx))
    {
        // Check for AVX (bit 28 of ecx)
        if (ecx & (1 << 28))
        {
            // Now check for AVX2 (bit 5 of ebx for leaf 7)
            if (__get_cpuid_count(7, 0, &eax, &ebx, &ecx, &edx))
            {
                return (ebx & (1 << 5)) != 0;
            }
        }
    }
#endif
    return false;
}

size_t
bf16_simd_width(void)
{
    if (bf16_cpu_has_avx2())
    {
        return 16;
    }
    return 1;
}

// Scalar operations

bf16_t
bf16_add(bf16_t a, bf16_t b)
{
    return bf16_add_scalar(a, b);
}

bf16_t
bf16_sub(bf16_t a, bf16_t b)
{
    return bf16_sub_scalar(a, b);
}

bf16_t
bf16_mul(bf16_t a, bf16_t b)
{
    return bf16_mul_scalar(a, b);
}

bf16_t
bf16_div(bf16_t a, bf16_t b)
{
    return bf16_div_scalar(a, b);
}

bool
bf16_eq(bf16_t a, bf16_t b)
{
    return bf16_eq_scalar(a, b);
}

bool
bf16_ne(bf16_t a, bf16_t b)
{
    return bf16_ne_scalar(a, b);
}

bool
bf16_lt(bf16_t a, bf16_t b)
{
    return bf16_lt_scalar(a, b);
}

bool
bf16_le(bf16_t a, bf16_t b)
{
    return bf16_le_scalar(a, b);
}

bool
bf16_gt(bf16_t a, bf16_t b)
{
    return bf16_gt_scalar(a, b);
}

bool
bf16_ge(bf16_t a, bf16_t b)
{
    return bf16_ge_scalar(a, b);
}

// Array operations with dispatching

void
bf16_add_arr(bf16_t *out, const bf16_t *a, const bf16_t *b, size_t n)
{
#ifdef CALLM_ENABLE_AVX2
    if (bf16_cpu_has_avx2())
    {
        bf16_add_arr_avx2(out, a, b, n);
        return;
    }
#endif
    bf16_add_arr_scalar(out, a, b, n);
}

void
bf16_sub_arr(bf16_t *out, const bf16_t *a, const bf16_t *b, size_t n)
{
#ifdef CALLM_ENABLE_AVX2
    if (bf16_cpu_has_avx2())
    {
        bf16_sub_arr_avx2(out, a, b, n);
        return;
    }
#endif
    bf16_sub_arr_scalar(out, a, b, n);
}

void
bf16_mul_arr(bf16_t *out, const bf16_t *a, const bf16_t *b, size_t n)
{
#ifdef CALLM_ENABLE_AVX2
    if (bf16_cpu_has_avx2())
    {
        bf16_mul_arr_avx2(out, a, b, n);
        return;
    }
#endif
    bf16_mul_arr_scalar(out, a, b, n);
}

void
bf16_div_arr(bf16_t *out, const bf16_t *a, const bf16_t *b, size_t n)
{
#ifdef CALLM_ENABLE_AVX2
    if (bf16_cpu_has_avx2())
    {
        bf16_div_arr_avx2(out, a, b, n);
        return;
    }
#endif
    bf16_div_arr_scalar(out, a, b, n);
}
