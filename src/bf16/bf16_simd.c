#include "bf16.h"
#include "bf16_internal.h"

#ifdef CALLM_ENABLE_AVX2
#include <immintrin.h>
#include <stdalign.h>

// Helper: Vectorized bfloat16 addition for 8 elements (128 bits)
static inline __m128i bf16_add_vec8(__m128i a, __m128i b);

// Helper: Vectorized bfloat16 subtraction for 8 elements (128 bits)
static inline __m128i bf16_sub_vec8(__m128i a, __m128i b);

// Helper: Vectorized bfloat16 multiplication for 8 elements (128 bits)
static inline __m128i bf16_mul_vec8(__m128i a, __m128i b);

// Helper: Vectorized bfloat16 division for 8 elements (128 bits)
static inline __m128i bf16_div_vec8(__m128i a, __m128i b);

// Vectorized bfloat16 addition for 16 elements (256 bits)
static inline __m256i
bf16_add_vec16(__m256i a, __m256i b)
{
    __m128i a_lo = _mm256_castsi256_si128(a);
    __m128i a_hi = _mm256_extracti128_si256(a, 1);
    __m128i b_lo = _mm256_castsi256_si128(b);
    __m128i b_hi = _mm256_extracti128_si256(b, 1);

    __m128i out_lo = bf16_add_vec8(a_lo, b_lo);
    __m128i out_hi = bf16_add_vec8(a_hi, b_hi);

    return _mm256_setr_m128i(out_lo, out_hi);
}

// Vectorized bfloat16 subtraction for 16 elements (256 bits)
static inline __m256i
bf16_sub_vec16(__m256i a, __m256i b)
{
    __m128i a_lo = _mm256_castsi256_si128(a);
    __m128i a_hi = _mm256_extracti128_si256(a, 1);
    __m128i b_lo = _mm256_castsi256_si128(b);
    __m128i b_hi = _mm256_extracti128_si256(b, 1);

    __m128i out_lo = bf16_sub_vec8(a_lo, b_lo);
    __m128i out_hi = bf16_sub_vec8(a_hi, b_hi);

    return _mm256_setr_m128i(out_lo, out_hi);
}

// Vectorized bfloat16 multiplication for 16 elements (256 bits)
static inline __m256i
bf16_mul_vec16(__m256i a, __m256i b)
{
    __m128i a_lo = _mm256_castsi256_si128(a);
    __m128i a_hi = _mm256_extracti128_si256(a, 1);
    __m128i b_lo = _mm256_castsi256_si128(b);
    __m128i b_hi = _mm256_extracti128_si256(b, 1);

    __m128i out_lo = bf16_mul_vec8(a_lo, b_lo);
    __m128i out_hi = bf16_mul_vec8(a_hi, b_hi);

    return _mm256_setr_m128i(out_lo, out_hi);
}

// Vectorized bfloat16 division for 16 elements (256 bits)
static inline __m256i
bf16_div_vec16(__m256i a, __m256i b)
{
    __m128i a_lo = _mm256_castsi256_si128(a);
    __m128i a_hi = _mm256_extracti128_si256(a, 1);
    __m128i b_lo = _mm256_castsi256_si128(b);
    __m128i b_hi = _mm256_extracti128_si256(b, 1);

    __m128i out_lo = bf16_div_vec8(a_lo, b_lo);
    __m128i out_hi = bf16_div_vec8(a_hi, b_hi);

    return _mm256_setr_m128i(out_lo, out_hi);
}

// --- 128-bit (8-element) vectorized operations ---
static inline __m128i
bf16_add_vec8(__m128i a, __m128i b)
{
    alignas(16) uint16_t a_lanes[8];
    alignas(16) uint16_t b_lanes[8];
    alignas(16) uint16_t out_lanes[8];

    _mm_store_si128((__m128i *) a_lanes, a);
    _mm_store_si128((__m128i *) b_lanes, b);

    for (int i = 0; i < 8; i++)
        out_lanes[i] = bf16_add_scalar(a_lanes[i], b_lanes[i]);

    return _mm_load_si128((__m128i *) out_lanes);
}

static inline __m128i
bf16_sub_vec8(__m128i a, __m128i b)
{
    alignas(16) uint16_t a_lanes[8];
    alignas(16) uint16_t b_lanes[8];
    alignas(16) uint16_t out_lanes[8];

    _mm_store_si128((__m128i *) a_lanes, a);
    _mm_store_si128((__m128i *) b_lanes, b);

    for (int i = 0; i < 8; i++)
        out_lanes[i] = bf16_sub_scalar(a_lanes[i], b_lanes[i]);

    return _mm_load_si128((__m128i *) out_lanes);
}

static inline __m128i
bf16_mul_vec8(__m128i a, __m128i b)
{
    alignas(16) uint16_t a_lanes[8];
    alignas(16) uint16_t b_lanes[8];
    alignas(16) uint16_t out_lanes[8];

    _mm_store_si128((__m128i *) a_lanes, a);
    _mm_store_si128((__m128i *) b_lanes, b);

    for (int i = 0; i < 8; i++)
        out_lanes[i] = bf16_mul_scalar(a_lanes[i], b_lanes[i]);

    return _mm_load_si128((__m128i *) out_lanes);
}

static inline __m128i
bf16_div_vec8(__m128i a, __m128i b)
{
    alignas(16) uint16_t a_lanes[8];
    alignas(16) uint16_t b_lanes[8];
    alignas(16) uint16_t out_lanes[8];

    _mm_store_si128((__m128i *) a_lanes, a);
    _mm_store_si128((__m128i *) b_lanes, b);

    for (int i = 0; i < 8; i++)
        out_lanes[i] = bf16_div_scalar(a_lanes[i], b_lanes[i]);

    return _mm_load_si128((__m128i *) out_lanes);
}

// --- Array operations ---
void
bf16_add_arr_avx2(bf16_t *out, const bf16_t *a, const bf16_t *b, size_t n)
{
    size_t i = 0;
    for (; i + 15 < n; i += 16)
    {
        __m256i va = _mm256_loadu_si256((const __m256i *) &a[i]);
        __m256i vb = _mm256_loadu_si256((const __m256i *) &b[i]);
        __m256i vr = bf16_add_vec16(va, vb);
        _mm256_storeu_si256((__m256i *) &out[i], vr);
    }
    for (; i < n; i++)
        out[i] = bf16_add_scalar(a[i], b[i]);
}

void
bf16_sub_arr_avx2(bf16_t *out, const bf16_t *a, const bf16_t *b, size_t n)
{
    size_t i = 0;
    for (; i + 15 < n; i += 16)
    {
        __m256i va = _mm256_loadu_si256((const __m256i *) &a[i]);
        __m256i vb = _mm256_loadu_si256((const __m256i *) &b[i]);
        __m256i vr = bf16_sub_vec16(va, vb);
        _mm256_storeu_si256((__m256i *) &out[i], vr);
    }
    for (; i < n; i++)
        out[i] = bf16_sub_scalar(a[i], b[i]);
}

void
bf16_mul_arr_avx2(bf16_t *out, const bf16_t *a, const bf16_t *b, size_t n)
{
    size_t i = 0;
    for (; i + 15 < n; i += 16)
    {
        __m256i va = _mm256_loadu_si256((const __m256i *) &a[i]);
        __m256i vb = _mm256_loadu_si256((const __m256i *) &b[i]);
        __m256i vr = bf16_mul_vec16(va, vb);
        _mm256_storeu_si256((__m256i *) &out[i], vr);
    }
    for (; i < n; i++)
        out[i] = bf16_mul_scalar(a[i], b[i]);
}

void
bf16_div_arr_avx2(bf16_t *out, const bf16_t *a, const bf16_t *b, size_t n)
{
    size_t i = 0;
    for (; i + 15 < n; i += 16)
    {
        __m256i va = _mm256_loadu_si256((const __m256i *) &a[i]);
        __m256i vb = _mm256_loadu_si256((const __m256i *) &b[i]);
        __m256i vr = bf16_div_vec16(va, vb);
        _mm256_storeu_si256((__m256i *) &out[i], vr);
    }
    for (; i < n; i++)
        out[i] = bf16_div_scalar(a[i], b[i]);
}

#endif