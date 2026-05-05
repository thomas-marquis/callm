# 003_memory_arena - Technical Plan

## Architecture

The memory arena module (`arena`) implements a single contiguous memory allocation system with named blocks that can be
configured before allocation. The module follows the object-like pattern established in the project.

### Module Structure

```
arena.h  - Public API declarations
arena.c  - Implementation
```

### Core Components

1. **Arena struct** - Main container holding:
    - Configuration state (pre/post allocation)
    - List of configured blocks
    - The single contiguous memory allocation

2. **ArenaBlock struct** - Represents a configured memory block:
    - Block name (string)
    - Size in bytes
    - Storage hint (uint32_t)
    - Offset within the arena (computed at allocation time)

3. **Sorting mechanism** - Orders blocks by placement hints before layout

### Memory Layout

```
+----------------------------------------------------------+
| Arena metadata (Arena struct)                             |
+----------------------------------------------------------+
| Block A (size_A bytes, offset 0)                         |
+----------------------------------------------------------+
| Block B (size_B bytes, offset size_A)                    |
+----------------------------------------------------------+
| Block C (size_C bytes, offset size_A + size_B)          |
+----------------------------------------------------------+
| ...                                                      |
+----------------------------------------------------------+
```

All blocks are stored contiguously in a single `malloc()` call. The order is determined by:

1. Placement order hint (ARENA_HINT_PLACE_ORDER) - lower values first
2. Configuration order - for blocks with same placement order

## Technical Requirements

### Dependencies

- Standard C library: `stdlib.h`, `string.h`, `stdint.h`
- Project shared utilities: `errors.h` for error handling
- No external dependencies

### Compilation

- C standard: C11 or later
- No special compiler flags required
- Thread safety: NOT required (single-threaded use only per constitution)

### Resource Constraints

- Must handle allocations >10GB (safetensors files)
- Must work with limited RAM (16GB grandma's machine)
- No GPU support (CPU-only per constitution)

## Expected Folder Structure

```
src/core/memory/
├── arena.h
├── arena.c
└── CMakeLists.txt  (update existing)

tests/unit/core/
└── test_arena.c  (new)
```

## Existing Code Updates

### CMakeLists.txt Updates

**File: src/core/memory/CMakeLists.txt**

- Add `arena.c` to `CALLM_MEMORY_SOURCES`
- Add `arena.h` to `CALLM_MEMORY_HEADERS`

**File: src/core/CMakeLists.txt**

- No changes needed (already links callm_memory)

### Makefile Updates

No changes to the main Makefile. The existing build system via CMake handles library compilation.

Optional: Add convenience targets for arena-specific testing:

```makefile
build-arena:
	@cmake --build $(BUILD_DIR) --target callm_test_arena

test-arena:
	@cmake --build $(BUILD_DIR) --target callm_test_arena
	@ctest --test-dir $(BUILD_DIR) --output-on-failure -R "arena"
```

## API Design

### Public Types

```c
typedef struct Arena Arena;

typedef enum ArenaStatus {
    Arena_OK,
    Arena_ERROR,
} ArenaStatus;

typedef uint32_t ArenaHint;
```

### Storage Hint Constants

See specs.md for the complete list. Key categories:

- Alignment hints (bits 0-4): ARENA_HINT_ALIGN_4B, 8B, 16B, 32B, 64B
- Cache behavior hints (bits 5-7): ARENA_HINT_CACHE_STREAM, KEEP, PREFETCH
- Access pattern hints (bits 8-9): ARENA_HINT_ACCESS_SEQUENTIAL, RANDOM
- Placement order hints (bits 16-23): ARENA_HINT_PLACE_ORDER(n)
- Usage hints (bits 24-27): ARENA_HINT_USAGE_WEIGHTS, ACTIVATIONS, GRADIENTS, SCRATCH

### Public Functions

```c
// Lifecycle
Arena *Arena_new(void);
void Arena_free(Arena *arena);

// Configuration (pre-allocation only)
ArenaStatus Arena_config_block(Arena *arena, const char *name, size_t size, ArenaHint hint);
ArenaStatus Arena_config_int_block(Arena *arena, const char *name, size_t size);

// Allocation (one-time, finalizes configuration)
ArenaStatus Arena_allocate(Arena *arena);

// Access (post-allocation only)
void *Arena_get_block(Arena *arena, const char *name);
size_t Arena_get_block_size(Arena *arena, const char *name);
ArenaHint Arena_get_block_hint(Arena *arena, const char *name);
```

### Function Behavior

- `Arena_new()`: Creates empty arena, returns NULL on malloc failure
- `Arena_free()`: Frees all memory including blocks. Safe to call on NULL.
- `Arena_config_block()`: Adds block config. Fails if called after allocate.
- `Arena_config_int_block()`: Convenience wrapper with hint=0.
- `Arena_allocate()`: Performs single malloc for all blocks. Fails if:
    - Already allocated
    - No blocks configured
    - malloc fails
    - Duplicate block names exist
- `Arena_get_block()`: Returns pointer to block. Returns NULL if:
    - Arena not allocated
    - Block name not found
- `Arena_get_block_size()`: Returns block size. Returns 0 on error.
- `Arena_get_block_hint()`: Returns the hint value for a block. Returns 0 on error.

## Implementation Steps

### Phase 1: Core Structure

1. Define `ArenaBlock` struct with name, size, hint, offset
2. Define `Arena` struct with block list, allocation pointer, state flags
3. Implement `Arena_new()` and `Arena_free()`

### Phase 2: Configuration

1. Implement block list management (dynamic array)
2. Implement `Arena_config_block()` with validation:
    - Check for duplicate names
    - Check not already allocated
3. Implement `Arena_config_int_block()` as wrapper

### Phase 3: Allocation

1. Implement sorting by placement hints
    - Extract placement order from each block's hint
    - Sort blocks (stable sort to maintain config order within same priority)
2. Calculate total size needed
3. Perform single malloc
4. Compute offsets for each block
5. Implement `Arena_allocate()` with all validation

### Phase 4: Access

1. Implement lookup by name (linear search or hash table)
2. Implement `Arena_get_block()` with pointer arithmetic
3. Implement `Arena_get_block_size()`
4. Implement `Arena_get_block_hint()`

### Phase 5: Error Handling

1. Define all error conditions
2. Return appropriate ArenaStatus values
3. Ensure no memory leaks on error paths

## Testing Strategy

### Test Framework

- Use Unity test framework (already in project)
- Test file: `tests/unit/core/test_arena.c`

### Test Coverage

#### Basic Lifecycle Tests

- `test_Arena_new_and_free`: Basic creation and destruction
- `test_Arena_new_returns_null_on_failure`: malloc failure simulation

#### Configuration Tests

- `test_Arena_config_single_block`: Add one block
- `test_Arena_config_multiple_blocks`: Add multiple blocks
- `test_Arena_config_duplicate_name_fails`: Duplicate names not allowed
- `test_Arena_config_after_allocate_fails`: Config after allocate returns error

#### Allocation Tests

- `test_Arena_allocate_single_block`: Single block allocation
- `test_Arena_allocate_multiple_blocks`: Multiple blocks in config order
- `test_Arena_allocate_returns_error_when_called_twice`: Double allocation fails
- `test_Arena_allocate_with_no_blocks`: Empty arena (should this fail or be allowed?)

#### Placement Order Tests

- `test_Arena_placement_order_explicit`: Blocks with explicit orders
- `test_Arena_placement_order_default`: Blocks without placement hints (order 256)
- `test_Arena_placement_order_mixed`: Mix of explicit and default
- `test_Arena_placement_order_grouping`: Same order = contiguous
- `test_Arena_placement_order_config_order_preserved`: Same order maintains config order

#### Access Tests

- `test_Arena_get_block_valid`: Retrieve configured block
- `test_Arena_get_block_invalid_name`: Non-existent block returns NULL
- `test_Arena_get_block_not_allocated`: Get before allocate returns NULL
- `test_Arena_get_block_size`: Correct size returned
- `test_Arena_get_block_hint`: Correct hint returned
- `test_Arena_block_pointer_arithmetic`: Pointers are correct offsets

#### Memory Layout Tests

- `test_Arena_contiguous_layout`: All blocks are contiguous
- `test_Arena_no_gaps_between_blocks`: No padding between blocks (unless alignment added)
- `test_Arena_total_allocation_size`: Total size equals sum of block sizes

#### Type Agnostic Tests

- `test_Arena_float_block`: Cast to float*
- `test_Arena_int_block`: Cast to int*
- `test_Arena_custom_type`: Cast to custom struct
- `test_Arena_mixed_types`: Multiple blocks with different types

#### Error Handling Tests

- `test_Arena_free_null_arena`: Safe to free NULL
- `test_Arena_get_block_null_arena`: Returns NULL
- `test_Arena_get_block_null_name`: Returns NULL

### Test Data

Use small sizes for fast testing (e.g., 100-1000 bytes per block).

## Technical Risks and Limitations

### Risks

1. **Large allocation failure**: Single malloc for >10GB may fail on fragmented systems.
    - *Mitigation*: Document limitation. Users must ensure sufficient contiguous memory.

2. **Duplicate block names**: Must be detected during configuration.
    - *Mitigation*: Linear search on each config call (O(n²) but n is small).

3. **Block lookup performance**: Linear search on access may be slow with many blocks.
    - *Mitigation*: Expected block count is small (<100). Linear search acceptable.
    - *Alternative*: Could use hash table, but adds complexity and dependency.

4. **Sorting stability**: Must maintain config order for same-priority blocks.
    - *Mitigation*: Use stable sort algorithm (e.g., bubble sort for small n, or maintain config index).

5. **Memory waste**: Contiguous allocation may have internal fragmentation.
    - *Mitigation*: Acceptable tradeoff for cache efficiency. Users control layout via hints.

### Limitations

1. **No resizing**: Arena is immutable after allocation.
2. **No bounds checking**: Users must stay within configured sizes.
3. **No automatic alignment**: Users must apply alignment hints manually.
4. **No thread safety**: Single-threaded use only.
5. **No chunked allocation**: Single contiguous region only.
6. **Block count limited**: Only by available memory and address space.

## Memory Management Strategy

### Allocation Pattern

- Single `malloc()` for the entire arena memory
- Individual `malloc()` calls for Arena struct and block metadata
- Block list uses dynamic array (realloc as needed)

### Ownership

- Arena owns the contiguous memory allocation
- Arena owns the Arena struct and all ArenaBlock structs
- Users own the data stored in blocks (arena only manages memory lifetime)

### Cleanup

- `Arena_free()` frees:
    1. The contiguous memory block
    2. All ArenaBlock structs
    3. The Arena struct itself
- All frees are checked for NULL safety

## Naming Conventions

Follow project conventions:

- Struct type: `Arena` (typedef'd)
- Function names: `Arena_*` for ALL functions (capital A, no matter getter or other)
- Constants: `ARENA_HINT_*` (all caps, underscore separated)
- Status enum: `ArenaStatus` with `Arena_OK`, `Arena_ERROR`

## File Organization

### arena.h

```c
#include <stddef.h>
#include <stdint.h>
#include "../../shared/errors.h"

// Types and constants
// Function declarations
```

### arena.c

```c
#include "arena.h"
#include <stdlib.h>
#include <string.h>

// Struct definitions (opaque in header)
// Static helper functions
// Public function implementations
```

## Build Integration

The arena module integrates into the existing `callm_memory` library:

- Compiled as part of `libcallm_memory.a`
- Linked into `libcallm_core.a`
- No changes to top-level CMakeLists.txt needed

## Future Considerations

Not in scope for this feature, but considered:

- Alignment enforcement by arena (currently user responsibility)
- Thread-safe version (not needed per constitution)
- Chunked allocation for very large arenas
- Block resizing (would require reallocation)
- Serialization/deserialization of arena configuration
