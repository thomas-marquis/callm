# 003_memory_arena - Task List

## Implementation Tasks

### Phase 1: Update Build System

- [x] Update `src/CMakeLists.txt` to add `add_subdirectory(arena)`
- [x] Update `src/arena/CMakeLists.txt` to include `arena.c` in `CALLM_ARENA_SOURCES`
- [x] Verify `src/arena/CMakeLists.txt` links to `callm_shared` for error handling patterns

### Phase 2: Public API Header (`src/arena/arena.h`)

- [x] Update `ArenaStatus` enum to use `Arena_OK` and `Arena_ERROR` (matching spec)
- [x] Add `ArenaHint` typedef as `uint32_t`
- [x] Add all hint definition macros:
  - [x] Alignment hints (bits 0-4): `ARENA_HINT_ALIGN_DEFAULT`, `4B`, `8B`, `16B`, `32B`, `64B`
  - [x] Cache behavior hints (bits 5-7): `ARENA_HINT_CACHE_DEFAULT`, `STREAM`, `KEEP`, `PREFETCH`
  - [x] Access pattern hints (bits 8-9): `ARENA_HINT_ACCESS_DEFAULT`, `SEQUENTIAL`, `RANDOM`
  - [x] Placement hints (bits 10, 16-23): `ARENA_HINT_PLACE_CONTIGUOUS`, `ARENA_HINT_PLACE_ORDER_MASK`, `ARENA_HINT_PLACE_ORDER(n)`
  - [x] Usage hints (bits 24-27): `ARENA_HINT_USAGE_DEFAULT`, `WEIGHTS`, `ACTIVATIONS`, `GRADIENTS`, `SCRATCH`
- [x] Add include guards and necessary headers (`stddef.h`, `stdint.h`, `stdbool.h`)
- [x] Add forward declaration for `Arena` struct
- [x] Add function declarations for all public API functions:
  - [x] `Arena *Arena_new(void)`
  - [x] `void Arena_free(Arena *arena)`
  - [x] `ArenaStatus Arena_config_int_block(Arena *arena, const char *name, size_t size, ArenaHint storage_hint)`
  - [x] `ArenaStatus Arena_config_float_block(Arena *arena, const char *name, size_t size, ArenaHint storage_hint)`
  - [x] `ArenaStatus Arena_config_bf16_block(Arena *arena, const char *name, size_t size, ArenaHint storage_hint)`
  - [x] `ArenaStatus Arena_allocate(Arena *arena)`
  - [x] `void *Arena_get_block(Arena *arena, const char *name)`
  - [x] `int *Arena_get_int_block(Arena *arena, const char *name)`
  - [x] `float *Arena_get_float_block(Arena *arena, const char *name)`
  - [x] `bf16_t *Arena_get_bf16_block(Arena *arena, const char *name)`
  - [x] `size_t Arena_get_block_size(Arena *arena, const char *name)`
  - [x] `ArenaHint Arena_get_block_hint(Arena *arena, const char *name)`
- [x] Add comprehensive documentation for each function in header file

### Phase 3: Internal Data Structures (`src/arena/arena.c`)

- [x] Define internal `Arena` struct with fields:
  - [x] `blocks`: dynamic array of block configurations (name, size, hint, config_order)
  - [x] `block_count`: number of configured blocks
  - [x] `block_capacity`: capacity of blocks array
  - [x] `memory`: pointer to contiguous memory allocation
  - [x] `total_size`: total allocated size
  - [x] `allocated`: boolean flag for whether allocation has occurred
  - [x] `offsets`: array of offsets for each block (computed during allocation)

### Phase 4: Arena Lifecycle Implementation

- [x] Implement `Arena_new()`:
  - [x] Allocate Arena struct
  - [x] Initialize all fields to zero/NULL
  - [x] Return pointer to new arena or NULL on failure
- [x] Implement `Arena_free()`:
  - [x] Free all block names (strings)
  - [x] Free the contiguous memory region
  - [x] Free the blocks array
  - [x] Free the arena struct itself
  - [x] Handle NULL input gracefully

### Phase 5: Block Configuration Implementation

- [x] Implement dynamic array management for blocks (grow as needed)
- [x] Implement `Arena_config_int_block()`:
  - [x] Check if arena is already allocated → return `Arena_ERROR`
  - [x] Check for duplicate name → return `Arena_ERROR`
  - [x] Store block configuration (name, size, hint, config_order)
  - [x] Return `Arena_OK` on success
- [x] Implement `Arena_config_float_block()` (same logic as int)
- [x] Implement `Arena_config_bf16_block()` (same logic as int)

### Phase 6: Placement Algorithm Implementation

- [x] Implement hint extraction helper function:
  - [x] Extract placement order from hint using `ARENA_HINT_PLACE_ORDER_MASK`
  - [x] Return 256 for blocks with no explicit order (bits 16-23 == 255 or 0 without flag)
- [x] Implement comparison function for block sorting:
  - [x] Primary sort key: placement order value (ascending)
  - [x] Secondary sort key: original configuration order (for same placement order)
- [x] Implement layout computation:
  - [x] Sort blocks using comparison function
  - [x] Compute offsets sequentially
  - [x] Check `ARENA_HINT_PLACE_CONTIGUOUS` flag: if set on block i AND block i+1 has same placement order, add zero padding
  - [x] Otherwise, add padding for alignment if needed (default: no padding between blocks)
  - [x] Calculate total size needed

### Phase 7: Allocation Implementation

- [x] Implement `Arena_allocate()`:
  - [x] Check if already allocated → return `Arena_ERROR`
  - [x] Check if no blocks configured → return `Arena_ERROR`
  - [x] Compute total size using layout computation
  - [x] Call `malloc()` for contiguous region
  - [x] Store offsets for each block
  - [x] Set `allocated` flag to true
  - [x] Return `Arena_OK` on success, `Arena_ERROR` on failure

### Phase 8: Block Access Implementation

- [x] Implement `Arena_get_block()`:
  - [x] Check if arena is allocated → return NULL if not
  - [x] Linear search for block by name
  - [x] Return pointer to block memory (base + offset)
  - [x] Return NULL if not found
- [x] Implement typed accessors:
  - [x] `Arena_get_int_block()`: cast and return int pointer
  - [x] `Arena_get_float_block()`: cast and return float pointer
  - [x] `Arena_get_bf16_block()`: cast and return bf16_t pointer
- [x] Implement `Arena_get_block_size()`:
  - [x] Return stored size for named block, or 0 on error
- [x] Implement `Arena_get_block_hint()`:
  - [x] Return stored hint for named block, or 0 on error

### Phase 9: Error Handling

- [x] Ensure all configuration functions return `Arena_ERROR` for:
  - [x] Called after `Arena_allocate()`
  - [x] Duplicate block name
- [x] Ensure all accessor functions return NULL/0 for:
  - [x] Called before `Arena_allocate()`
  - [x] Block name not found
  - [x] NULL arena parameter
- [x] Handle malloc failures in `Arena_new()` and `Arena_allocate()`

### Phase 10: Test Infrastructure

- [x] Create `tests/unit/arena/CMakeLists.txt`:
  - [x] Add all test source files
  - [x] Link to `callm_arena` and `callm_core` (for bf16_t)
  - [x] Link to Unity test framework
- [x] Update `tests/unit/CMakeLists.txt` to add `add_subdirectory(arena)`

### Phase 11: Unit Tests

- [x] Create `tests/unit/arena/test_arena_basic.c`:
  - [x] Test `Arena_new()` returns non-NULL
  - [x] Test `Arena_free()` with valid arena
  - [x] Test `Arena_free()` with NULL arena
  - [x] Test multiple create/free cycles

- [x] Create `tests/unit/arena/test_arena_config.c`:
  - [x] Test single block configuration (int, float, bf16)
  - [x] Test multiple block configuration
  - [x] Test duplicate name detection returns `Arena_ERROR`
  - [x] Test configuration after allocation returns `Arena_ERROR`

- [x] Create `tests/unit/arena/test_arena_allocation.c`:
  - [x] Test single block allocation
  - [x] Test multiple block allocation
  - [x] Test double allocation returns `Arena_ERROR`
  - [x] Test allocation with no blocks returns `Arena_ERROR`
  - [x] Test allocation failure handling (mock malloc failure)

- [x] Create `tests/unit/arena/test_arena_access.c`:
  - [x] Test `Arena_get_block()` returns correct pointer
  - [x] Test typed accessors return correct types
  - [x] Test access before allocation returns NULL/0
  - [x] Test access with invalid name returns NULL/0
  - [x] Test `Arena_get_block_size()` returns correct size
  - [x] Test `Arena_get_block_hint()` returns correct hint
  - [x] Test NULL arena parameter returns NULL/0

- [x] Create `tests/unit/arena/test_arena_layout.c`:
  - [x] Test default ordering (configuration order preserved)
  - [x] Test placement order sorting (lower order first)
  - [x] Test same placement order maintains configuration order
  - [x] Test contiguity enforcement (zero padding between blocks with same order and CONTIGUOUS hint)
  - [x] Test blocks without explicit order placed after explicitly-ordered blocks
  - [x] Test `ARENA_HINT_PLACE_ORDER(255)` is invalid/undefined

- [x] Create `tests/unit/arena/test_arena_hints.c`:
  - [x] Test hint combination via bitwise OR
  - [x] Test all hint categories store correctly
  - [x] Test hint extraction for placement works correctly
  - [x] Test all hints are preserved and retrievable

- [x] Create `tests/unit/arena/test_arena_errors.c`:
  - [x] Test all error conditions return `Arena_ERROR`
  - [x] Test all error accessors return NULL/0
  - [x] Test error states are consistent

### Phase 12: Build Verification

- [x] Verify `src/arena/CMakeLists.txt` compiles without errors
- [x] Verify `src/CMakeLists.txt` includes arena module
- [x] Verify all tests compile and run successfully
- [x] Verify arena module links correctly with `callm_core` for bf16_t

### Phase 13: Code Review and Cleanup

- [x] Ensure all functions have documentation in header file
- [x] Ensure code follows project conventions (object-like approach, naming patterns)
- [x] Ensure no memory leaks (valgrind or similar)
- [x] Verify all edge cases are handled
