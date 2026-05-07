#include <unity.h>
#include <bf16.h>

void setUp(void) {}
void tearDown(void) {}

void test_bf16_eq(void) {
    TEST_ASSERT_TRUE(bf16_eq(bf16_from_bits(0x3F80), bf16_from_bits(0x3F80)));
    TEST_ASSERT_TRUE(bf16_eq(BF16_ZERO, BF16_NEG_ZERO));
    TEST_ASSERT_FALSE(bf16_eq(BF16_NAN, BF16_NAN));
    TEST_ASSERT_FALSE(bf16_eq(bf16_from_bits(0x3F80), bf16_from_bits(0x4000)));
}

void test_bf16_lt(void) {
    TEST_ASSERT_TRUE(bf16_lt(bf16_from_bits(0x3F80), bf16_from_bits(0x4000))); // 1.0 < 2.0
    TEST_ASSERT_TRUE(bf16_lt(BF16_NEG_INF, BF16_INF));
    TEST_ASSERT_TRUE(bf16_lt(bf16_from_bits(0xC000), bf16_from_bits(0xBF80))); // -2.0 < -1.0
    TEST_ASSERT_FALSE(bf16_lt(BF16_ZERO, BF16_NEG_ZERO));
    TEST_ASSERT_FALSE(bf16_lt(BF16_NAN, bf16_from_bits(0x3F80)));
}

void test_bf16_ordered(void) {
    bf16_t v1 = bf16_from_bits(0x3F00); // 0.5
    bf16_t v2 = bf16_from_bits(0x3F80); // 1.0
    bf16_t v3 = bf16_from_bits(0x4000); // 2.0

    TEST_ASSERT_TRUE(bf16_le(v1, v2));
    TEST_ASSERT_TRUE(bf16_le(v2, v2));
    TEST_ASSERT_TRUE(bf16_gt(v3, v2));
    TEST_ASSERT_TRUE(bf16_ge(v3, v3));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_bf16_eq);
    RUN_TEST(test_bf16_lt);
    RUN_TEST(test_bf16_ordered);
    return UNITY_END();
}
