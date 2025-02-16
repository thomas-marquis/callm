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
test_should_encode_text_correctly()
{
    TEST_ASSERT_EQUAL_STRING(" Simple", base64_decode("IFNpbXBsZQ==", NULL));
    TEST_ASSERT_EQUAL_STRING("Print", base64_decode("UHJpbnQ=", NULL));
    TEST_ASSERT_EQUAL_STRING("/index", base64_decode("L2luZGV4", NULL));
    TEST_ASSERT_EQUAL_STRING(" vous", base64_decode("IHZvdXM=", NULL));
    TEST_ASSERT_EQUAL_STRING(" \"_", base64_decode("ICJf", NULL));
}

void
test_should_decode_with_output_len()
{
    int res = 0;

    base64_decode("IFNpbXBsZQ==", &res);
    TEST_ASSERT_EQUAL_INT((int) strlen(" Simple"), res);

    base64_decode("UHJpbnQ=", &res);
    TEST_ASSERT_EQUAL_INT((int) strlen("Print"), res);

    base64_decode("L2luZGV4", &res);
    TEST_ASSERT_EQUAL_INT((int) strlen("/index"), res);

    base64_decode("IHZvdXM=", &res);
    TEST_ASSERT_EQUAL_INT((int) strlen(" vous"), res);

    base64_decode("ICJf", &res);
    TEST_ASSERT_EQUAL_INT((int) strlen(" \"_"), res);
}

int
main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_should_encode_text_correctly);
    RUN_TEST(test_should_decode_with_output_len);
    return UNITY_END();
}
