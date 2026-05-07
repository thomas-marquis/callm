# 003_maths

<!-- HUMAN-START -->

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

## Other considerations

- Use the memory Arena to manage memory, no direct malloc, mmap or free allowed
- This library MUST be able to work on a single GPU. Levreage all possible techniques to achieve this (parallelism,
  pipelining, threads, etc).
- No GPU
- The library works with bf16 numbers.

<!-- HUMAN-END -->

## **Performance Considerations for C**

1. **BLAS Acceleration**:
    - Use **OpenBLAS** or **Intel MKL** for matrix multiplications (`GEMM`, `GEMV`).
    - For embedded systems, consider **ARM NEON** or **SSE/AVX** intrinsics for small matrices.

2. **Memory Layout**:
    - Use **row-major** (C-style) or **column-major** (Fortran-style) consistently. BLAS typically uses column-major.
    - For GEMM, ensure matrices are **contiguous** in memory.

3. **Numerical Stability**:
    - Use **Kahan summation** for softmax to avoid overflow.
    - For RMSNorm, compute `mean_squared` in **float32** even if input is `bfloat16` (emulate in C if needed).

4. **Parallelism**:
    - Parallelize **batch processing** (e.g., process each sequence in the batch in parallel).
    - Parallelize **head processing** in attention (each head can be computed independently).
    - Use **OpenMP** for multi-threading.

5. **Quantization**:
    - For inference, consider **8-bit quantization** (e.g., `int8` for weights, `float32` for activations).
    - Use **QGEMM** (quantized GEMM) for matrix multiplications.

6. **Cache Optimization**:
    - Reuse buffers for intermediate results (e.g., `attn_weights`, `gate`, `up`).
    - Preallocate memory for `KVCache` to avoid dynamic allocations during generation.

7. **MoE Optimization**:
    - Only compute **active experts** for each token (sparse computation).
    - Use **custom kernels** for scatter-add to accumulate expert outputs.

8. **RoPE Optimization**:
    - Precompute `cos` and `sin` for all possible `position_ids` (up to `max_seq_len`).
    - Cache `inv_freq` and reuse for all layers.

