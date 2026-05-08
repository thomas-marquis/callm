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
test_arena_config_single(void)
{
    Arena *arena = Arena_new();
    TEST_ASSERT_EQUAL_INT(Arena_OK, Arena_config_int_block(arena, "test", 10, 0));
    Arena_free(arena);
}

void
test_arena_config_multiple(void)
{
    Arena *arena = Arena_new();
    TEST_ASSERT_EQUAL_INT(Arena_OK, Arena_config_int_block(arena, "int", 10, 0));
    TEST_ASSERT_EQUAL_INT(Arena_OK, Arena_config_float_block(arena, "float", 20, 0));
    TEST_ASSERT_EQUAL_INT(Arena_OK, Arena_config_bf16_block(arena, "bf16", 30, 0));
    Arena_free(arena);
}

void
test_arena_config_duplicate_name(void)
{
    Arena *arena = Arena_new();
    TEST_ASSERT_EQUAL_INT(Arena_OK, Arena_config_int_block(arena, "test", 10, 0));
    TEST_ASSERT_EQUAL_INT(Arena_ERROR, Arena_config_float_block(arena, "test", 20, 0));
    Arena_free(arena);
}

void
test_arena_config_after_allocate(void)
{
    Arena *arena = Arena_new();
    Arena_config_int_block(arena, "test", 10, 0);
    Arena_allocate(arena);
    TEST_ASSERT_EQUAL_INT(Arena_ERROR, Arena_config_int_block(arena, "test2", 10, 0));
    Arena_free(arena);
}

int
main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_arena_config_single);
    RUN_TEST(test_arena_config_multiple);
    RUN_TEST(test_arena_config_duplicate_name);
    RUN_TEST(test_arena_config_after_allocate);
    return UNITY_END();
}
