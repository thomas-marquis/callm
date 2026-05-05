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
- The configuration process is not thread safe by design
- We assume the user knows exactly how much memory he needs to allocate beforehand.
- Block names must be unique. An error must be returned during configuration if the user tries to configure a block
  again.

Pseudocode example (it is not the actual or expected implementation, neither a source of truth, feel free to change
it):

```c++
typedef struct Arena Arena;

typedef enum ArenaStatus {
    Arena_OK,
    Arena_ERROR,
} ArenaStatus;

////// constructor / descturctor

Arena* 
Arena_new(void);

void 
Arena_free(Arena* arena);

////// config: block definition

// Configures a named memory block in the arena.
// `storage_hint` is a user-defined integer to specify how the block should be stored (e.g., cache-optimized, aligned, etc.).
ArenaStatus
Arena_config_int_block(Arena* arena, const char* name, size_t size, int storage_hint);

ArenaStatus
Arena_config_float_block(Arena* arena, const char* name, size_t size, int storage_hint);

// Here, we assume the bf16 type does exist
ArenaStatus
Arena_config_bf16_block(Arena* arena, const char* name, size_t size, int storage_hint);

////// allocation: config no longer possible after that

// Allocates all configured blocks in the arena.
// Returns 0 on success, non-zero on error.
// This function must be called only once per arena.
// Configuration is no longer possible after allocation, and will return error.
ArenaStatus
Arena_allocate(Arena* arena);

// Returns a pointer to the named block, or NULL if not found or on error.
// If the specified block name does not correspond to an integer block, an error is returned.
int *
arena_get_int_block(Arena* arena, const char* name);

float *
arena_get_float_block(Arena* arena, const char* name);

bf16 *
arena_get_bf16_block(Arena* arena, const char* name);

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

## Memory Optimization Concepts

This section explains the key memory optimization concepts used in the arena design.

### Why Contiguous Memory Matters

Modern CPUs have a memory hierarchy:

- **Registers**: Fastest (nanoseconds), very limited size
- **L1 Cache**: ~1-4 cycles access, typically 32-64KB
- **L2 Cache**: ~10-20 cycles access, typically 256KB-1MB
- **L3 Cache**: ~30-50 cycles access, typically 2-32MB
- **Main Memory (RAM)**: ~100-300 cycles access, gigabytes available

When the CPU accesses memory, it loads data in **cache lines** (typically 64 bytes). If adjacent memory locations are
accessed together, they likely share a cache line, reducing the number of cache misses.

**Example**: Accessing array[0], array[1], array[2] is faster than accessing scattered pointers because the CPU
prefetches
sequential memory automatically.

### Memory Alignment

CPU instructions, especially SIMD (Single Instruction Multiple Data) instructions like SSE, AVX, and AVX-512, require
data to be aligned to specific byte boundaries:

| Instruction Set | Register Size | Required Alignment |
|-----------------|---------------|--------------------|
| SSE             | 128-bit (16B) | 16-byte boundary   |
| AVX             | 256-bit (32B) | 32-byte boundary   |
| AVX-512         | 512-bit (64B) | 64-byte boundary   |

An address is "aligned to N bytes" if `address % N == 0`. For example:

- Address 0x1000 is 16-byte aligned (0x1000 % 16 = 0)
- Address 0x1004 is 4-byte aligned but NOT 16-byte aligned

**Why it matters**: Loading misaligned data with SIMD instructions causes:

1. Performance penalty (extra cycles for realignment)
2. On some architectures: crashes (SSE2+ on x86 handles misaligned loads but with penalty)

**How to align**: If a pointer is at address 0x1004 and you need 16-byte alignment:

```c
uintptr_t addr = (uintptr_t)ptr;
void *aligned = (void *)((addr + 15) & ~15); // Rounds up to next 16-byte boundary
```

The arena's alignment hints (ARENA_HINT_ALIGN_16B, ARENA_HINT_ALIGN_32B, etc.) allow users to mark blocks that need
specific alignment, then apply it when accessing the block.

### Cache Behavior

Different data access patterns benefit from different cache strategies:

**Cache Hint Categories:**

| Hint                      | Meaning                        | When to Use                                                |
|---------------------------|--------------------------------|------------------------------------------------------------|
| ARENA_HINT_CACHE_DEFAULT  | No preference                  | General purpose                                            |
| ARENA_HINT_CACHE_STREAM   | Don't cache, sequential access | Large datasets accessed once (e.g., loading weights)       |
| ARENA_HINT_CACHE_KEEP     | Keep in cache if possible      | Frequently accessed data (e.g., current layer activations) |
| ARENA_HINT_CACHE_PREFETCH | Prefetch before use            | Data that will be needed soon                              |

**Streaming (ARENA_HINT_CACHE_STREAM)**:

- Tells the CPU: "Don't waste cache space on this data, I'll access it once and move on"
- Prevents evicting useful cached data
- Use for: loading model weights from disk, one-time matrix operations

**Keep (ARENA_HINT_CACHE_KEEP)**:

- Tells the CPU: "This data is valuable, keep it cached"
- Use for: frequently reused data like attention weights in a transformer

**Prefetch (ARENA_HINT_CACHE_PREFETCH)**:

- Tells the CPU: "Load this into cache now, I'll need it soon"
- Reduces latency when data is eventually accessed
- Use for: next layer's weights in a neural network

**Implementation note**: These are hints, not commands. The CPU may ignore them. Users apply these via platform-specific
intrinsics like `__builtin_prefetch()` on GCC/Clang.

### Access Patterns

How data is accessed affects performance:

| Pattern    | Description                           | Optimization                             |
|------------|---------------------------------------|------------------------------------------|
| Sequential | Access elements in order (0,1,2,3...) | Prefetch works well, cache-friendly      |
| Random     | Access elements in arbitrary order    | Poor cache locality, causes cache misses |

**Sequential access** (ARENA_HINT_ACCESS_SEQUENTIAL):

```c
for (int i = 0; i < n; i++) {
    sum += array[i];  // Each access is next to previous
}
```

- CPU prefetcher can predict next access
- Excellent cache line utilization (all 64 bytes used)

**Random access** (ARENA_HINT_ACCESS_RANDOM):

```c
for (int i = 0; i < n; i++) {
    sum += array[indices[i]];  // indices are random
}
```

- CPU prefetcher cannot predict
- Poor cache utilization (might use only 1 byte per 64-byte cache line)

### Placement Order and Cache Locality

**The key insight**: Data accessed together should be stored together in memory.

**Example - Matrix Multiplication (C = A @ B):**

```c
// Bad: A and B are far apart in memory
// Memory: [weights][A][scratch][B][output]
// When multiplying A @ B, we jump between A and B

// Good: A and B are contiguous
// Memory: [A][B][C][scratch]
// Both A and B are loaded into cache together
```

By placing A and B contiguously (using ARENA_HINT_PLACE_ORDER(0) for both), when the CPU loads cache lines for A,
it also loads cache lines for B, reducing cache misses during the multiplication.

**Grouping strategy**:

- Group "hot" data (frequently accessed together) with same placement order
- Separate "cold" data (rarely accessed) with higher placement order values
- Place data in order of access during computation

### Why These Hints Are Advisory (Not Enforced)

1. **Portability**: Different CPUs have different cache architectures
2. **Flexibility**: Users know their access patterns best
3. **Simplicity**: The arena focuses on layout, not runtime optimization
4. **Performance**: Applying hints at runtime would add overhead

The arena stores the hints as metadata. Users retrieve them with `Arena_get_block_hint()` and apply them in their
own code using platform-specific features.

## Functional specification

### Feature goal

Implement a memory arena module in C that pre-allocates a large contiguous memory region at application startup and
allows named memory blocks to be configured within it. The module is supposed to manage a predefined set of types: int,
float and bf16 (we will assume here this type exists).
The module must provide **library-defined storage hints** that control both memory access characteristics and block
placement ordering.

### Scope

- The arena allocates a single contiguous memory region containing all configured blocks
- Blocks are configured by name, size (in bytes), and optional storage hints before allocation
- **By default, blocks are laid out in the order they were configured**
- Storage hints can override the default ordering to place specific blocks first or group blocks together
- Once allocated, the configuration is immutable
- Blocks can be retrieved by name for read/write operations
- The module does NOT manage the lifetime of data stored in blocks (users are responsible for their own data)
- The module does NOT interpret the data stored in blocks (type-agnostic raw memory)

### Core requirements

**Arena lifecycle:**

- `Arena_new()`: Creates a new empty arena configuration
- `Arena_free()`: Releases all memory allocated by the arena
- `Arena_allocate()`: Allocates the contiguous memory region for all configured blocks (one-time operation)

**Block configuration:**

- `Arena_config_<type>_block()`: Convenience function for <type>-sized blocks (storage_hint defaults to 0)
- Multiple blocks can be configured sequentially
- Configuration is impossible after `Arena_allocate()` is called

**Block access:**

- `Arena_get_block()`: Returns a pointer to a named block's memory
- `Arena_get_block_size()`: Returns the size of a named block
- Both functions return NULL/0 on error (block not found, arena not allocated)

**Memory layout:**

- All blocks are stored contiguously in a single allocation
- **Default layout**: Blocks appear in the order they were configured
- **Custom layout**: Storage hints can override this ordering (see Storage Hints section)

### Storage hints

Storage hints are **library-defined** constants that users combine using bitwise OR. They serve two purposes:

1. **Advisory metadata**: Hints about alignment, cache behavior, and access patterns (users apply these in their own
   code)
2. **Placement control**: Hints that affect the physical layout of blocks within the arena

The arena module **stores** all hints and **uses placement hints** to determine block ordering. Other hints (alignment,
cache, access) are stored but not enforced by the arena - users apply them in their own code when accessing blocks.

#### Hint categories

```c
// Storage hints are 32-bit unsigned integers
// All hints are library-defined - users do NOT invent their own values
typedef uint32_t ArenaHint;

// ============================================================================
// CATEGORY 1: Alignment Hints (bits 0-4)
// Advisory: Suggests alignment requirements for SIMD or cache line access
// ============================================================================
#define ARENA_HINT_ALIGN_DEFAULT  0u           // No specific alignment
#define ARENA_HINT_ALIGN_4B      (1u << 0)    // 4-byte alignment
#define ARENA_HINT_ALIGN_8B      (1u << 1)    // 8-byte alignment
#define ARENA_HINT_ALIGN_16B     (1u << 2)    // 16-byte alignment (SSE)
#define ARENA_HINT_ALIGN_32B     (1u << 3)    // 32-byte alignment (AVX)
#define ARENA_HINT_ALIGN_64B     (1u << 4)    // 64-byte alignment (cache line)

// ============================================================================
// CATEGORY 2: Cache Behavior Hints (bits 5-7)
// Advisory: Suggests how data should be cached
// ============================================================================
#define ARENA_HINT_CACHE_DEFAULT  0u           // No specific cache behavior
#define ARENA_HINT_CACHE_STREAM   (1u << 5)    // Streaming: don't cache, sequential access
#define ARENA_HINT_CACHE_KEEP     (1u << 6)    // Keep in cache if possible
#define ARENA_HINT_CACHE_PREFETCH (1u << 7)    // Prefetch before use

// ============================================================================
// CATEGORY 3: Access Pattern Hints (bits 8-9)
// Advisory: Describes how the data will be accessed
// ============================================================================
#define ARENA_HINT_ACCESS_DEFAULT  0u           // No specific access pattern
#define ARENA_HINT_ACCESS_SEQUENTIAL (1u << 8)  // Sequential access (e.g., loops)
#define ARENA_HINT_ACCESS_RANDOM    (1u << 9)  // Random access (e.g., lookups)

// ============================================================================
// CATEGORY 4: Placement Order Hints (bits 16-23)
// ACTIONABLE: The arena USES these to determine block layout order
// ============================================================================
// Default behavior (no placement hint): blocks follow configuration order
//
// With ARENA_HINT_PLACE_ORDER(n):
// - Blocks are sorted by their order value (lower = placed first)
// - Blocks with the same order value maintain their configuration order
// - Order values: 0-255 (use 0 for "first", 1 for "second", etc.)
// - Blocks without explicit order (value 0 in these bits) go to position 256 (last)
#define ARENA_HINT_PLACE_ORDER_MASK (0xFFu << 16)
#define ARENA_HINT_PLACE_ORDER(n)    (((n) & 0xFF) << 16)

// ============================================================================
// CATEGORY 5: Usage Hints (bits 24-27)
// Advisory: Describes the purpose of the memory block
// ============================================================================
#define ARENA_HINT_USAGE_DEFAULT     0u           // No specific usage
#define ARENA_HINT_USAGE_WEIGHTS     (1u << 24)    // Model weights
#define ARENA_HINT_USAGE_ACTIVATIONS (1u << 25)    // Activations
#define ARENA_HINT_USAGE_GRADIENTS   (1u << 26)    // Gradients
#define ARENA_HINT_USAGE_SCRATCH     (1u << 27)    // Temporary/scratch space
```

#### Why library-defined hints?

The hints are library-defined (not user-defined integers) because:

1. **Consistency**: All users and code can rely on the same hint values
2. **Documentation**: Each hint has a clear, documented meaning
3. **Type safety**: Using `#define` constants prevents typos and magic numbers
4. **Future-proof**: The library can add new hints without breaking existing code
5. **Combinability**: Hints from different categories can be combined with `|` (bitwise OR)

Users **cannot** invent their own hint values. If they need custom metadata, they should use a separate mechanism (e.g.,
their own hash table mapping block names to custom data).

#### How the arena uses placement hints

The **only** hints that the arena **actively uses** are the placement order hints (`ARENA_HINT_PLACE_ORDER`). All other
hints are stored as metadata that users can retrieve and use in their own code.

**Placement algorithm:**

1. Collect all configured blocks
2. For each block, extract its placement order value from the hint:
    - If `ARENA_HINT_PLACE_ORDER(n)` is set: use `n` as the sort key
    - If no placement order hint: use 256 as the sort key (places these blocks after explicit-order blocks)
3. Sort blocks by their placement order value (ascending)
4. Within blocks that have the same placement order value, maintain their original configuration order
5. Lay out blocks contiguously in this sorted order

**Example:**

```c
Arena *arena = Arena_new();
// Block C: explicit order 2
Arena_config_bf16_block(arena, "C", 100, ARENA_HINT_PLACE_ORDER(2));
// Block A: explicit order 0 (first)
Arena_config_bf16_block(arena, "A", 100, ARENA_HINT_PLACE_ORDER(0));
// Block B: explicit order 1
Arena_config_bf16_block(arena, "B", 100, ARENA_HINT_PLACE_ORDER(1));
// Block D: no placement hint (default order = 256)
Arena_config_bf16_block(arena, "D", 100, 0);

Arena_allocate(arena);
// Resulting layout: [A][B][C][D]
// A has order 0, B has order 1, C has order 2, D has order 256
```

**Grouping blocks together:**
To place multiple blocks contiguously (e.g., for cache efficiency), give them the **same** placement order value. They
will maintain their configuration order within that group.

```c
Arena *arena = Arena_new();
// For matrix multiplication: A and B are used together, place them first and contiguous
Arena_config_bf16_block(arena, "A", 1000000, ARENA_HINT_PLACE_ORDER(0));
Arena_config_bf16_block(arena, "B", 1000000, ARENA_HINT_PLACE_ORDER(0)); // Same order = contiguous with A
// Output matrix comes after
Arena_config_bf16_block(arena, "C", 1000000, ARENA_HINT_PLACE_ORDER(1));
// Scratch space last
Arena_config_bf16_block(arena, "scratch", 100000, ARENA_HINT_PLACE_ORDER(2));

Arena_allocate(arena);
// Resulting layout: [A][B][C][scratch]
// A and B are contiguous (good for cache when multiplying A @ B)
```

#### How users apply advisory hints

Advisory hints (alignment, cache, access, usage) are **not** enforced by the arena. Instead, users retrieve them and
apply them in their own code:

```c
// After allocating, user can check hints to decide how to use blocks
void *ptr = Arena_get_block(arena, "weights");
size_t size = Arena_get_block_size(arena, "weights");
ArenaHint hint = Arena_get_block_hint(arena, "weights"); // New helper function

// Apply alignment if needed
if (hint & ARENA_HINT_ALIGN_64B) {
    ptr = (void *)(((uintptr_t)ptr + 63) & ~63); // Align to 64 bytes
}

// Apply cache prefetch if needed
if (hint & ARENA_HINT_CACHE_PREFETCH) {
    // Platform-specific prefetch
    #if defined(__x86_64__)
    __builtin_prefetch(ptr, 0, 3);
    #endif
}

// Use the memory with appropriate type
float *weights = (float *)ptr;
// ...
```

### Concrete examples

#### Example 1: Default behavior (no hints)

By default, blocks are placed in configuration order:

```c
Arena *arena = Arena_new();
Arena_config_bf16_block(arena, "weights", 4000000, 0);
Arena_config_bf16_block(arena, "activations", 2000000, 0);
Arena_config_bf16_block(arena, "scratch", 1000000, 0);
Arena_allocate(arena);
```

Memory layout:

```
Address range     | Block        | Size   | Notes
-----------------|--------------|--------|----------------------
0x0000-0x3D08FF   | weights      | 4MB    | First configured
0x3D0900-0x5A517F | activations  | 2MB    | Second configured
0x5A5180-0x69F4BF | scratch      | 1MB    | Third configured
```

#### Example 2: Matrix multiplication optimization

For matrix multiplication C = A @ B, we want A and B contiguous for cache efficiency:

```c
Arena *arena = Arena_new();

// Input matrix A (1024x1024 floats)
Arena_config_bf16_block(arena, "A", 1024*1024*sizeof(float),
                   ARENA_HINT_PLACE_ORDER(0) | ARENA_HINT_ALIGN_64B);

// Weight matrix B (1024x1024 floats) - same order as A = contiguous
Arena_config_bf16_block(arena, "B", 1024*1024*sizeof(float),
                   ARENA_HINT_PLACE_ORDER(0) | ARENA_HINT_ALIGN_64B);

// Output matrix C (1024x1024 floats)
Arena_config_bf16_block(arena, "C", 1024*1024*sizeof(float),
                   ARENA_HINT_PLACE_ORDER(1));

// Scratch space for intermediate calculations
Arena_config_bf16_block(arena, "scratch", 1000000,
                   ARENA_HINT_PLACE_ORDER(2) | ARENA_HINT_CACHE_STREAM);

Arena_allocate(arena);
```

Memory layout:

```
Address range        | Block   | Size   | Notes
--------------------|---------|--------|------------------------------
0x0000-0x0FFFBF      | A       | 4MB    | Order 0, first in group
0x100000-0x1FFFFF    | B       | 4MB    | Order 0, contiguous with A
0x200000-0x2FFFFF    | C       | 4MB    | Order 1
0x300000-0x3F3FFF    | scratch | ~1MB   | Order 2
```

When the user accesses these for matrix multiplication, they know A and B are adjacent in memory, improving cache
locality.

#### Example 3: Mixed types with placement control

```c
Arena *arena = Arena_new();

// bf16 activations (2 bytes per element) - most frequently accessed, place first
Arena_config_bf16_block(arena, "bf16_activations", 2000000 * sizeof(bf16),
                   ARENA_HINT_PLACE_ORDER(0) | 
                   ARENA_HINT_ACCESS_SEQUENTIAL | 
                   ARENA_HINT_CACHE_KEEP);

// Float weights (4 bytes per element) - next
Arena_config_bf16_block(arena, "float_weights", 1000000 * sizeof(float),
                   ARENA_HINT_PLACE_ORDER(1) | 
                   ARENA_HINT_ALIGN_64B | 
                   ARENA_HINT_USAGE_WEIGHTS);

// Int indices - last
Arena_config_bf16_block(arena, "int_indices", 500000 * sizeof(int),
                   ARENA_HINT_PLACE_ORDER(2));

Arena_allocate(arena);
```

Memory layout (assuming bf16=2B, float=4B, int=4B):

```
Address range     | Block              | Size    | Notes
-----------------|--------------------|---------|-------------------
0x0000-0x03D08F   | bf16_activations   | 4MB     | Order 0, first
0x03D090-0x07508F | float_weights      | 4MB     | Order 1, 64B align
0x075090-0x08E37F | int_indices        | 2MB     | Order 2, last
```

#### Example 4: SIMD alignment with placement

For SIMD vector operations, ensure proper alignment:

```c
Arena *arena = Arena_new();

// SSE vectors (16-byte aligned)
Arena_config_float_block(arena, "sse_vectors", 10000 * 16,
                   ARENA_HINT_PLACE_ORDER(0) | ARENA_HINT_ALIGN_16B);

// AVX vectors (32-byte aligned)
Arena_config_float_block(arena, "avx_vectors", 10000 * 32,
                   ARENA_HINT_PLACE_ORDER(1) | ARENA_HINT_ALIGN_32B);

// AVX-512 vectors (64-byte aligned)
Arena_config_float_block(arena, "avx512_vectors", 10000 * 64,
                   ARENA_HINT_PLACE_ORDER(2) | ARENA_HINT_ALIGN_64B);

Arena_allocate(arena);

// When using the blocks, user applies alignment:
float *sse = (float *)align_pointer(arena_get_block(arena, "sse_vectors"), 16);
float *avx = (float *)align_pointer(arena_get_block(arena, "avx_vectors"), 32);
float *avx512 = (float *)align_pointer(arena_get_block(arena, "avx512_vectors"), 64);

// Now safe for SIMD operations
__m128 sse_vec = _mm_load_ps(sse);        // Requires 16B alignment
__m256 avx_vec = _mm256_load_ps(avx);      // Requires 32B alignment
__m512 avx512_vec = _mm512_load_ps(avx512); // Requires 64B alignment
```

#### Example 5: Grouping multiple blocks together

Place all "hot" (frequently accessed) blocks together for cache efficiency:

```c
Arena *arena = Arena_new();

// Hot blocks (frequently accessed) - group in order 0
Arena_config_bf16_block(arena, "input", 1000000, ARENA_HINT_PLACE_ORDER(0));
Arena_config_bf16_block(arena, "weights", 4000000, ARENA_HINT_PLACE_ORDER(0));
Arena_config_bf16_block(arena, "output", 1000000, ARENA_HINT_PLACE_ORDER(0));

// Cold blocks (rarely accessed) - group in order 1
Arena_config_bf16_block(arena, "backup_A", 1000000, ARENA_HINT_PLACE_ORDER(1));
Arena_config_bf16_block(arena, "backup_B", 1000000, ARENA_HINT_PLACE_ORDER(1));

Arena_allocate(arena);
```

Memory layout:

```
Address range     | Block      | Size   | Notes
-----------------|------------|--------|------------------------
0x0000-0x0F3FFF   | input      | 1MB    | Hot group, first
0x0F4000-0x4E7FFF | weights    | 4MB    | Hot group, contiguous
0x4E8000-0x5DBFFF | output     | 1MB    | Hot group, contiguous
0x5DC000-0x6CFBFF | backup_A   | 1MB    | Cold group
0x6CFC00-0x7C33FF | backup_B   | 1MB    | Cold group, contiguous with backup_A
```

All hot blocks are contiguous, improving cache efficiency when they're accessed together.

#### Example 6: Complex combined hints

```c
Arena *arena = Arena_new();

// Critical path: place first, aligned, keep in cache, sequential access
Arena_config_bf16_block(arena, "critical_data", 500000,
                   ARENA_HINT_PLACE_ORDER(0) | 
                   ARENA_HINT_ALIGN_64B | 
                   ARENA_HINT_CACHE_KEEP | 
                   ARENA_HINT_ACCESS_SEQUENTIAL | 
                   ARENA_HINT_USAGE_ACTIVATIONS);

// Weights: second, aligned, streaming access (we'll iterate once)
Arena_config_bf16_block(arena, "weights", 2000000,
                   ARENA_HINT_PLACE_ORDER(1) | 
                   ARENA_HINT_ALIGN_64B | 
                   ARENA_HINT_CACHE_STREAM | 
                   ARENA_HINT_ACCESS_SEQUENTIAL | 
                   ARENA_HINT_USAGE_WEIGHTS);

// Scratch: last, no special alignment, temporary
Arena_config_bf16_block(arena, "scratch", 100000,
                   ARENA_HINT_PLACE_ORDER(2) | 
                   ARENA_HINT_USAGE_SCRATCH);

Arena_allocate(arena);
```

### Error handling

- All configuration and access functions return an `ArenaStatus` enum (OK or ERROR)
- `Arena_allocate()` fails if called more than once on the same arena
- `Arena_config_bf16_block()`/`Arena_config_int_block()`/etc. fail if called after allocation
- `Arena_get_block()` returns NULL if block name not found or arena not allocated
- `Arena_get_block_size()` returns 0 if block name not found or arena not allocated

### New API consideration

To support retrieving hints (for users to apply advisory hints in their code), consider adding:

```c
// Returns the storage hint that was configured for this block
ArenaHint Arena_get_block_hint(Arena *arena, const char *name);
```

This allows users to check alignment requirements and apply them when accessing blocks.

## Risks and limitations

- **Large allocation failure**: A single contiguous allocation for very large arenas may fail on systems with fragmented
  memory. The module should handle this gracefully but has no fallback mechanism (no chunked allocation).
- **No resizing**: Once allocated, the arena cannot be resized. Users must know all memory requirements upfront.
- **No bounds checking**: The module does not validate memory access bounds within blocks. Users are responsible for
  staying within configured block sizes.
- **No automatic alignment**: Alignment hints are advisory only. Users must manually align pointers if needed using the
  provided hints.
- **Memory waste**: Contiguous allocation may have padding between blocks for alignment. Users control this through
  placement hints.
- **Thread safety**: The current specification does not address thread safety. Concurrent access to configuration or
  allocation functions is undefined behavior.
- **Block naming**: No validation on block names is specified. Users must ensure names are unique and valid C strings.
- **No ownership tracking**: The arena does not track what data users store in blocks. Users must manage their own data
  lifetimes.
- **Type safety**: The arena is type-agnostic. Users must ensure correct type casting and size calculations.
- **Hint conflicts**: If users specify conflicting placement orders, the arena uses the sort algorithm described (lower
  order values first, then config order). There is no error for "conflicting" hints.
