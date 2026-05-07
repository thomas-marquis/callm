#include "arena.h"
#include <stdlib.h>
#include <string.h>

/**
 * @struct BlockConfig
 * @brief Internal configuration for a single block in the arena.
 */
typedef struct
{
    char *name;
    size_t size; /* Number of elements */
    ArenaHint hint;
    size_t config_order; /* Original configuration order */
    size_t element_size; /* Size of a single element in bytes */
} BlockConfig;

/**
 * @struct Arena
 * @brief Internal representation of the memory arena.
 */
struct Arena
{
    BlockConfig *blocks;
    size_t block_count;
    size_t block_capacity;
    void *memory;
    size_t total_size; /* Total size in bytes */
    bool allocated;
    size_t *offsets; /* Computed byte offsets for each block (same order as blocks) */
};

Arena *
Arena_new(void)
{
    Arena *arena = (Arena *) calloc(1, sizeof(Arena));
    if (!arena)
        return NULL;
    return arena;
}

void
Arena_free(Arena *arena)
{
    if (!arena)
        return;

    if (arena->blocks)
    {
        for (size_t i = 0; i < arena->block_count; i++)
        {
            free(arena->blocks[i].name);
        }
        free(arena->blocks);
    }

    free(arena->memory);
    free(arena->offsets);
    free(arena);
}

static char *
internal_strdup(const char *s)
{
    size_t len = strlen(s) + 1;
    char *new_s = (char *) malloc(len);
    if (new_s)
        memcpy(new_s, s, len);
    return new_s;
}

static ArenaStatus
ensure_capacity(Arena *arena)
{
    if (arena->block_count < arena->block_capacity)
    {
        return Arena_OK;
    }

    size_t new_capacity = (arena->block_capacity == 0) ? 8 : arena->block_capacity * 2;
    BlockConfig *new_blocks = (BlockConfig *) realloc(arena->blocks, new_capacity * sizeof(BlockConfig));
    if (!new_blocks)
    {
        return Arena_ERROR;
    }

    arena->blocks = new_blocks;
    arena->block_capacity = new_capacity;
    return Arena_OK;
}

static ArenaStatus
config_block(Arena *arena, const char *name, size_t size, ArenaHint storage_hint, size_t element_size)
{
    if (!arena || !name || arena->allocated)
    {
        return Arena_ERROR;
    }

    /* Check for duplicate name */
    for (size_t i = 0; i < arena->block_count; i++)
    {
        if (strcmp(arena->blocks[i].name, name) == 0)
        {
            return Arena_ERROR;
        }
    }

    if (ensure_capacity(arena) != Arena_OK)
    {
        return Arena_ERROR;
    }

    BlockConfig *block = &arena->blocks[arena->block_count];
    block->name = internal_strdup(name);
    if (!block->name)
    {
        return Arena_ERROR;
    }

    block->size = size;
    block->hint = storage_hint;
    block->config_order = arena->block_count;
    block->element_size = element_size;

    arena->block_count++;
    return Arena_OK;
}

ArenaStatus
Arena_config_int_block(Arena *arena, const char *name, size_t size, ArenaHint storage_hint)
{
    return config_block(arena, name, size, storage_hint, sizeof(int));
}

ArenaStatus
Arena_config_float_block(Arena *arena, const char *name, size_t size, ArenaHint storage_hint)
{
    return config_block(arena, name, size, storage_hint, sizeof(float));
}

ArenaStatus
Arena_config_bf16_block(Arena *arena, const char *name, size_t size, ArenaHint storage_hint)
{
    return config_block(arena, name, size, storage_hint, sizeof(bf16_t));
}

static int
get_placement_order(ArenaHint hint)
{
    uint32_t order = (hint & ARENA_HINT_PLACE_ORDER_MASK) >> 16;
    if (order == 0 || order == 255)
    {
        return 256;
    }
    return (int) order;
}

static int
block_ptr_compare(const void *a, const void *b)
{
    const BlockConfig *ba = *(const BlockConfig **) a;
    const BlockConfig *bb = *(const BlockConfig **) b;

    int order_a = get_placement_order(ba->hint);
    int order_b = get_placement_order(bb->hint);

    if (order_a != order_b)
    {
        return order_a - order_b;
    }

    /* Secondary key: configuration order */
    if (ba->config_order < bb->config_order)
        return -1;
    if (ba->config_order > bb->config_order)
        return 1;
    return 0;
}

ArenaStatus
Arena_allocate(Arena *arena)
{
    if (!arena || arena->allocated || arena->block_count == 0)
    {
        return Arena_ERROR;
    }

    /* 1. Create a temporary array of pointers to blocks for sorting */
    BlockConfig **sorted_blocks = malloc(arena->block_count * sizeof(BlockConfig *));
    if (!sorted_blocks)
    {
        return Arena_ERROR;
    }
    for (size_t i = 0; i < arena->block_count; i++)
    {
        sorted_blocks[i] = &arena->blocks[i];
    }

    /* 2. Sort pointers based on placement order */
    qsort(sorted_blocks, arena->block_count, sizeof(BlockConfig *), block_ptr_compare);

    /* 3. Compute layout and offsets */
    arena->offsets = (size_t *) calloc(arena->block_count, sizeof(size_t));
    if (!arena->offsets)
    {
        free(sorted_blocks);
        return Arena_ERROR;
    }

    size_t current_offset = 0;
    for (size_t i = 0; i < arena->block_count; i++)
    {
        BlockConfig *block = sorted_blocks[i];

        bool skip_alignment = false;
        if (i > 0)
        {
            BlockConfig *prev_block = sorted_blocks[i - 1];
            if ((prev_block->hint & ARENA_HINT_PLACE_CONTIGUOUS)
                && get_placement_order(prev_block->hint) == get_placement_order(block->hint))
            {
                skip_alignment = true;
            }
        }

        if (!skip_alignment)
        {
            size_t alignment = 1;
            uint32_t align_bits = block->hint & 0x1F; /* bits 0-4 */
            switch (align_bits)
            {
            case ARENA_HINT_ALIGN_4B:
                alignment = 4;
                break;
            case ARENA_HINT_ALIGN_8B:
                alignment = 8;
                break;
            case ARENA_HINT_ALIGN_16B:
                alignment = 16;
                break;
            case ARENA_HINT_ALIGN_32B:
                alignment = 32;
                break;
            case ARENA_HINT_ALIGN_64B:
                alignment = 64;
                break;
            default:
                alignment = 1;
                break;
            }
            if (alignment > 1)
            {
                current_offset = (current_offset + alignment - 1) & ~(alignment - 1);
            }
        }

        arena->offsets[block->config_order] = current_offset;
        current_offset += block->size * block->element_size;
    }

    arena->total_size = current_offset;
    arena->memory = malloc(arena->total_size);
    if (!arena->memory)
    {
        free(sorted_blocks);
        free(arena->offsets);
        arena->offsets = NULL;
        return Arena_ERROR;
    }

    memset(arena->memory, 0, arena->total_size);

    arena->allocated = true;
    free(sorted_blocks);
    return Arena_OK;
}

void *
Arena_get_block(Arena *arena, const char *name)
{
    if (!arena || !arena->allocated || !name)
    {
        return NULL;
    }

    for (size_t i = 0; i < arena->block_count; i++)
    {
        if (strcmp(arena->blocks[i].name, name) == 0)
        {
            return (char *) arena->memory + arena->offsets[i];
        }
    }

    return NULL;
}

int *
Arena_get_int_block(Arena *arena, const char *name)
{
    return Arena_get_block(arena, name);
}

float *
Arena_get_float_block(Arena *arena, const char *name)
{
    return Arena_get_block(arena, name);
}

bf16_t *
Arena_get_bf16_block(Arena *arena, const char *name)
{
    return Arena_get_block(arena, name);
}

size_t
Arena_get_block_size(Arena *arena, const char *name)
{
    if (!arena || !name)
        return 0;
    for (size_t i = 0; i < arena->block_count; i++)
    {
        if (strcmp(arena->blocks[i].name, name) == 0)
        {
            return arena->blocks[i].size;
        }
    }
    return 0;
}

ArenaHint
Arena_get_block_hint(Arena *arena, const char *name)
{
    if (!arena || !name)
        return 0;
    for (size_t i = 0; i < arena->block_count; i++)
    {
        if (strcmp(arena->blocks[i].name, name) == 0)
        {
            return arena->blocks[i].hint;
        }
    }
    return 0;
}
