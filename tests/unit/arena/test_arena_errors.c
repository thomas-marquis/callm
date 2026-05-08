#include "unity.h"
#include <callm/arena.h>

void
setUp(void)
{
}
void
tearDown(void)
{
}

void
test_arena_errors_null_params(void)
{
    TEST_ASSERT_EQUAL_INT(Arena_ERROR, Arena_config_int_block(NULL, "test", 10, 0));
    TEST_ASSERT_NULL(Arena_get_block(NULL, "test"));
    TEST_ASSERT_EQUAL_INT(0, Arena_get_block_size(NULL, "test"));
    TEST_ASSERT_EQUAL_INT(0, Arena_get_block_hint(NULL, "test"));
}

int
main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_arena_errors_null_params);
    return UNITY_END();
}
