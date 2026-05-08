#include "bf16.h"
#include "bf16_internal.h"

#ifdef CALLM_ENABLE_AVX2
#include <immintrin.h>
#include <stdalign.h>

// Expand 8 bf16 values to 8 floats using AVX
// bf16 bit pattern in uint16_t: sign(1)|exponent(8)|mantissa(7)
// float bit pattern in uint32_t: sign(1)|exponent(8)|mantissa(23)
// To convert: zero-extend to 32 bits, then the bf16 bits are in the upper 16 bits

static inline __m256
bf16_vec_to_float_vec(__m128i bf16_vec)
{
    // Zero-extend uint16_t to uint32_t
    __m256i bf16_32 = _mm256_cvtepu16_epi32(bf16_vec);
    // Shift left by 16 to place bf16 bits in float position
    __m256i float_bits = _mm256_slli_epi32(bf16_32, 16);
    // Cast to float
    return _mm256_castsi256_ps(float_bits);
}

static inline __m128i
float_vec_to_bf16_vec(__m256 float_vec)
{
    // Cast float to int32
    __m256i float_bits = _mm256_castps_si256(float_vec);
    // Shift right by 16 to get bf16 bits
    __m256i bf16_32 = _mm256_srli_epi32(float_bits, 16);
    // Pack 32-bit values to 16-bit
    // We need to extract low and high 128-bit lanes and pack them
    __m128i bf16_lo = _mm256_castsi256_si128(bf16_32);
    __m128i bf16_hi = _mm256_extracti128_si256(bf16_32, 1);
    return _mm_packus_epi32(bf16_lo, bf16_hi);
}

// Process 16 bf16 values (32 bytes) using AVX2
// Load 16 bf16 = 256 bits = 2 x 128-bit registers
// Convert each 128-bit register of 8 bf16 to 256-bit register of 8 floats
// Do the arithmetic on floats
// Convert back to bf16

static inline __m256i
bf16_add_vec16(__m256i a, __m256i b)
{
    // Extract low and high 128-bit lanes from each input
    __m128i a_lo = _mm256_castsi256_si128(a);
    __m128i a_hi = _mm256_extracti128_si256(a, 1);
    __m128i b_lo = _mm256_castsi256_si128(b);
    __m128i b_hi = _mm256_extracti128_si256(b, 1);
    
    // Convert bf16 to float
    __m256 fa_lo = bf16_vec_to_float_vec(a_lo);
    __m256 fa_hi = bf16_vec_to_float_vec(a_hi);
    __m256 fb_lo = bf16_vec_to_float_vec(b_lo);
    __m256 fb_hi = bf16_vec_to_float_vec(b_hi);
    
    // Add
    __m256 fr_lo = _mm256_add_ps(fa_lo, fb_lo);
    __m256 fr_hi = _mm256_add_ps(fa_hi, fb_hi);
    
    // Convert back to bf16
    __m128i out_lo = float_vec_to_bf16_vec(fr_lo);
    __m128i out_hi = float_vec_to_bf16_vec(fr_hi);
    
    // Combine into 256-bit result
    return _mm256_setr_m128i(out_lo, out_hi);
}

static inline __m256i
bf16_sub_vec16(__m256i a, __m256i b)
{
    __m128i a_lo = _mm256_castsi256_si128(a);
    __m128i a_hi = _mm256_extracti128_si256(a, 1);
    __m128i b_lo = _mm256_castsi256_si128(b);
    __m128i b_hi = _mm256_extracti128_si256(b, 1);
    
    __m256 fa_lo = bf16_vec_to_float_vec(a_lo);
    __m256 fa_hi = bf16_vec_to_float_vec(a_hi);
    __m256 fb_lo = bf16_vec_to_float_vec(b_lo);
    __m256 fb_hi = bf16_vec_to_float_vec(b_hi);
    
    __m256 fr_lo = _mm256_sub_ps(fa_lo, fb_lo);
    __m256 fr_hi = _mm256_sub_ps(fa_hi, fb_hi);
    
    __m128i out_lo = float_vec_to_bf16_vec(fr_lo);
    __m128i out_hi = float_vec_to_bf16_vec(fr_hi);
    
    return _mm256_setr_m128i(out_lo, out_hi);
}

static inline __m256i
bf16_mul_vec16(__m256i a, __m256i b)
{
    __m128i a_lo = _mm256_castsi256_si128(a);
    __m128i a_hi = _mm256_extracti128_si256(a, 1);
    __m128i b_lo = _mm256_castsi256_si128(b);
    __m128i b_hi = _mm256_extracti128_si256(b, 1);
    
    __m256 fa_lo = bf16_vec_to_float_vec(a_lo);
    __m256 fa_hi = bf16_vec_to_float_vec(a_hi);
    __m256 fb_lo = bf16_vec_to_float_vec(b_lo);
    __m256 fb_hi = bf16_vec_to_float_vec(b_hi);
    
    __m256 fr_lo = _mm256_mul_ps(fa_lo, fb_lo);
    __m256 fr_hi = _mm256_mul_ps(fa_hi, fb_hi);
    
    __m128i out_lo = float_vec_to_bf16_vec(fr_lo);
    __m128i out_hi = float_vec_to_bf16_vec(fr_hi);
    
    return _mm256_setr_m128i(out_lo, out_hi);
}

static inline __m256i
bf16_div_vec16(__m256i a, __m256i b)
{
    __m128i a_lo = _mm256_castsi256_si128(a);
    __m128i a_hi = _mm256_extracti128_si256(a, 1);
    __m128i b_lo = _mm256_castsi256_si128(b);
    __m128i b_hi = _mm256_extracti128_si256(b, 1);
    
    __m256 fa_lo = bf16_vec_to_float_vec(a_lo);
    __m256 fa_hi = bf16_vec_to_float_vec(a_hi);
    __m256 fb_lo = bf16_vec_to_float_vec(b_lo);
    __m256 fb_hi = bf16_vec_to_float_vec(b_hi);
    
    __m256 fr_lo = _mm256_div_ps(fa_lo, fb_lo);
    __m256 fr_hi = _mm256_div_ps(fa_hi, fb_hi);
    
    __m128i out_lo = float_vec_to_bf16_vec(fr_lo);
    __m128i out_hi = float_vec_to_bf16_vec(fr_hi);
    
    return _mm256_setr_m128i(out_lo, out_hi);
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
