#include "unity.h"

#include "../../src/core/base64.h"

void
setUp(void)
{
    // set stuff up here
}

void
tearDown(void)
{
    // clean stuff up here
}

void
test_should_decode_base64_encoded_string()
{
}

int
main(void)
{
    UNITY_BEGIN();
    // RUN_TEST (test_should_insert_item_into_hash_map);
    RUN_TEST(test_should_decode_base64_encoded_string);

    return UNITY_END();
}
