#include "unity.h"

#include "../../src/core/base64.h"
#include <string.h>

void
setUp(void)
{
}

void
tearDown(void)
{
}

void
test_should_decode_base64_encoded_string()
{
    TEST_ASSERT_EQUAL_STRING(" Simple", base64_decode("IFNpbXBsZQ==", NULL));
    // TEST_ASSERT_EQUAL_STRING("Print", base64_decode("UHJpbnQ=", NULL));
    // TEST_ASSERT_EQUAL_STRING("/index", base64_decode("L2luZGV4", NULL));
    // TEST_ASSERT_EQUAL_STRING(" vous", base64_decode("IHZvdXM=", NULL));
    // TEST_ASSERT_EQUAL_STRING(" \"_", base64_decode("ICJf", NULL));
}

int
main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_should_decode_base64_encoded_string);
    return UNITY_END();
}
