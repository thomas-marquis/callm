#include <unity.h>
#include <bf16.h>

void setUp(void) {}
void tearDown(void) {}

void test_bf16_bits_conversion(void) {
    uint16_t bits = 0x1234;
    bf16_t a = bf16_from_bits(bits);
    TEST_ASSERT_EQUAL_UINT16(bits, bf16_to_bits(a));
}

void test_bf16_get_sign(void) {
    TEST_ASSERT_EQUAL_UINT16(0, bf16_get_sign(bf16_from_bits(0x0000)));
    TEST_ASSERT_EQUAL_UINT16(1, bf16_get_sign(bf16_from_bits(0x8000)));
    TEST_ASSERT_EQUAL_UINT16(0, bf16_get_sign(bf16_from_bits(0x7F80)));
    TEST_ASSERT_EQUAL_UINT16(1, bf16_get_sign(bf16_from_bits(0xFF80)));
}

void test_bf16_get_exponent(void) {
    TEST_ASSERT_EQUAL_UINT16(0x00, bf16_get_exponent(bf16_from_bits(0x0000)));
    TEST_ASSERT_EQUAL_UINT16(0xFF, bf16_get_exponent(bf16_from_bits(0x7F80)));
    TEST_ASSERT_EQUAL_UINT16(0x7F, bf16_get_exponent(bf16_from_bits(0x3F80))); // 1.0 in float32/bf16
}

void test_bf16_get_mantissa(void) {
    TEST_ASSERT_EQUAL_UINT16(0x00, bf16_get_mantissa(bf16_from_bits(0x0000)));
    TEST_ASSERT_EQUAL_UINT16(0x00, bf16_get_mantissa(bf16_from_bits(0x7F80)));
    TEST_ASSERT_EQUAL_UINT16(0x7F, bf16_get_mantissa(bf16_from_bits(0x007F)));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_bf16_bits_conversion);
    RUN_TEST(test_bf16_get_sign);
    RUN_TEST(test_bf16_get_exponent);
    RUN_TEST(test_bf16_get_mantissa);
    return UNITY_END();
}
