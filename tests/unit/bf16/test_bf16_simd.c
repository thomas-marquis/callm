#include <callm/bf16.h>
#include <stdlib.h>
#include <unity.h>

void
setUp(void)
{
}
void
tearDown(void)
{
}

void
test_bf16_simd_add(void)
{
    const size_t n = 32;
    bf16_t *a = malloc(n * sizeof(bf16_t));
    bf16_t *b = malloc(n * sizeof(bf16_t));
    bf16_t *out = malloc(n * sizeof(bf16_t));
    bf16_t *ref = malloc(n * sizeof(bf16_t));

    for (size_t i = 0; i < n; i++)
    {
        a[i] = bf16_from_bits(0x3F80);    // 1.0
        b[i] = bf16_from_bits(0x3F00);    // 0.5
        ref[i] = bf16_from_bits(0x3FC0);  // 1.5
    }

    bf16_add_arr(out, a, b, n);

    for (size_t i = 0; i < n; i++)
    {
        TEST_ASSERT_EQUAL_UINT16(bf16_to_bits(ref[i]), bf16_to_bits(out[i]));
    }

    free(a);
    free(b);
    free(out);
    free(ref);
}

void
test_bf16_simd_unaligned(void)
{
    const size_t n = 20;
    bf16_t *a = malloc(n * sizeof(bf16_t));
    bf16_t *b = malloc(n * sizeof(bf16_t));
    bf16_t *out = malloc(n * sizeof(bf16_t));

    for (size_t i = 0; i < n; i++)
    {
        a[i] = bf16_from_bits(0x3F80);
        b[i] = bf16_from_bits(0x3F80);
    }

    // Call with size not multiple of 16
    bf16_add_arr(out, a, b, n);

    for (size_t i = 0; i < n; i++)
    {
        TEST_ASSERT_EQUAL_UINT16(0x4000, bf16_to_bits(out[i]));
    }

    free(a);
    free(b);
    free(out);
}

int
main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_bf16_simd_add);
    RUN_TEST(test_bf16_simd_unaligned);
    return UNITY_END();
}
