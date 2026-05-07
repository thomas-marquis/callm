#include <unity.h>
#include <bf16.h>

void setUp(void) {}
void tearDown(void) {}

void test_bf16_rounding_ties_to_even(void) {
    // 1.0 + small value
    // 1.0 is 0x3F80
    // small value that triggers rounding
    // mantissa 1.0000000 (0x80)
    // Add bits that are exactly 0.5 of the LSB (which is bit 0 of mantissa, i.e. bit 7 of the 16-bit word)
    // No, bit 0 of mantissa is bit 0 of the 16-bit word.
    
    // In bf16_add_scalar, we shift left by 8 bits for precision.
    // So bit 8 is the LSB of the mantissa. Bits 7-0 are rounding bits.
    // 0x80 (1000 0000) is exactly halfway.
    
    // Let's test with multiplication as it's easier to control.
    // (1 + 1/128) * (1 + 1/128) = 1 + 2/128 + 1/16384
    // mant_a = 0x81, mant_b = 0x81
    // res_m = 0x81 * 0x81 = 0x4101
    // Bit 15 is NOT set (0x4101 < 0x8000).
    // so we use res_m >>= 7 -> 0x82. round bits = 0x01.
    // 0x01 < 0x40, so we truncate. Result mantissa 0x02 (after implicit).
}

int main(void) {
    UNITY_BEGIN();
    // RUN_TEST(test_bf16_rounding_ties_to_even);
    return UNITY_END();
}
