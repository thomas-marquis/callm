# 003_memory_arena - Technical Plan

## Architecture

The memory arena is a **contiguous memory allocator** that pre-allocates a single large memory region at startup and
manages named blocks within it. The module follows the object-like C pattern from the constitution.

### Core Components

```
┌─────────────────────────────────────────────────────────────┐
│                         Arena                                  │
│  ┌─────────────────────────────────────────────────────────┐│
│  │                    Configuration Phase                    ││
│  │  - BlockRegistry: stores block configs (name, size, hint) ││
│  │  - Sorter: sorts blocks by placement hints                ││
│  │  - Validator: checks for duplicates, total size           ││
│  └─────────────────────────────────────────────────────────┘│
│  ┌─────────────────────────────────────────────────────────┐│
│  │                    Allocation Phase                        ││
│  │  - Single malloc() for entire arena                        ││
│  │  - BlockLayout: computes offsets based on sorted order    ││
│  │  - Metadata storage: block name→offset mapping             ││
│  └─────────────────────────────────────────────────────────┘│
└─────────────────────────────────────────────────────────────┘
       │
       ▼
┌─────────────────────────────────────────────────────────────┐
│              Contiguous Memory Region                          │
│  ┌─────────┐┌─────────┐┌─────────┐┌───────────────────────┐  │
│  │ Block A ││ Block B ││ Block C ││  Contiguity Group      │  │
│  └─────────┘└─────────┘└─────────┘└───────────────────────┘  │
│  [order=0]  [order=0]  [order=1]  [order=2, contiguous]        │
└─────────────────────────────────────────────────────────────┘
```

### Data Structures

**Primary Struct (arena.h)**

```c
typedef struct Arena Arena;
```

**Internal Struct (arena.c)**

```c
struct Arena {
    // Configuration phase data
    struct {
        char *name;           // Block name (owned)
        size_t size;         // Size in bytes
        ArenaHint hint;      // Storage hint
        size_t config_order; // Original configuration order
    } *blocks;
    size_t block_count;
    size_t block_capacity;
    
    // Allocation phase data
    void *memory;           // Single contiguous allocation
    size_t total_size;      // Total allocated size
    bool allocated;        // Whether Arena_allocate() was called
    
    // Post-allocation lookup (name → block index)
    // Using linear search for simplicity (block_count is small)
};
```

### Memory Layout Strategy

1. **Sort blocks** by placement order (bits 16-23 of hint)
    - Lower order values come first
    - Same order: maintain configuration order
    - No explicit order (bits 16-23 == 255 or 0 without flag): sort key = 256

2. **Compute offsets** sequentially
    - For each block in sorted order:
        - offset[i+1] = offset[i] + size[i]
        - If block i has ARENA_HINT_PLACE_CONTIGUOUS AND block i+1 has same placement order: no padding
        - Otherwise: add padding for alignment if needed

3. **Total size** = sum(all block sizes) + internal padding

### Hint Processing

Only **placement hints** are actionable by the arena:

- `ARENA_HINT_PLACE_ORDER(n)`: affects block ordering
- `ARENA_HINT_PLACE_CONTIGUOUS`: eliminates padding to next block with same order

All other hints (alignment, cache, access, usage) are stored as metadata and returned via `Arena_get_block_hint()` for
user application.

## Technical Requirements

### Dependencies

- C99 standard
- `src/core/bf16.h` for `bf16_t` type
- `src/shared/errors.h` for error handling patterns (reference only, not direct dependency)
- Standard library: `stdlib.h`, `string.h`, `stdint.h`, `stdbool.h`

### Memory Constraints

- Single contiguous allocation via `malloc()`
- No chunked or fallback allocation
- Maximum arena size: limited by system memory and address space
- Block count: dynamically grown array during configuration

### Type Support

| Type  | C Type   | Size             | Header      |
|-------|----------|------------------|-------------|
| int   | `int`    | `sizeof(int)`    | stdint.h    |
| float | `float`  | `sizeof(float)`  | stdint.h    |
| bf16  | `bf16_t` | `sizeof(bf16_t)` | core/bf16.h |

Note: `bf16_t` is `uint16_t` from `src/core/bf16.h`

## Expected Folder Structure

```
src/arena/
├── CMakeLists.txt          # Build configuration (already exists)
├── arena.h                # Public API header (partial exists)
├── arena.c                # Implementation
└── arena_config.h         # Internal configuration (optional)

tests/unit/arena/
├── CMakeLists.txt          # Test build configuration
├── test_arena_basic.c     # Basic functionality tests
├── test_arena_hints.c     # Hint processing tests
├── test_arena_layout.c    # Memory layout tests
└── test_arena_errors.c    # Error handling tests
```

## Existing Code Updates

### Files to Modify

1. **src/CMakeLists.txt**
    - Add: `add_subdirectory(arena)`
    - Currently missing (arena exists but not in build)

2. **src/arena/arena.h**
    - Update to match full specification
    - Add all public API functions
    - Add hint definitions
    - Add proper documentation

3. **src/arena/CMakeLists.txt**
    - Add `arena.c` to `CALLM_ARENA_SOURCES`
    - Link to `callm_core` for bf16_t (or add direct include path)

### New Files to Create

1. **src/arena/arena.c** - Full implementation
2. **tests/unit/arena/CMakeLists.txt** - Test configuration
3. **tests/unit/arena/test_arena_*.c** - Unit tests

## Public API

### Header File: arena.h

```c
#ifndef CALLM_ARENA_H
#define CALLM_ARENA_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// Forward declaration
typedef struct Arena Arena;

// Status enum
typedef enum ArenaStatus {
    Arena_OK,
    Arena_ERROR
} ArenaStatus;

// Hint type (32-bit unsigned integer)
typedef uint32_t ArenaHint;

// ============================================================================
// ALIGNMENT HINTS (bits 0-4)
// ============================================================================
#define ARENA_HINT_ALIGN_DEFAULT  0u
#define ARENA_HINT_ALIGN_4B      (1u << 0)
#define ARENA_HINT_ALIGN_8B      (1u << 1)
#define ARENA_HINT_ALIGN_16B     (1u << 2)
#define ARENA_HINT_ALIGN_32B     (1u << 3)
#define ARENA_HINT_ALIGN_64B     (1u << 4)

// ============================================================================
// CACHE BEHAVIOR HINTS (bits 5-7)
// ============================================================================
#define ARENA_HINT_CACHE_DEFAULT  0u
#define ARENA_HINT_CACHE_STREAM   (1u << 5)
#define ARENA_HINT_CACHE_KEEP     (1u << 6)
#define ARENA_HINT_CACHE_PREFETCH (1u << 7)

// ============================================================================
// ACCESS PATTERN HINTS (bits 8-9)
// ============================================================================
#define ARENA_HINT_ACCESS_DEFAULT      0u
#define ARENA_HINT_ACCESS_SEQUENTIAL (1u << 8)
#define ARENA_HINT_ACCESS_RANDOM      (1u << 9)

// ============================================================================
// PLACEMENT HINTS - ACTIONABLE BY ARENA
// ============================================================================
#define ARENA_HINT_PLACE_CONTIGUOUS (1u << 10)
#define ARENA_HINT_PLACE_ORDER_MASK  (0xFFu << 16)
#define ARENA_HINT_PLACE_ORDER(n)    (((n) & 0xFF) << 16)

// ============================================================================
// USAGE HINTS (bits 24-27)
// ============================================================================
#define ARENA_HINT_USAGE_DEFAULT     0u
#define ARENA_HINT_USAGE_WEIGHTS     (1u << 24)
#define ARENA_HINT_USAGE_ACTIVATIONS (1u << 25)
#define ARENA_HINT_USAGE_GRADIENTS   (1u << 26)
#define ARENA_HINT_USAGE_SCRATCH     (1u << 27)

// ============================================================================
// LIFECYCLE
// ============================================================================

/**
 * Creates a new empty arena configuration.
 * 
 * @return A pointer to a new Arena, or NULL on allocation failure.
 */
Arena *Arena_new(void);

/**
 * Releases all memory allocated by the arena.
 * 
 * @param arena The arena to free. If NULL, does nothing.
 */
void Arena_free(Arena *arena);

// ============================================================================
// CONFIGURATION (must be called before Arena_allocate)
// ============================================================================

/**
 * Configures an int-sized memory block in the arena.
 * 
 * @param arena The arena to configure.
 * @param name The unique name for this block.
 * @param size The size of the block IN BYTES.
 * @param storage_hint Library-defined hint for storage behavior.
 * @return Arena_OK on success, Arena_ERROR if:
 *         - name is not unique
 *         - called after Arena_allocate()
 */
ArenaStatus
Arena_config_int_block(Arena *arena, const char *name, size_t size, ArenaHint storage_hint);

/**
 * Configures a float-sized memory block in the arena.
 * 
 * @param arena The arena to configure.
 * @param name The unique name for this block.
 * @param size The size of the block IN BYTES.
 * @param storage_hint Library-defined hint for storage behavior.
 * @return Arena_OK on success, Arena_ERROR on failure.
 */
ArenaStatus
Arena_config_float_block(Arena *arena, const char *name, size_t size, ArenaHint storage_hint);

/**
 * Configures a bf16-sized memory block in the arena.
 * 
 * @param arena The arena to configure.
 * @param name The unique name for this block.
 * @param size The size of the block IN BYTES.
 * @param storage_hint Library-defined hint for storage behavior.
 * @return Arena_OK on success, Arena_ERROR on failure.
 */
ArenaStatus
Arena_config_bf16_block(Arena *arena, const char *name, size_t size, ArenaHint storage_hint);

// ============================================================================
// ALLOCATION
// ============================================================================

/**
 * Allocates all configured blocks in a single contiguous memory region.
 * 
 * This function must be called exactly once per arena.
 * After calling this, configuration is no longer possible.
 * 
 * @param arena The arena to allocate.
 * @return Arena_OK on success, Arena_ERROR if:
 *         - called more than once
 *         - allocation fails
 *         - no blocks configured
 */
ArenaStatus Arena_allocate(Arena *arena);

// ============================================================================
// BLOCK ACCESS (only valid after successful Arena_allocate)
// ============================================================================

/**
 * Returns a pointer to the named block's memory.
 * 
 * @param arena The arena containing the block.
 * @param name The name of the block.
 * @return Pointer to the block memory, or NULL if not found or error.
 */
void *Arena_get_block(Arena *arena, const char *name);

/**
 * Returns a typed pointer to an int block.
 * 
 * @param arena The arena containing the block.
 * @param name The name of the block.
 * @return Pointer to the int block, or NULL if not found or error.
 */
int *Arena_get_int_block(Arena *arena, const char *name);

/**
 * Returns a typed pointer to a float block.
 * 
 * @param arena The arena containing the block.
 * @param name The name of the block.
 * @return Pointer to the float block, or NULL if not found or error.
 */
float *Arena_get_float_block(Arena *arena, const char *name);

/**
 * Returns a typed pointer to a bf16 block.
 * 
 * @param arena The arena containing the block.
 * @param name The name of the block.
 * @return Pointer to the bf16 block, or NULL if not found or error.
 */
bf16_t *Arena_get_bf16_block(Arena *arena, const char *name);

/**
 * Returns the size of a named block in bytes.
 * 
 * @param arena The arena containing the block.
 * @param name The name of the block.
 * @return Size in bytes, or 0 if not found or error.
 */
size_t Arena_get_block_size(Arena *arena, const char *name);

/**
 * Returns the storage hint configured for a named block.
 * 
 * @param arena The arena containing the block.
 * @param name The name of the block.
 * @return The ArenaHint value, or 0 if not found or error.
 */
ArenaHint Arena_get_block_hint(Arena *arena, const char *name);

#endif // CALLM_ARENA_H
```

## Testing Strategy

### Test Framework

- Use Unity test framework (already integrated in project via CPM)
- Tests in `tests/unit/arena/` directory
- Each test file focuses on a specific aspect

### Test Categories

1. **Basic Lifecycle Tests** (`test_arena_basic.c`)
    - Arena_new() / Arena_free()
    - Multiple create/free cycles
    - NULL handling

2. **Configuration Tests** (`test_arena_config.c`)
    - Single block configuration
    - Multiple block configuration
    - Duplicate name detection
    - Configuration after allocation fails

3. **Allocation Tests** (`test_arena_allocation.c`)
    - Single block allocation
    - Multiple block allocation
    - Allocation failure handling
    - Double allocation fails

4. **Block Access Tests** (`test_arena_access.c`)
    - Get block by name
    - Typed accessors return correct types
    - Access before allocation fails
    - Access with invalid name fails
    - Block size retrieval
    - Block hint retrieval

5. **Layout Tests** (`test_arena_layout.c`)
    - Default ordering (configuration order)
    - Placement order sorting
    - Contiguity enforcement
    - Offset calculations

6. **Hint Tests** (`test_arena_hints.c`)
    - Hint combination via bitwise OR
    - Hint extraction for placement
    - All hint categories store correctly

7. **Error Handling Tests** (`test_arena_errors.c`)
    - All error conditions return Arena_ERROR
    - All error accessors return NULL/0
    - Error states are consistent

### Test Data Sizes

- Small blocks (1-100 bytes) for basic tests
- Medium blocks (1KB-1MB) for layout tests
- No very large blocks (>100MB) to avoid test failures on CI

## Makefile Targets

No new targets needed. The existing CMake build system will:

1. Build `callm_arena` static library
2. Link it to other modules that need it
3. Include tests in dev builds (RELEASE_TYPE=DEV)

The arena will be integrated into the main build via `add_subdirectory(arena)` in `src/CMakeLists.txt`.

## Technical Risks and Limitations

### Risks

1. **Large Allocation Failure**
    - Risk: Single malloc() for very large arena may fail
    - Mitigation: Return Arena_ERROR from Arena_allocate(), user must handle
    - No fallback: chunked allocation is out of scope

2. **Memory Fragmentation**
    - Risk: External fragmentation prevents contiguous allocation
    - Mitigation: Document that users must know total size upfront
    - Note: On 16GB systems, arenas >10GB may fail

3. **Name Collision**
    - Risk: Users accidentally use same name twice
    - Mitigation: Check for duplicates during configuration, return Arena_ERROR

4. **Thread Safety**
    - Risk: Concurrent configuration/allocation
    - Mitigation: Document as intentionally thread-unsafe
    - Note: Users must synchronize access if needed

5. **Hint Misuse**
    - Risk: Users specify conflicting placement hints
    - Mitigation: Document placement algorithm clearly
    - Note: Arena uses deterministic sort (order, then config order)

6. **Reserved Hint Value**
    - Risk: Users use ARENA_HINT_PLACE_ORDER(255)
    - Mitigation: Document as invalid, undefined behavior if used

### Limitations

1. **No Resizing**
    - Cannot add/remove blocks after Arena_allocate()
    - Cannot change arena size after allocation

2. **No Bounds Checking**
    - No validation of memory access within blocks
    - Users responsible for staying within configured sizes

3. **No Automatic Alignment**
    - Alignment hints are advisory only
    - Users must manually align pointers if needed

4. **No Ownership Tracking**
    - Arena doesn't track what users store in blocks
    - Users manage their own data lifetimes

5. **Type Safety**
    - Arena is type-agnostic
    - Users must ensure correct type casting and size calculations

6. **Performance Overhead**
    - Block lookup is O(n) where n = number of blocks
    - For typical use (<100 blocks), this is acceptable
    - Future optimization: hash table for large block counts

## Implementation Steps

### Phase 1: Core Structure

1. Implement Arena struct with block registry
2. Implement Arena_new() and Arena_free()
3. Add block configuration storage (dynamic array)

### Phase 2: Configuration

1. Implement Arena_config_int_block()
2. Implement Arena_config_float_block()
3. Implement Arena_config_bf16_block()
4. Add duplicate name checking
5. Add post-allocation configuration rejection

### Phase 3: Placement Algorithm

1. Implement placement order extraction from hints
2. Implement block sorting by placement order
3. Implement contiguity detection
4. Compute final layout with offsets

### Phase 4: Allocation

1. Calculate total size needed
2. Implement Arena_allocate() with single malloc()
3. Store block offsets for lookup
4. Handle allocation failure

### Phase 5: Block Access

1. Implement Arena_get_block() with linear search
2. Implement typed accessors (int, float, bf16)
3. Implement Arena_get_block_size()
4. Implement Arena_get_block_hint()
5. Add error checking for all accessors

### Phase 6: Testing

1. Create test directory structure
2. Implement basic tests
3. Implement configuration tests
4. Implement layout tests
5. Implement hint tests
6. Implement error tests

### Phase 7: Integration

1. Add arena to src/CMakeLists.txt
2. Update src/arena/CMakeLists.txt with sources
3. Add include dependency for bf16_t
4. Verify build succeeds
5. Run all tests

### Phase 8: Documentation

1. Verify all functions have documentation in header
2. Add any missing comments
3. Ensure consistency with project style
