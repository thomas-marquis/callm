#include "unity.h"
#include <callm/arena.h>
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
test_arena_access_basic(void)
{
    Arena *arena = Arena_new();
    Arena_config_int_block(arena, "int", 10, 0);
    Arena_config_float_block(arena, "float", 20, 0);
    Arena_allocate(arena);

    TEST_ASSERT_NOT_NULL(Arena_get_block(arena, "int"));
    TEST_ASSERT_NOT_NULL(Arena_get_int_block(arena, "int"));
    TEST_ASSERT_NOT_NULL(Arena_get_float_block(arena, "float"));

    TEST_ASSERT_EQUAL_INT(10, Arena_get_block_size(arena, "int"));
    TEST_ASSERT_EQUAL_INT(20, Arena_get_block_size(arena, "float"));

    Arena_free(arena);
}

void
test_arena_access_before_allocate(void)
{
    Arena *arena = Arena_new();
    Arena_config_int_block(arena, "int", 10, 0);
    TEST_ASSERT_NULL(Arena_get_block(arena, "int"));
    TEST_ASSERT_EQUAL_INT(10, Arena_get_block_size(arena, "int"));
    Arena_free(arena);
}

void
test_arena_access_invalid_name(void)
{
    Arena *arena = Arena_new();
    Arena_config_int_block(arena, "int", 10, 0);
    Arena_allocate(arena);
    TEST_ASSERT_NULL(Arena_get_block(arena, "invalid"));
    TEST_ASSERT_EQUAL_INT(0, Arena_get_block_size(arena, "invalid"));
    Arena_free(arena);
}

int
main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_arena_access_basic);
    RUN_TEST(test_arena_access_before_allocate);
    RUN_TEST(test_arena_access_invalid_name);
    return UNITY_END();
}
