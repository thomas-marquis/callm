#ifndef CALLM_ARENA_H
#define CALLM_ARENA_H

#include <stddef.h>

typedef struct Arena Arena;

typedef enum ArenaStatus
{
    OK,
    ERROR
} ArenaStatus;

Arena *Arena_new(void);

void Arena_free(Arena *arena);

////////// Configuration //////////

ArenaStatus Arena_config_int_block(Arena *arena, const char *block_name, size_t num_elements);

////////// Allocation //////////

////////// Usage //////////

#endif  // CALLM_ARENA_H
