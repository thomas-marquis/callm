# 005_linalg

<!-- HUMAN-START -->

## General considerations

- Implement a C static library named `callm_linalg` that exposes and implements linear algebra operations.
- This library MUST rely on the existing arena library for memory management (no direct malloc)
- This library MUST rely on the existing bf16 library to manage the bf16 type. If an operation is missing from this
  library, please mention it
- This library doesn't handle IO operations, it gona be used later by the safetensors library, but this is out of scope
  for now.
- Each function of the library linalg must be type-specific. Thus, they must contain the type name in their name. E.g:
  some_operation_bf16.
- For now, only the bf16 type must be supported. Nothing else.
- This library MUST be model agnostic. It must not contains any model-specific logic or assumptions.
- The library MUST use an OOP approach for naming: function names must start with the object they operate on (e.g.,
  Tensor_matmul, Matrix_transpose, Vector_add).
- Except bf16 and arena, no other third party library is allowed.
- This library MUST be able to work on a single CPU. Levreage all possible techniques to achieve this (parallelism,
  pipelining, threads, etc).
- No GPU
- ignore the existing llm code

## Function list for the library

Here’s a **checklist of all mathematical operations** you’ll need to implement in C, grouped by category:

### **🔹 Core Linear Algebra**

| **Operation**                | **C Implementation Notes**                                                              | **Dependencies**    |
|------------------------------|-----------------------------------------------------------------------------------------|---------------------|
| Matrix Multiplication        | `C = A @ B` (GEMM). Use **BLAS** (`cblas_sgemm`/`cblas_dgemm`) or a custom GEMM kernel. | BLAS or custom GEMM |
| Matrix-Vector Multiplication | `y = A @ x` (GEMV). Use `cblas_sgemv`/`cblas_dgemv`.                                    | BLAS                |
| Transpose                    | In-place or out-of-place transpose. Optimize for cache locality.                        | None                |
| Reshape                      | No computation, just reinterpret memory layout.                                         | None                |
| Concatenation                | Copy memory blocks sequentially.                                                        | None                |
| Split                        | Create views into existing memory (no copy).                                            | None                |

### **🔹 Normalization (RMSNorm)**

| **Operation** | **C Implementation Notes**                                                                                                                                                     | **Dependencies**              |
|---------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|-------------------------------|
| RMSNorm       | 1. Compute `mean_squared = sum(x_i^2) / dim + eps`.<br>2. `inv_norm = 1.0 / sqrt(mean_squared)`.<br>3. `x_normed = x * inv_norm`.<br>4. If `with_scale`: `x_normed *= weight`. | `sqrt`, `pow` (from `math.h`) |

### **🔹 Rotary Position Embedding (RoPE)**

| **Operation**                 | **C Implementation Notes**                                                          | **Dependencies**                    |
|-------------------------------|-------------------------------------------------------------------------------------|-------------------------------------|
| Inverse Frequency Calculation | Precompute `inv_freq` as `1.0 / (base ^ (2i / dim))` for `i = 0, 2, 4, ..., dim-2`. | `pow`, `log` (if `base` is dynamic) |
| Frequency Calculation         | `freqs = inv_freq @ position_ids` (outer product).                                  | Matrix multiplication               |
| Cosine/Sine                   | `cos = cos(freqs)`, `sin = sin(freqs)`. Use `cosf`/`sinf` for float32.              | `math.h` (`cosf`, `sinf`)           |
| Rotate Half                   | Split `x` into two halves, negate the second half, and concatenate.                 | Memory copy                         |
| Apply RoPE                    | `x_rotated = x * cos + rotate_half(x) * sin` (element-wise).                        | Element-wise ops                    |

### **🔹 Attention Mechanism**

| **Operation**           | **C Implementation Notes**                                                                   | **Dependencies**       |
|-------------------------|----------------------------------------------------------------------------------------------|------------------------|
| Attention Scores        | `scores = Q @ K^T * scaling`. Use GEMM with `alpha = scaling`.                               | BLAS GEMM              |
| Softcap                 | `scores = tanh(scores / softcap) * softcap`. Use `tanhf` for float32.                        | `math.h` (`tanhf`)     |
| Mask Addition           | Add `attention_mask` (broadcast if needed). Handle `-inf` for padding.                       | Element-wise add       |
| Softmax                 | `softmax(x)_i = exp(x_i - max(x)) / sum(exp(x_j - max(x)))` (numerically stable).            | `expf`, `logf`, `maxf` |
| Dropout (Training Only) | Generate random mask with `rand() / RAND_MAX < dropout_prob`. Scale by `1/(1-dropout_prob)`. | `rand()`, `RAND_MAX`   |
| Context Calculation     | `attn_output = softmax @ V`. Use GEMM.                                                       | BLAS GEMM              |
| Repeat KV               | Expand `K` and `V` by repeating along the head dimension.                                    | Memory copy            |
| Transpose + Reshape     | Reinterpret memory layout for `attn_output`.                                                 | None                   |

### **🔹 MLP**

| **Operation**      | **C Implementation Notes**                                                        | **Dependencies**      |
|--------------------|-----------------------------------------------------------------------------------|-----------------------|
| Gate/Up Projection | Two separate GEMMs: `gate = x @ gate_proj`, `up = x @ up_proj`.                   | BLAS GEMM             |
| Activation (GELU)  | `gelu(x) = 0.5 * x * (1 + tanh(sqrt(2/π) * (x + 0.044715 * x^3)))` (approximate). | `tanhf`, `powf`       |
| Gating             | `intermediate = gate * up` (element-wise).                                        | Element-wise multiply |
| Down Projection    | `mlp_output = intermediate @ down_proj`.                                          | BLAS GEMM             |

### **🔹 MoE (Mixture of Experts)**

| **Operation**           | **C Implementation Notes**                                                                           | **Dependencies**            |
|-------------------------|------------------------------------------------------------------------------------------------------|-----------------------------|
| Router Norm             | RMSNorm (no scale).                                                                                  | Same as RMSNorm             |
| Router Scaling          | `h_scaled = h_norm * scale * (1/sqrt(H))`.                                                           | Element-wise multiply       |
| Router Projection       | `scores = h_scaled @ router_proj`.                                                                   | BLAS GEMM                   |
| Softmax                 | Same as attention softmax.                                                                           | `expf`, `logf`              |
| Top-K                   | Use a **priority queue** or `std::partial_sort` to find top-K experts per token.                     | Custom top-K implementation |
| Normalize Top-K Weights | `weights = weights / sum(weights)`.                                                                  | Element-wise ops            |
| Per-Expert Scaling      | `weights = weights * per_expert_scale[index]`.                                                       | Element-wise multiply       |
| Expert Mask             | Create a sparse mask to identify active experts.                                                     | Bitmask or boolean array    |
| Expert Computation      | For each active expert, compute `gate = x @ gate_up_proj`, `up = x @ up_proj`, `h = act(gate) * up`. | BLAS GEMM, activation       |
| Down Projection         | `h = h @ down_proj`.                                                                                 | BLAS GEMM                   |
| Weight Scaling          | `h = h * top_k_weights[token_idx, expert_idx]`.                                                      | Element-wise multiply       |
| Accumulate Outputs      | Use **scatter-add** to accumulate `h` into `final_hidden_states`.                                    | Custom scatter-add kernel   |

### **🔹 Residual Connections & Layer Scaling**

| **Operation**     | **C Implementation Notes**             | **Dependencies**      |
|-------------------|----------------------------------------|-----------------------|
| Residual Addition | `h = h + residual` (element-wise).     | Element-wise add      |
| Layer Scaling     | `h = h * layer_scalar` (element-wise). | Element-wise multiply |

### **🔹 Embedding & Output**

| **Operation**     | **C Implementation Notes**                                                         | **Dependencies** |
|-------------------|------------------------------------------------------------------------------------|------------------|
| Token Embedding   | `hidden_states = embeddings[input_ids] * embed_scale`. Use a **2D array lookup**.  | Memory access    |
| Embedding Scale   | `embed_scale = sqrt(hidden_size)`. Precompute.                                     | `sqrtf`          |
| LM Head           | `logits = hidden_states @ lm_head.weight`.                                         | BLAS GEMM        |
| Logit Softcapping | `logits = logits / softcap`, `logits = tanh(logits)`, `logits = logits * softcap`. | `tanhf`          |

### **🔹 KV Cache**

| **Operation**    | **C Implementation Notes**                                                                         | **Dependencies** |
|------------------|----------------------------------------------------------------------------------------------------|------------------|
| Cache Update     | Concatenate new `K` and `V` to the existing cache. Use **dynamic arrays** or preallocated buffers. | Memory copy      |
| Cache Retrieval  | Return cached `K` and `V` for the current layer.                                                   | Memory access    |
| Shared KV States | Store full-length `K` and `V` for shared layers.                                                   | Memory copy      |

### **🔹 Activation Functions**

| **Function** | **C Implementation**                                                    | **Dependencies**   |
|--------------|-------------------------------------------------------------------------|--------------------|
| GELU         | `0.5 * x * (1 + tanh(sqrt(2/π) * (x + 0.044715 * x^3)))` (approximate). | `tanhf`, `powf`    |
| Softmax      | `exp(x_i - max(x)) / sum(exp(x_j - max(x)))` (numerically stable).      | `expf`, `logf`     |
| Tanh         | Use `tanhf` for float32.                                                | `math.h` (`tanhf`) |

### **🔹 Utility Functions**

| **Function**     | **C Implementation**                                                    | **Dependencies**           |
|------------------|-------------------------------------------------------------------------|----------------------------|
| Clamp            | `x = max(min(x, max_val), min_val)`.                                    | `fmaxf`, `fminf`           |
| Top-K            | Use a **min-heap** of size `K` to track top-K values.                   | Custom heap implementation |
| One-Hot          | Create a sparse matrix with `1` at the index of `top_k_index`.          | Memory allocation          |
| Indexed Addition | `output[index] += value`. Use **scatter-add** for efficiency.           | Custom kernel              |
| Repeat KV        | Copy `K` and `V` `num_key_value_groups` times along the head dimension. | Memory copy                |

<!-- HUMAN-END -->

## Clarification Questions

1. **bf16 library operations**: The human spec states we must rely on the existing bf16 library and mention any missing
   operations. What operations are missing?
    - **Answer**: The current bf16 library provides scalar arithmetic (add, sub, mul, div), comparisons, and array
      operations (add_arr, sub_arr, mul_arr, div_arr). Missing operations for linalg:
        - No reduction operations (sum, mean, max, min)
        - No element-wise mathematical functions (sqrt, exp, log, pow, tanh, sin, cos)
        - No type conversion to/from float for intermediate computations
    - **Decision**: We need to extend the bf16 library with these operations OR implement them as part of linalg. Given
      the type-specific requirement, linalg should provide `bf16` versions of these operations.

2. **BLAS references in function list**: The function list table mentions BLAS (`cblas_sgemm`, `cblas_dgemv`) as
   dependencies and suggests using BLAS or custom GEMM kernel. However, the General considerations explicitly state "
   Except bf16 and arena, no other third party library is allowed". How to reconcile this?
    - **Answer**: The BLAS references in the function list are suggestions for implementation approaches, but the
      General considerations take precedence. We must implement custom GEMM/GEMV kernels without any BLAS or other
      third-party library dependencies.

3. **"For now, only the bf16 type must be supported"**: Does this mean the library should be designed to potentially
   support other types (float32, etc.) in the future?
    - **Answer**: The naming convention (`operation_bf16`) suggests type-specific functions. The library should be
      designed with extensibility in mind but only implement bf16 support for now.

4. **"ignore the existing llm code"**: The human spec explicitly says to ignore the existing llm code. How does this
   affect our implementation?
    - **Answer**: We must NOT use, reference, or modify any code in `src/llm/` (RMSNorm, RoPE, Attention, MLP, etc.).
      The linalg library is a completely new, standalone implementation. Any existing float-based Matrix operations are
      irrelevant for this bf16-focused library.

5. **Model agnostic requirement**: The human spec states "This library MUST be model agnostic". What does this mean for
   the implementation?
    - **Answer**: The linalg library must NOT contain any model-specific logic, assumptions, or hardcoded values. This
      means:
        - No hardcoded tensor dimensions (e.g., hidden_size, num_heads, num_layers)
        - No model-specific formulas or constants
        - No assumptions about model architecture (e.g., transformer vs. MoE vs. RNN)
        - All operations must be generic and work with any tensor shapes that are valid for the operation
        - The library provides building blocks that can be composed to implement any model

7. **OOP naming convention**: The human spec states "function name must start by the object they are related to". How
   should this be applied consistently?
    - **Answer**: All functions must follow the pattern `<Object>_<operation>` where:
        - `<Object>` is the primary data structure the function operates on (Tensor, Matrix, Vector, KVCache, etc.)
        - `<operation>` describes the action being performed
        - For type-specific functions, the type suffix (`_bf16`) comes at the end: `Tensor_matmul_bf16`,
          `Matrix_transpose_bf16`, etc.
        - This ensures consistent, predictable naming that groups related operations together

## Functional specification

### Feature goal

Create a new standalone static library named `callm_linalg` that provides linear algebra operations specifically for the
bf16
type. This library will serve as the foundation for all matrix and vector computations in the CaLLM runtime, replacing
the existing float-based matrix operations with bf16-native implementations.

### Scope

- **Included**:
    - Matrix and vector data structures for bf16 type
    - Core linear algebra operations (GEMM, GEMV, transpose, reshape, concat, split)
    - Normalization operations (RMSNorm)
    - Rotary Position Embedding (RoPE) operations
    - Attention mechanism operations (scores, softmax, mask, dropout, context)
    - MLP operations (gate/up projection, GELU activation, gating, down projection)
    - MoE operations (router norm, scaling, projection, top-k, expert computation)
    - Residual connections and layer scaling
    - Embedding and output operations (token embedding, LM head)
    - KV cache operations
    - Activation functions (GELU, Softmax, Tanh)
    - Utility functions (clamp, top-k, one-hot, indexed addition, repeat KV)
    - Element-wise mathematical operations (sqrt, exp, log, pow, sin, cos, tanh)

- **Excluded**:
    - GPU support (explicitly excluded per General considerations: "No GPU", "single CPU")
    - BLAS or other external library dependencies (explicitly excluded per General considerations: "Except bf16 and
      arena, no other third party library is allowed")
    - Direct memory allocation (must use Arena)
    - Type support other than bf16 (for now)
    - Direct file I/O operations (delegated later to safetensors library)
    - Model loading/saving (handled later via safetensors integration)
    - Tokenizer integration

### Library characteristics

- **Consistent naming convention**: All functions must follow OOP-style naming: `<Object>_<operation>_<type>`, e.g.,
  `Tensor_matmul_bf16`, `Matrix_transpose_bf16`, `Vector_add_bf16`
- **Memory management**: All memory must be allocated through the Arena library
- **No direct allocation**: No `malloc`, `calloc`, `mmap`, or `free` calls allowed
- **bf16 dependency**: Must use the existing bf16 library for bf16 type and basic operations
- **Safetensors integration**: Will be integrated later to the existing safetensors library for all tensor I/O
  operations, but out of scope for this specification.
- **Model agnostic**: Must NOT contain any model-specific logic, assumptions, or hardcoded values
- **Thread safety**: Functions should be thread-safe where applicable (no global state)

### Model agnostic design

The linalg library must be completely model agnostic. This means:

- **No hardcoded dimensions**: Functions must accept tensor dimensions as parameters or from the Tensor structure
  itself. No assumptions about specific dimension sizes (e.g., hidden_size = 4096).
- **No model-specific operations**: All operations are generic linear algebra operations (matmul, normalize, softmax,
  etc.) that can be composed to build any model architecture.
- **Generic tensor operations**: Functions operate on tensors based on their shape and dtype, not based on what model
  component they represent.
- **No architectural assumptions**: The library does NOT assume transformer architecture, MoE, RNN, CNN, or any other
  specific model type.
- **Composable building blocks**: The library provides low-level operations that can be combined in any way to implement
  different model architectures.

### Data structures

The library must define the following core data structures:

#### Tensor structure

```c
typedef struct Tensor Tensor;

// Opaque structure for bf16 tensors
// Internally contains: dimensions, strides, data pointer (bf16_t*), arena reference
```

The Tensor structure must:

- Support arbitrary dimensionality (at least 1D, 2D, 3D, 4D)
- Store data in row-major (C-contiguous) format
- Track ownership of the underlying memory (owned vs. view)
- Support views (slices, transposes, reshapes) without copying data

#### Matrix and Vector

For convenience and performance, specialized types for common cases:

```c
typedef struct Matrixbf16 Matrixbf16;  // 2D tensor
typedef struct Vectorbf16 Vectorbf16;  // 1D tensor
```

However, these can be type-aliased or implemented as Tensor with dimension checks.

### Operation categories and functions

#### Type definition and constants

```c
// Re-export bf16_t from bf16 library for convenience
typedef bf16_t linalg_bf16_t;

// Define special values
#define LINALG_BF16_ZERO BF16_ZERO
#define LINALG_BF16_ONE bf16_from_bits(0x3F80)  // 1.0 in bf16
```

#### Memory management

```c
// Allocate a new tensor through arena
Tensor *Tensor_new_bf16(Arena *arena, const char *name, int ndim, const size_t *dims);

// Create a tensor view (no allocation)
Tensor *Tensor_view_bf16(Tensor *source, int ndim, const size_t *dims, const size_t *offsets);

// Free tensor resources (only if owned)
void Tensor_free_bf16(Tensor *t);
```

#### Element-wise operations

```c
// Addition: C = A + B (element-wise)
void Tensor_add_bf16(Tensor *C, const Tensor *A, const Tensor *B);

// Subtraction: C = A - B
void Tensor_sub_bf16(Tensor *C, const Tensor *A, const Tensor *B);

// Multiplication: C = A * B
void Tensor_mul_bf16(Tensor *C, const Tensor *A, const Tensor *B);

// Division: C = A / B
void Tensor_div_bf16(Tensor *C, const Tensor *A, const Tensor *B);

// Scalar multiplication: C = A * scalar
void Tensor_mul_scalar_bf16(Tensor *C, const Tensor *A, bf16_t scalar);

// Scalar addition: C = A + scalar
void Tensor_add_scalar_bf16(Tensor *C, const Tensor *A, bf16_t scalar);
```

#### Reduction operations

```c
// Sum all elements
bf16_t Tensor_sum_bf16(const Tensor *A);

// Sum along dimension
void Tensor_sum_along_bf16(Tensor *C, const Tensor *A, int dim);

// Mean all elements
bf16_t Tensor_mean_bf16(const Tensor *A);

// Mean along dimension
void Tensor_mean_along_bf16(Tensor *C, const Tensor *A, int dim);

// Max all elements
bf16_t Tensor_max_bf16(const Tensor *A);

// Max along dimension
void Tensor_max_along_bf16(Tensor *C, const Tensor *A, int dim);

// Min all elements
bf16_t Tensor_min_bf16(const Tensor *A);

// Min along dimension
void Tensor_min_along_bf16(Tensor *C, const Tensor *A, int dim);
```

#### Mathematical functions

```c
// Square root (element-wise)
void Tensor_sqrt_bf16(Tensor *C, const Tensor *A);

// Exponential (element-wise)
void Tensor_exp_bf16(Tensor *C, const Tensor *A);

// Natural logarithm (element-wise)
void Tensor_log_bf16(Tensor *C, const Tensor *A);

// Power: C = A ^ B (element-wise)
void Tensor_pow_bf16(Tensor *C, const Tensor *A, const Tensor *B);

// Tanh (element-wise)
void Tensor_tanh_bf16(Tensor *C, const Tensor *A);

// Sin (element-wise)
void Tensor_sin_bf16(Tensor *C, const Tensor *A);

// Cos (element-wise)
void Tensor_cos_bf16(Tensor *C, const Tensor *A);
```

#### Core Linear Algebra

```c
// Matrix multiplication: C = A @ B (GEMM)
// C dimensions: (m, n), A dimensions: (m, k), B dimensions: (k, n)
void Tensor_matmul_bf16(Tensor *C, const Tensor *A, const Tensor *B);

// Matrix-vector multiplication: y = A @ x (GEMV)
// y dimensions: (m,), A dimensions: (m, n), x dimensions: (n,)
void Tensor_matvec_bf16(Tensor *y, const Tensor *A, const Tensor *x);

// Transpose: C = A^T
void Tensor_transpose_bf16(Tensor *C, const Tensor *A);

// In-place transpose
void Tensor_transpose_inplace_bf16(Tensor *A);

// Reshape: C = reshape(A, new_shape)
// Returns a view if possible, otherwise allocates new memory
Tensor *Tensor_reshape_bf16(const Tensor *A, int ndim, const size_t *new_dims);

// Concatenate along dimension
void Tensor_concat_bf16(Tensor *C, const Tensor *A, const Tensor *B, int dim);

// Split tensor along dimension
// Returns array of Tensor pointers (caller must free array, not tensors)
Tensor **Tensor_split_bf16(const Tensor *A, int dim, size_t *num_splits);
```

#### Normalization

```c
// RMSNorm: y = (x * weight) / sqrt(mean(x^2) + eps)
// If weight is NULL, skip scaling
void Tensor_rmsnorm_bf16(Tensor *y, const Tensor *x, const Tensor *weight, bf16_t eps);
```

#### Rotary Position Embedding (RoPE)

```c
// Precompute inverse frequencies
void Tensor_rope_inv_freq_bf16(Tensor *inv_freq, int dim, bf16_t base);

// Compute frequencies: freqs = inv_freq @ position_ids
void Tensor_rope_freqs_bf16(Tensor *freqs, const Tensor *inv_freq, const Tensor *position_ids);

// Compute cos and sin of frequencies
void Tensor_rope_cos_sin_bf16(Tensor *cos, Tensor *sin, const Tensor *freqs);

// Rotate half: split x into two halves, negate second half, concatenate
void Tensor_rope_rotate_half_bf16(Tensor *y, const Tensor *x);

// Apply RoPE: y = x * cos + rotate_half(x) * sin
void Tensor_rope_apply_bf16(Tensor *y, const Tensor *x, const Tensor *cos, const Tensor *sin);
```

#### Attention Mechanism

```c
// Attention scores: scores = Q @ K^T * scaling
void Tensor_attention_scores_bf16(Tensor *scores, const Tensor *Q, const Tensor *K, bf16_t scaling);

// Softcap: scores = tanh(scores / softcap) * softcap
void Tensor_softcap_bf16(Tensor *y, const Tensor *x, bf16_t softcap);

// Add attention mask (broadcast if needed)
void Tensor_attention_mask_add_bf16(Tensor *scores, const Tensor *mask);

// Softmax (numerically stable)
void Tensor_softmax_bf16(Tensor *y, const Tensor *x, int dim);

// Dropout (training only)
void Tensor_dropout_bf16(Tensor *y, const Tensor *x, bf16_t dropout_prob);

// Context calculation: attn_output = softmax @ V
void Tensor_attention_context_bf16(Tensor *attn_output, const Tensor *softmax, const Tensor *V);

// Repeat KV: expand K and V along head dimension
void Tensor_repeat_kv_bf16(Tensor *K_out, Tensor *V_out, const Tensor *K, const Tensor *V, int num_groups);
```

#### MLP

```c
// Gate projection: gate = x @ gate_proj
void Tensor_mlp_gate_bf16(Tensor *gate, const Tensor *x, const Tensor *gate_proj);

// Up projection: up = x @ up_proj
void Tensor_mlp_up_bf16(Tensor *up, const Tensor *x, const Tensor *up_proj);

// GELU activation
void Tensor_gelu_bf16(Tensor *y, const Tensor *x);

// Gating: intermediate = gate * up
void Tensor_mlp_gating_bf16(Tensor *intermediate, const Tensor *gate, const Tensor *up);

// Down projection: mlp_output = intermediate @ down_proj
void Tensor_mlp_down_bf16(Tensor *mlp_output, const Tensor *intermediate, const Tensor *down_proj);
```

#### MoE (Mixture of Experts)

```c
// Router norm (RMSNorm without scale)
void Tensor_moe_router_norm_bf16(Tensor *y, const Tensor *x, bf16_t eps);

// Router scaling: h_scaled = h_norm * scale * (1/sqrt(H))
void Tensor_moe_router_scaling_bf16(Tensor *h_scaled, const Tensor *h_norm, bf16_t scale, int H);

// Router projection: scores = h_scaled @ router_proj
void Tensor_moe_router_proj_bf16(Tensor *scores, const Tensor *h_scaled, const Tensor *router_proj);

// Top-K selection
// Returns top_k_indices (shape: [batch, top_k]) and top_k_weights (shape: [batch, top_k])
void Tensor_moe_topk_bf16(Tensor *top_k_weights, Tensor *top_k_indices, const Tensor *scores, int top_k);

// Normalize top-K weights
void Tensor_moe_normalize_weights_bf16(Tensor *normalized, const Tensor *weights);

// Per-expert scaling
void Tensor_moe_per_expert_scaling_bf16(Tensor *scaled, const Tensor *weights, const Tensor *expert_scale);

// Expert computation
void Tensor_moe_expert_compute_bf16(Tensor *h, const Tensor *x, const Tensor *gate_up_proj, 
                                       const Tensor *up_proj, const Tensor *down_proj);

// Weight scaling
void Tensor_moe_weight_scaling_bf16(Tensor *scaled, const Tensor *h, const Tensor *weights);

// Accumulate outputs (scatter-add)
void Tensor_moe_accumulate_bf16(Tensor *output, const Tensor *h, const Tensor *top_k_indices);
```

#### Residual Connections & Layer Scaling

```c
// Residual addition: h = h + residual
void Tensor_residual_add_bf16(Tensor *h, const Tensor *residual);

// Layer scaling: h = h * layer_scalar
void Tensor_layer_scaling_bf16(Tensor *h, bf16_t scalar);
```

#### Embedding & Output

```c
// Token embedding lookup
// embeddings: [vocab_size, hidden_size]
// input_ids: [batch_size]
// hidden_states: [batch_size, hidden_size]
void Tensor_token_embedding_bf16(Tensor *hidden_states, const Tensor *embeddings, const Tensor *input_ids, bf16_t embed_scale);

// LM Head: logits = hidden_states @ lm_head.weight
void Tensor_lm_head_bf16(Tensor *logits, const Tensor *hidden_states, const Tensor *lm_head_weight);

// Logit softcapping
void Tensor_logit_softcap_bf16(Tensor *logits, bf16_t softcap);
```

#### KV Cache

```c
// KV Cache structure
typedef struct KVCache KVCache;

// Create new KV cache
KVCache *KVCache_new_bf16(Arena *arena, size_t max_seq_len, size_t num_heads, size_t head_dim);

// Free KV cache
void KVCache_free_bf16(KVCache *cache);

// Update cache with new K and V
void KVCache_update_bf16(KVCache *cache, const Tensor *K, const Tensor *V);

// Retrieve cached K and V
void KVCache_get_bf16(const KVCache *cache, Tensor **K, Tensor **V);

// Shared KV states (for shared layers)
void KVCache_shared_bf16(KVCache *cache, const Tensor *K, const Tensor *V);
```

#### Utility Functions

```c
// Clamp: x = max(min(x, max_val), min_val)
void Tensor_clamp_bf16(Tensor *y, const Tensor *x, bf16_t min_val, bf16_t max_val);

// Top-K selection (general purpose)
void Tensor_topk_bf16(Tensor *values, Tensor *indices, const Tensor *x, int k, int dim);

// One-Hot encoding
void Tensor_one_hot_bf16(Tensor *output, const Tensor *indices, int depth);

// Indexed addition: output[index] += value
void Tensor_indexed_add_bf16(Tensor *output, const Tensor *indices, const Tensor *values);

// Repeat KV
void Tensor_repeat_bf16(Tensor *output, const Tensor *input, int repeats, int dim);
```

### Missing bf16 operations

The following operations are NOT provided by the current bf16 library and must be implemented as part of linalg:

1. **Reduction operations**: sum, mean, max, min
2. **Mathematical functions**: sqrt, exp, log, pow, tanh, sin, cos
3. **Comparison reductions**: argmax, argmin

These can be implemented either as:

- Extensions to the bf16 library (preferred for reusability)
- Private helper functions within linalg

**Decision**: For the scope of this feature, implement these as private helper functions within linalg. A separate
feature can later extend the bf16 library.

### Library structure

```
src/linalg/
├── include/
│   ├── linalg.h              # Public header with all function declarations
│   └── linalg_types.h        # Type definitions (Tensor, Matrixbf16, Vectorbf16, KVCache)
├── tensor/
│   ├── tensor.c              # Tensor implementation
│   ├── tensor.h              # Tensor private header
│   ├── tensor_view.c         # Tensor view operations
│   └── tensor_ops.c          # Tensor element-wise operations
├── matmul/
│   ├── gemm.c                # General matrix multiplication (GEMM)
│   ├── gemm_bf16.c           # bf16-specific GEMM
│   └── gemv.c                # General matrix-vector multiplication
├── normalization/
│   └── rmsnorm.c             # RMSNorm implementation
├── attention/
│   ├── scores.c              # Attention score computation
│   ├── softmax.c             # Softmax implementation
│   └── context.c             # Attention context computation
├── rope/
│   ├── inv_freq.c            # Inverse frequency calculation
│   ├── freqs.c               # Frequency calculation
│   ├── cos_sin.c             # Cosine/sine computation
│   ├── rotate_half.c         # Rotate half implementation
│   └── apply.c               # RoPE application
├── mlp/
│   ├── gate_up.c             # Gate and up projections
│   ├── gelu.c                # GELU activation
│   ├── gating.c              # Gating operation
│   └── down.c                # Down projection
├── moe/
│   ├── router.c              # Router operations
│   ├── topk.c                # Top-K implementation
│   └── expert.c              # Expert computation
├── embedding/
│   ├── token.c               # Token embedding
│   └── lm_head.c             # LM head
├── cache/
│   └── kv_cache.c            # KV cache implementation
├── utils/
│   ├── clamp.c               # Clamp function
│   ├── topk.c                # General top-K
│   ├── one_hot.c             # One-hot encoding
│   ├── indexed_add.c         # Indexed addition
│   └── repeat.c              # Repeat operation
├── math/
│   ├── sqrt.c                # Square root
│   ├── exp.c                 # Exponential
│   ├── log.c                 # Logarithm
│   ├── pow.c                 # Power
│   ├── tanh.c                # Hyperbolic tangent
│   ├── sin.c                 # Sine
│   └── cos.c                 # Cosine
└── CMakeLists.txt
```

### Build requirements

- Compile as a static library (`liblinalg.a`)
- Dependencies: `callm_arena`, `callm_bf16`, and  `callm_shared` standard C library (`math.h`)
- C99 or later
- No external dependencies (no BLAS, no OpenBLAS, no Intel MKL)
- Must be buildable with CMake
- Must not depend on safetensors library

### Performance considerations

Given the CPU-only constraint and bf16 type, performance optimizations must include:

1. **Cache locality**: Optimize memory access patterns for cache efficiency
2. **SIMD**: Leverage AVX2 (and potentially AVX-512 if available) for vectorized operations
3. **Parallelism**: Use OpenMP or pthreads for multi-threaded operations where beneficial
4. **Blocking**: Implement blocked GEMM for better cache utilization
5. **Fused operations**: Combine multiple operations (e.g., matmul + residual add) to reduce memory bandwidth
6. **bf16-specific optimizations**: Exploit bf16's 8-bit exponent (same as float32) for efficient conversions

### Numerics and precision

1. **bf16 precision**: With only 7 mantissa bits, careful attention must be paid to:
    - Order of operations to minimize rounding errors
    - Accumulation precision (consider using float32 accumulators for sums)
    - Denormalized number handling

2. **Numerical stability**: Implement numerically stable versions of:
    - Softmax (subtract max before exp)
    - RMSNorm (add epsilon before division)
    - GELU (use approximate formula provided)

## Risks and limitations

### Performance risks

1. **bf16 software emulation overhead**: All bf16 operations are software-emulated. Matrix operations (especially GEMM)
   will be significantly slower than hardware-accelerated float32 operations. Mitigation: Use aggressive blocking, SIMD,
   and parallelism.

2. **No BLAS**: Not using optimized BLAS libraries means we must implement highly optimized GEMM kernels ourselves.
   Mitigation: Study existing high-performance GEMM implementations (e.g., BLIS, OpenBLAS) for algorithmic techniques.

3. **Memory bandwidth**: bf16 uses half the memory of float32, but we may need to convert to float32 for intermediate
   computations, negating this benefit. Mitigation: Perform as many operations as possible in bf16; only convert to
   float32 when absolutely necessary for precision.

4. **SIMD complexity**: AVX2 operates on 256 bits = 16 bf16 values. Implementing efficient bf16 arithmetic with AVX2
   requires careful bit manipulation. Mitigation: Use existing bf16 SIMD implementations from the bf16 library.

### Correctness risks

1. **Numerical accuracy**: bf16 has limited precision. Chaining many operations may lead to unacceptable accuracy loss.
   Mitigation: Use float32 accumulators for reductions (sums, means), implement operations carefully to minimize
   rounding.

2. **Edge cases**: Special values (NaN, Inf, -Inf, zero) must be handled correctly in all operations. Mitigation:
   Implement comprehensive unit tests for edge cases.

3. **Type consistency**: All operations must correctly handle bf16 type without implicit conversions. Mitigation: Use
   strong typing, avoid mixing types.

4. **Memory management**: Using Arena for all memory means we cannot use standard patterns (malloc/free). Mitigation:
   Design careful lifecycle management; document ownership clearly.

5. **Dimension checking**: Matrix operations must validate dimensions at runtime (in debug mode) or rely on caller
   correctness (in release mode). Mitigation: Use assert-like macros that are compiled out in release builds.

### Design limitations

1. **bf16-only**: The library only supports bf16 for now. Future support for float32, int8, etc., will require
   duplicating all operations or creating a templated system.

2. **Arena dependency**: Tight coupling with Arena means the library cannot be used independently. Mitigation: This is
   by design; document clearly.

3. **No dynamic shapes**: Tensor dimensions must be known at tensor creation time. Dynamic resizing is not supported.
   Mitigation: Design for pre-allocated tensors with views for dynamic behavior.

4. **View lifecycle**: Tensor views share memory with their parent. The parent must outlive all views. Mitigation:
   Document ownership semantics clearly; consider reference counting.

### Resource constraints

1. **Memory**: Must work on machines with only 16GB RAM. Large matrix multiplications may require significant memory.
   Mitigation: Implement operations in a memory-efficient manner; support batching/blocking for large operations.

2. **Single CPU**: Must work on a single CPU (no distributed computing). Mitigation: Use multi-threading for parallelism
   within a single process.

3. **No GPU**: Cannot offload computations to GPU. All operations must be CPU-bound. Mitigation: This is a hard
   constraint; focus on CPU optimization.

### Implementation complexity

1. **GEMM implementation**: High-performance GEMM is complex. Mitigation: Start with a simple implementation, then
   optimize incrementally.

2. **MoE top-k**: Efficient top-k selection for MoE routing is non-trivial. Mitigation: Implement a simple O(n log k)
   algorithm using a priority queue.

3. **KV cache**: Efficient cache management for variable-length sequences is complex. Mitigation: Start with a simple
   pre-allocated buffer approach.

4. **bf16 math functions**: Implementing sqrt, exp, log, sin, cos for bf16 requires careful approximation. Mitigation:
   Convert to float32, use standard math library, convert back to bf16.

## Clarified decisions for this feature

1. **CPU-only**: The General considerations explicitly state "This library MUST be able to work on a single CPU" and "No
   GPU". All operations must be CPU-bound and optimized for single-CPU performance.

2. **No third-party libraries**: The General considerations state "Except bf16 and arena, no other third party library
   is allowed". We will NOT use BLAS, OpenBLAS, Intel MKL, or any other external linear algebra library. All linalg
   computational
   operations will be implemented from scratch, no I/O operations needed.

3. **OOP naming convention**: Per the General considerations, "function name must start by the object they are related
   to". All functions follow the pattern `<Object>_<operation>_<type>` (e.g., `Tensor_matmul_bf16`,
   `KVCache_update_bf16`, `Matrix_transpose_bf16`). This ensures a consistent, predictable API where related operations
   are grouped by their primary data structure.

4. **bf16-only for now**: The library will only support bf16 type. All functions are type-specific with `_bf16` suffix.

5. **Arena-based memory**: All memory allocation must go through the Arena library. No direct malloc/free.

6. **Math functions**: For bf16 mathematical functions (sqrt, exp, log, pow, tanh, sin, cos), we will:
    - Convert bf16 to float32
    - Use standard C math library (`math.h`)
    - Convert result back to bf16
    - This is acceptable because these are element-wise operations and the conversion overhead is negligible compared to
      the computation.

7. **GEMM accumulation**: For matrix multiplication, we will use float32 accumulators to maintain precision, then
   convert final result to bf16.

8. **Ignore existing llm code**: Per the explicit instruction "ignore the existing llm code", this linalg library is a
   completely NEW, standalone implementation. We must NOT use, reference, or modify any code in `src/llm/`. The existing
   float-based Matrix operations are irrelevant for this bf16-focused library.

9. **Model agnostic**: Per the General considerations, "This library MUST be model agnostic". The library will:
    - Not contain any hardcoded tensor dimensions or model-specific constants
    - Not make assumptions about model architecture (transformer, MoE, RNN, etc.)
    - Provide only generic, composable linear algebra operations
    - Allow any valid tensor shapes for operations

10. **Library organization**: The library is organized by operation category (as shown in the structure above) with a
    flat public API in `linalg.h`.

11. **Testing**: Each operation category will have corresponding unit tests. Tests will verify correctness against
    float32 reference implementations.

12. **Missing bf16 operations**: The operations listed in the function tables require bf16 mathematical functions not in
    the current bf16 library: sqrt, exp, log, pow, tanh, sin, cos. These will be implemented within linalg as private
    helpers using float32 conversion.
