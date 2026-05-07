#include "unity.h"
#include "../../../src/arena/arena.h"
#include <stdint.h>

void setUp(void) {}
void tearDown(void) {}

void test_arena_layout_order(void) {
    Arena *arena = Arena_new();
    /* Config order: A, B, C */
    /* Placement order: C(10), A(20), B(no order=256) */
    Arena_config_int_block(arena, "A", 1, ARENA_HINT_PLACE_ORDER(20));
    Arena_config_int_block(arena, "B", 1, 0);
    Arena_config_int_block(arena, "C", 1, ARENA_HINT_PLACE_ORDER(10));
    
    Arena_allocate(arena);
    
    int *ptrA = Arena_get_int_block(arena, "A");
    int *ptrB = Arena_get_int_block(arena, "B");
    int *ptrC = Arena_get_int_block(arena, "C");
    
    /* Expected order in memory: C, A, B */
    TEST_ASSERT_TRUE((char *)ptrC < (char *)ptrA);
    TEST_ASSERT_TRUE((char *)ptrA < (char *)ptrB);
    
    Arena_free(arena);
}

void test_arena_layout_alignment(void) {
    Arena *arena = Arena_new();
    
    Arena_config_int_block(arena, "A", 1, 0); /* 4 bytes */
    Arena_config_int_block(arena, "B", 1, ARENA_HINT_ALIGN_16B);
    
    Arena_allocate(arena);
    
    char *ptrA = (char *)Arena_get_block(arena, "A");
    char *ptrB = (char *)Arena_get_block(arena, "B");
    
    TEST_ASSERT_EQUAL_INT(16, (int)(ptrB - ptrA));
    
    Arena_free(arena);
}

void test_arena_layout_contiguous(void) {
    Arena *arena = Arena_new();
    
    Arena_config_int_block(arena, "A", 1, ARENA_HINT_PLACE_ORDER(10) | ARENA_HINT_PLACE_CONTIGUOUS);
    Arena_config_int_block(arena, "B", 1, ARENA_HINT_PLACE_ORDER(10) | ARENA_HINT_ALIGN_16B);
    
    Arena_allocate(arena);
    
    char *ptrA = (char *)Arena_get_block(arena, "A");
    char *ptrB = (char *)Arena_get_block(arena, "B");
    
    TEST_ASSERT_EQUAL_INT((int)sizeof(int), (int)(ptrB - ptrA));
    
    Arena_free(arena);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_arena_layout_order);
    RUN_TEST(test_arena_layout_alignment);
    RUN_TEST(test_arena_layout_contiguous);
    return UNITY_END();
}
