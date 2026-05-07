#include <unity.h>
#include <bf16.h>
#include <stdio.h>

void setUp(void) {}
void tearDown(void) {}

void test_bf16_cpu_features(void) {
    bool has_avx2 = bf16_cpu_has_avx2();
    size_t width = bf16_simd_width();
    
    printf("CPU has AVX2: %s\n", has_avx2 ? "yes" : "no");
    printf("SIMD width: %zu\n", width);
    
    if (has_avx2) {
        TEST_ASSERT_EQUAL_UINT(16, width);
    } else {
        TEST_ASSERT_EQUAL_UINT(1, width);
    }
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_bf16_cpu_features);
    return UNITY_END();
}
