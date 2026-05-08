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
test_bf16_is_special(void)
{
    TEST_ASSERT_TRUE(bf16_is_zero(BF16_ZERO));
    TEST_ASSERT_TRUE(bf16_is_zero(BF16_NEG_ZERO));
    TEST_ASSERT_FALSE(bf16_is_zero(bf16_from_bits(0x0001)));

    TEST_ASSERT_TRUE(bf16_is_inf(BF16_INF));
    TEST_ASSERT_TRUE(bf16_is_inf(BF16_NEG_INF));
    TEST_ASSERT_FALSE(bf16_is_inf(BF16_NAN));

    TEST_ASSERT_TRUE(bf16_is_nan(BF16_NAN));
    TEST_ASSERT_TRUE(bf16_is_nan(bf16_from_bits(0x7F81)));
    TEST_ASSERT_FALSE(bf16_is_nan(BF16_INF));

    TEST_ASSERT_TRUE(bf16_is_neg(BF16_NEG_ZERO));
    TEST_ASSERT_TRUE(bf16_is_neg(BF16_NEG_INF));
    TEST_ASSERT_FALSE(bf16_is_neg(BF16_ZERO));
}

void
test_bf16_add_basic(void)
{
    // 1.0 + 1.0 = 2.0
    // 1.0 in bf16 is 0x3F80
    // 2.0 in bf16 is 0x4000
    bf16_t a = bf16_from_bits(0x3F80);
    bf16_t b = bf16_from_bits(0x3F80);
    bf16_t res = bf16_add(a, b);
    TEST_ASSERT_EQUAL_UINT16(0x4000, bf16_to_bits(res));

    // 1.0 + 0.5 = 1.5
    // 0.5 in bf16 is 0x3F00
    // 1.5 in bf16 is 0x3FC0
    b = bf16_from_bits(0x3F00);
    res = bf16_add(a, b);
    TEST_ASSERT_EQUAL_UINT16(0x3FC0, bf16_to_bits(res));
}

void
test_bf16_add_special(void)
{
    TEST_ASSERT_TRUE(bf16_is_nan(bf16_add(BF16_NAN, bf16_from_bits(0x3F80))));
    TEST_ASSERT_EQUAL_UINT16(BF16_INF, bf16_to_bits(bf16_add(BF16_INF, bf16_from_bits(0x3F80))));
    TEST_ASSERT_TRUE(bf16_is_nan(bf16_add(BF16_INF, BF16_NEG_INF)));
}

void
test_bf16_mul_basic(void)
{
    // 2.0 * 1.5 = 3.0
    // 2.0: 0x4000
    // 1.5: 0x3FC0
    // 3.0: 0x4040
    bf16_t a = bf16_from_bits(0x4000);
    bf16_t b = bf16_from_bits(0x3FC0);
    bf16_t res = bf16_mul(a, b);
    TEST_ASSERT_EQUAL_UINT16(0x4040, bf16_to_bits(res));
}

void
test_bf16_div_basic(void)
{
    // 3.0 / 2.0 = 1.5
    // 3.0: 0x4040
    // 2.0: 0x4000
    // 1.5: 0x3FC0
    bf16_t a = bf16_from_bits(0x4040);
    bf16_t b = bf16_from_bits(0x4000);
    bf16_t res = bf16_div(a, b);
    TEST_ASSERT_EQUAL_UINT16(0x3FC0, bf16_to_bits(res));
}

void
test_bf16_div_special(void)
{
    TEST_ASSERT_EQUAL_UINT16(BF16_INF, bf16_to_bits(bf16_div(bf16_from_bits(0x3F80), BF16_ZERO)));
    TEST_ASSERT_TRUE(bf16_is_nan(bf16_div(BF16_ZERO, BF16_ZERO)));
    TEST_ASSERT_EQUAL_UINT16(BF16_ZERO, bf16_to_bits(bf16_div(bf16_from_bits(0x3F80), BF16_INF)));
}

int
main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_bf16_is_special);
    RUN_TEST(test_bf16_add_basic);
    RUN_TEST(test_bf16_add_special);
    RUN_TEST(test_bf16_mul_basic);
    RUN_TEST(test_bf16_div_basic);
    RUN_TEST(test_bf16_div_special);
    return UNITY_END();
}
