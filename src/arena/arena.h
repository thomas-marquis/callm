#ifndef CALLM_ARENA_H
#define CALLM_ARENA_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "../core/bf16.h"

/**
 * @struct Arena
 * @brief Opaque structure representing a memory arena.
 */
typedef struct Arena Arena;

/**
 * @enum ArenaStatus
 * @brief Return codes for Arena operations.
 */
typedef enum ArenaStatus
{
    Arena_OK = 0,
    Arena_ERROR = 1
} ArenaStatus;

/**
 * @typedef ArenaHint
 * @brief Bitmask for providing hints about block allocation and usage.
 */
typedef uint32_t ArenaHint;

/* Alignment hints (bits 0-4) */
#define ARENA_HINT_ALIGN_DEFAULT  0x00
#define ARENA_HINT_ALIGN_4B       0x01
#define ARENA_HINT_ALIGN_8B       0x02
#define ARENA_HINT_ALIGN_16B      0x03
#define ARENA_HINT_ALIGN_32B      0x04
#define ARENA_HINT_ALIGN_64B      0x05

/* Cache behavior hints (bits 5-7) */
#define ARENA_HINT_CACHE_DEFAULT  (0x00 << 5)
#define ARENA_HINT_CACHE_STREAM   (0x01 << 5)
#define ARENA_HINT_CACHE_KEEP     (0x02 << 5)
#define ARENA_HINT_CACHE_PREFETCH (0x03 << 5)

/* Access pattern hints (bits 8-9) */
#define ARENA_HINT_ACCESS_DEFAULT    (0x00 << 8)
#define ARENA_HINT_ACCESS_SEQUENTIAL (0x01 << 8)
#define ARENA_HINT_ACCESS_RANDOM     (0x02 << 8)

/* Placement hints (bits 10, 16-23) */
#define ARENA_HINT_PLACE_CONTIGUOUS  (0x01 << 10)
#define ARENA_HINT_PLACE_ORDER_MASK  (0xFF << 16)
#define ARENA_HINT_PLACE_ORDER(n)    (((n) & 0xFF) << 16)

/* Usage hints (bits 24-27) */
#define ARENA_HINT_USAGE_DEFAULT     (0x00 << 24)
#define ARENA_HINT_USAGE_WEIGHTS     (0x01 << 24)
#define ARENA_HINT_USAGE_ACTIVATIONS (0x02 << 24)
#define ARENA_HINT_USAGE_GRADIENTS   (0x03 << 24)
#define ARENA_HINT_USAGE_SCRATCH     (0x04 << 24)

/**
 * @brief Creates a new memory arena.
 * @return A pointer to the new Arena, or NULL on failure.
 */
Arena *Arena_new(void);

/**
 * @brief Frees all memory associated with the arena.
 * @param arena The arena to free.
 */
void Arena_free(Arena *arena);

/**
 * @brief Configures a block for integer data.
 * @param arena The arena.
 * @param name Unique name for the block.
 * @param size Number of elements (not bytes).
 * @param storage_hint Hints for this block.
 * @return Arena_OK on success, Arena_ERROR on failure (e.g. duplicate name, already allocated).
 */
ArenaStatus Arena_config_int_block(Arena *arena, const char *name, size_t size, ArenaHint storage_hint);

/**
 * @brief Configures a block for float data.
 * @param arena The arena.
 * @param name Unique name for the block.
 * @param size Number of elements (not bytes).
 * @param storage_hint Hints for this block.
 * @return Arena_OK on success, Arena_ERROR on failure.
 */
ArenaStatus Arena_config_float_block(Arena *arena, const char *name, size_t size, ArenaHint storage_hint);

/**
 * @brief Configures a block for bfloat16 data.
 * @param arena The arena.
 * @param name Unique name for the block.
 * @param size Number of elements (not bytes).
 * @param storage_hint Hints for this block.
 * @return Arena_OK on success, Arena_ERROR on failure.
 */
ArenaStatus Arena_config_bf16_block(Arena *arena, const char *name, size_t size, ArenaHint storage_hint);

/**
 * @brief Allocates the contiguous memory for all configured blocks.
 * @param arena The arena.
 * @return Arena_OK on success, Arena_ERROR on failure (e.g. no blocks, already allocated).
 */
ArenaStatus Arena_allocate(Arena *arena);

/**
 * @brief Gets a generic pointer to a block's memory.
 * @param arena The arena.
 * @param name The name of the block.
 * @return Pointer to the block memory, or NULL if not found or not allocated.
 */
void *Arena_get_block(Arena *arena, const char *name);

/**
 * @brief Gets an integer pointer to a block's memory.
 * @param arena The arena.
 * @param name The name of the block.
 * @return Pointer to the block memory, or NULL if not found or not allocated.
 */
int *Arena_get_int_block(Arena *arena, const char *name);

/**
 * @brief Gets a float pointer to a block's memory.
 * @param arena The arena.
 * @param name The name of the block.
 * @return Pointer to the block memory, or NULL if not found or not allocated.
 */
float *Arena_get_float_block(Arena *arena, const char *name);

/**
 * @brief Gets a bfloat16 pointer to a block's memory.
 * @param arena The arena.
 * @param name The name of the block.
 * @return Pointer to the block memory, or NULL if not found or not allocated.
 */
bf16_t *Arena_get_bf16_block(Arena *arena, const char *name);

/**
 * @brief Gets the size of a block (number of elements).
 * @param arena The arena.
 * @param name The name of the block.
 * @return Number of elements, or 0 if not found.
 */
size_t Arena_get_block_size(Arena *arena, const char *name);

/**
 * @brief Gets the hint associated with a block.
 * @param arena The arena.
 * @param name The name of the block.
 * @return The block hint, or 0 if not found.
 */
ArenaHint Arena_get_block_hint(Arena *arena, const char *name);

#endif  // CALLM_ARENA_H
