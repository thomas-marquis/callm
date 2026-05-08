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
test_arena_allocate_single(void)
{
    Arena *arena = Arena_new();
    Arena_config_int_block(arena, "test", 10, 0);
    TEST_ASSERT_EQUAL_INT(Arena_OK, Arena_allocate(arena));
    Arena_free(arena);
}

void
test_arena_allocate_multiple(void)
{
    Arena *arena = Arena_new();
    Arena_config_int_block(arena, "int", 10, 0);
    Arena_config_float_block(arena, "float", 20, 0);
    TEST_ASSERT_EQUAL_INT(Arena_OK, Arena_allocate(arena));
    Arena_free(arena);
}

void
test_arena_allocate_double(void)
{
    Arena *arena = Arena_new();
    Arena_config_int_block(arena, "test", 10, 0);
    TEST_ASSERT_EQUAL_INT(Arena_OK, Arena_allocate(arena));
    TEST_ASSERT_EQUAL_INT(Arena_ERROR, Arena_allocate(arena));
    Arena_free(arena);
}

void
test_arena_allocate_no_blocks(void)
{
    Arena *arena = Arena_new();
    TEST_ASSERT_EQUAL_INT(Arena_ERROR, Arena_allocate(arena));
    Arena_free(arena);
}

int
main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_arena_allocate_single);
    RUN_TEST(test_arena_allocate_multiple);
    RUN_TEST(test_arena_allocate_double);
    RUN_TEST(test_arena_allocate_no_blocks);
    return UNITY_END();
}
