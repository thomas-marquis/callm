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
test_arena_new_free(void)
{
    Arena *arena = Arena_new();
    TEST_ASSERT_NOT_NULL(arena);
    Arena_free(arena);
}

void
test_arena_free_null(void)
{
    Arena_free(NULL); /* Should not crash */
}

int
main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_arena_new_free);
    RUN_TEST(test_arena_free_null);
    return UNITY_END();
}
