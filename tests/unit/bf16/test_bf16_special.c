#include <callm/bf16.h>
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
test_bf16_nan_propagation(void)
{
    bf16_t nan = BF16_NAN;
    bf16_t normal = bf16_from_bits(0x3F80);

    TEST_ASSERT_TRUE(bf16_is_nan(bf16_add(nan, normal)));
    TEST_ASSERT_TRUE(bf16_is_nan(bf16_mul(nan, normal)));
    TEST_ASSERT_TRUE(bf16_is_nan(bf16_div(normal, nan)));
}

void
test_bf16_inf_behavior(void)
{
    bf16_t inf = BF16_INF;
    bf16_t n_inf = BF16_NEG_INF;

    TEST_ASSERT_EQUAL_UINT16(BF16_INF, bf16_to_bits(bf16_add(inf, inf)));
    TEST_ASSERT_TRUE(bf16_is_nan(bf16_add(inf, n_inf)));
    TEST_ASSERT_EQUAL_UINT16(BF16_INF, bf16_to_bits(bf16_mul(inf, bf16_from_bits(0x3F80))));
    TEST_ASSERT_TRUE(bf16_is_nan(bf16_mul(inf, BF16_ZERO)));
}

int
main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_bf16_nan_propagation);
    RUN_TEST(test_bf16_inf_behavior);
    return UNITY_END();
}
