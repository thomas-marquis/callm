# 003_memory_arena

<!-- HUMAN-START -->

This specification focuses ONLY on the design and implementation of a memory arena module as a C library for later use
in
the project.

The module, simply named `arena`, must handle allocation of big and known amount of memory, typically on application
startup.

The module must provide functions to configure the arena before allocation. The configuration functions must allows the
user to specify:

- the desired size of memory to allocate.
- each memory block is named
- functions to access to memory bloc. The user may be able to decide how the block is stored in memory. Please see
  bellow
- each function may be called multiple times to configure the arena block by bloc.
- functions to access the blocs or write on them

Other considerations:

- For ex: we want to configure 2 blocs named `A` and `B`, for 2 matrices. We know already we are going to multiply both
  later on runtime, so we want a way to store the matrices values in a smart and cache-optimized way. But this system
  must be set by the user on configuration, and not by the library itself.

Pseudocode example (it is not the actual or expected implementation, neither a source of truth, feel free to change
it):

```c++
typedef struct Arena Arena;

typedef enum ArenaStatus {
    Arena_OK,
    Arena_ERROR,
} ArenaStatus;

Arena* 
Arena_new(void);

void 
Arena_free(Arena* arena);

// Configures a named memory block in the arena.
// `storage_hint` is a user-defined integer to specify how the block should be stored (e.g., cache-optimized, aligned, etc.).
ArenaStatus
Arena_config_block(Arena* arena, const char* name, size_t size, int storage_hint);

ArenaStatus
Arena_config_int_block(Arena* arena, const char* name, size_t size);

// Allocates all configured blocks in the arena.
// Returns 0 on success, non-zero on error.
// This function must be called only once per arena.
// Configuration is no longer possible after allocation, and will return error.
ArenaStatus
Arena_allocate(Arena* arena);

// Returns a pointer to the named block, or NULL if not found or on error.
void *
arena_get_block(Arena* arena, const char* name);

// Returns the size of the named block, or 0 if not found or on error.
size_t 
arena_get_block_size(Arena* arena, const char* name);

```

What the memory looks like:

```python
A = [[1, 2, 3],
     [4, 5, 6]]

B = [[10, 20],
     [30, 40],
     [50, 60]]

C = A @ B

# 1*10 + 2*30 + 3*50, 1*20 + 2*40 + 3*60, 4*10 + 5*30 + 6*50, 4*20 + 5*40 + 6*60
# A[1, 1]*B[1, 1] + A[1, 2]*B[2, 1] + A[1, 3]*B[3, 1], ...

#             [[10,           20],
#              [30,           40],
#              [50,           60]]
# [[1, 2, 3],  [[10+60+150,   20+80+180],
#  [4, 5, 6]]   [40+150+300,  80+200+360]]
```

```c++
Arena *arena = Arena_new();
Arena_config_int_block(arena, "A", 6); // 6 is the block size
Arena_config_int_block(arena, "B", 6);
Arena_allocate(arena);
```

| address | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10| 11|
| block.s | A | A | A | A | A | A | B | B | B | B | B | B |
| content | 1 | 2 | 3 | 4 | 5 | 6 | 10| 20| 30| 40| 50| 60|

Get block:

```c++
int *block_a = (int *)Arena_get_block(arena, "A");
// block_a[0] == 1
// block_a[1] == 2
// block_a[2] == 3
// block_a[3] == 4
// block_a[4] == 5
// block_a[5] == 6

int *block_b = (int *)Arena_get_block(arena, "B");
// block_b[0] == 10
// block_b[1] == 20
// block_b[2] == 30
// block_b[3] == 40
// block_b[4] == 50
// block_b[5] == 60
```

<!-- HUMAN-END -->