#include "unity.h"
#include "../../../src/arena/arena.h"

void setUp(void) {}
void tearDown(void) {}

void test_arena_hints_storage(void) {
    Arena *arena = Arena_new();
    ArenaHint hint = ARENA_HINT_ALIGN_16B | ARENA_HINT_CACHE_STREAM | ARENA_HINT_USAGE_WEIGHTS;
    
    Arena_config_int_block(arena, "test", 10, hint);
    
    TEST_ASSERT_EQUAL_UINT32(hint, Arena_get_block_hint(arena, "test"));
    
    Arena_free(arena);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_arena_hints_storage);
    return UNITY_END();
}
