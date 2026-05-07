## **1. List of All Mathematical Operations for Inference**

### **A. Embedding Layer**

| **Operation**   | **Math Formula**                                                | **Shape**   | **Notes**                               |
|-----------------|-----------------------------------------------------------------|-------------|-----------------------------------------|
| Token Embedding | `hidden_states = input_ids @ embed_tokens.weight * embed_scale` | `(B, S, H)` | `embed_scale = sqrt(H)` (H=hidden_size) |
| Embedding Scale | `embed_scale = sqrt(hidden_size)`                               | Scalar      | Applied to all token embeddings         |

### **B. Positional Embeddings (RoPE)**

| **Operation**                 | **Math Formula**                                                                          | **Shape**       | **Notes**                                                                  |
|-------------------------------|-------------------------------------------------------------------------------------------|-----------------|----------------------------------------------------------------------------|
| Inverse Frequency Calculation | `inv_freq = 1.0 / (base ** (torch.arange(0, dim, 2, dtype=torch.float32) / dim))`         | `(dim/2,)`      | `base = rope_theta` (e.g., 500000.0)                                       |
| Frequency Calculation         | `freqs = inv_freq_expanded @ position_ids_expanded`                                       | `(B, S, dim/2)` | `inv_freq_expanded`: `(B, dim/2, 1)`, `position_ids_expanded`: `(B, 1, S)` |
| Cosine/Sine Embeddings        | `cos = freqs.cos() * attention_scaling`, `sin = freqs.sin() * attention_scaling`          | `(B, S, dim)`   | `attention_scaling` is usually 1.0 for default RoPE                        |
| Rotate Half                   | `rotate_half(x) = torch.cat((-x[..., x.shape[-1]//2:], x[..., :x.shape[-1]//2]), dim=-1)` | `(B, S, dim)`   | Splits `x` in half, negates the second half, and concatenates              |
| Apply RoPE                    | `x_rotated = (x * cos) + (rotate_half(x) * sin)`                                          | `(B, S, dim)`   | Applied to `query_states` and `key_states`                                 |
| Multidimensional RoPE         | Split `x`, `cos`, `sin` into `ndim` parts, apply RoPE to each, then concatenate           | `(B, S, dim)`   | Used for sliding window attention (if enabled)                             |

### **C. RMSNorm (Root Mean Square Normalization)**

| **Operation**                | **Math Formula**                                       | **Shape**     | **Notes**                                           |
|------------------------------|--------------------------------------------------------|---------------|-----------------------------------------------------|
| Mean Squared                 | `mean_squared = x.pow(2).mean(-1, keepdim=True) + eps` | `(B, S, 1)`   | `eps = 1e-6` (default)                              |
| Normalization                | `x_normed = x * torch.pow(mean_squared, -0.5)`         | `(B, S, dim)` | Applied in float32, then cast back to input dtype   |
| Scale (if `with_scale=True`) | `x_normed = x_normed * weight`                         | `(B, S, dim)` | `weight` is a learnable parameter of shape `(dim,)` |

**Where Used:**

- `input_layernorm`, `post_attention_layernorm`, `pre_feedforward_layernorm`, `post_feedforward_layernorm`
- `q_norm`, `k_norm`, `v_norm` (in attention)
- `per_layer_projection_norm`, `post_per_layer_input_norm`
- `router.norm`, `embedding_pre_projection_norm`

### **D. Attention Mechanism**

#### **D.1 Projections**

| **Operation**     | **Math Formula**                               | **Shape**                         | **Notes**                                                                                        |
|-------------------|------------------------------------------------|-----------------------------------|--------------------------------------------------------------------------------------------------|
| Query Projection  | `query_states = hidden_states @ q_proj.weight` | `(B, S, num_heads * head_dim)`    | Reshaped to `(B, S, num_heads, head_dim)`                                                        |
| Key Projection    | `key_states = hidden_states @ k_proj.weight`   | `(B, S, num_kv_heads * head_dim)` | Reshaped to `(B, S, num_kv_heads, head_dim)`                                                     |
| Value Projection  | `value_states = hidden_states @ v_proj.weight` | `(B, S, num_kv_heads * head_dim)` | Reshaped to `(B, S, num_kv_heads, head_dim)`. If `attention_k_eq_v`, `value_states = key_states` |
| Output Projection | `attn_output = attn_output @ o_proj.weight`    | `(B, S, num_heads * head_dim)`    | Reshaped to `(B, S, H)`                                                                          |

#### **D.2 Attention Scores**

| **Operation**       | **Math Formula**                                                        | **Shape**                      | **Notes**                                                                |
|---------------------|-------------------------------------------------------------------------|--------------------------------|--------------------------------------------------------------------------|
| Attention Scores    | `attn_weights = (query_states @ key_states.T) * scaling`                | `(B, num_heads, S, S)`         | `scaling = 1.0 / sqrt(head_dim)` (default) or from config                |
| Softcap (optional)  | `attn_weights = tanh(attn_weights / softcap) * softcap`                 | `(B, num_heads, S, S)`         | Only if `softcap` is not `None`                                          |
| Mask Addition       | `attn_weights = attn_weights + attention_mask`                          | `(B, num_heads, S, S)`         | `attention_mask` is broadcastable to `(B, 1, S, S)` or `(B, 1, 1, S)`    |
| Softmax             | `attn_weights = softmax(attn_weights, dim=-1, dtype=torch.float32)`     | `(B, num_heads, S, S)`         | Upcast to float32 for numerical stability, then cast back to input dtype |
| Dropout             | `attn_weights = dropout(attn_weights, p=dropout, training=is_training)` | `(B, num_heads, S, S)`         | Only applied during training                                             |
| Context Calculation | `attn_output = attn_weights @ value_states`                             | `(B, num_heads, S, head_dim)`  |                                                                          |
| Transpose & Reshape | `attn_output = attn_output.transpose(1, 2).reshape(B, S, -1)`           | `(B, S, num_heads * head_dim)` |                                                                          |

#### **D.3 Key/Value Repetition (Multi-Query Attention)**

| **Operation** | **Math Formula**                                           | **Shape**                     | **Notes**                                                                               |
|---------------|------------------------------------------------------------|-------------------------------|-----------------------------------------------------------------------------------------|
| Repeat KV     | `key_states = repeat_kv(key_states, num_key_value_groups)` | `(B, num_heads, S, head_dim)` | Expands `num_kv_heads` to `num_heads` via repetition. See `repeat_kv` function in code. |

### **E. MLP (Feed-Forward Network)**

| **Operation**   | **Math Formula**                               | **Shape**                   | **Notes**                                                                  |
|-----------------|------------------------------------------------|-----------------------------|----------------------------------------------------------------------------|
| Gate Projection | `gate = hidden_states @ gate_proj.weight`      | `(B, S, intermediate_size)` | `intermediate_size = config.intermediate_size * (2 if double_wide else 1)` |
| Up Projection   | `up = hidden_states @ up_proj.weight`          | `(B, S, intermediate_size)` |                                                                            |
| Activation      | `gate = act_fn(gate)`                          | `(B, S, intermediate_size)` | `act_fn` is typically `GELU` (or `silu` for newer models)                  |
| Gating          | `intermediate = gate * up`                     | `(B, S, intermediate_size)` | Element-wise multiplication                                                |
| Down Projection | `mlp_output = intermediate @ down_proj.weight` | `(B, S, hidden_size)`       |                                                                            |

### **F. Mixture of Experts (MoE) (if `enable_moe_block=True`)**

#### **F.1 Router**

| **Operation**           | **Math Formula**                                                                   | **Shape**            | **Notes**                                                             |
|-------------------------|------------------------------------------------------------------------------------|----------------------|-----------------------------------------------------------------------|
| Router Norm             | `hidden_states_normed = norm(hidden_states)`                                       | `(B*S, H)`           | RMSNorm without scale                                                 |
| Router Scaling          | `hidden_states_scaled = hidden_states_normed * scale * scalar_root_size`           | `(B*S, H)`           | `scalar_root_size = 1.0 / sqrt(H)`                                    |
| Expert Scores           | `expert_scores = hidden_states_scaled @ router.proj.weight`                        | `(B*S, num_experts)` |                                                                       |
| Router Softmax          | `router_probabilities = softmax(expert_scores, dim=-1)`                            | `(B*S, num_experts)` |                                                                       |
| Top-K Selection         | `top_k_weights, top_k_index = topk(router_probabilities, k=top_k_experts, dim=-1)` | `(B*S, K)`           | `K = top_k_experts` (e.g., 2)                                         |
| Normalize Top-K Weights | `top_k_weights = top_k_weights / top_k_weights.sum(dim=-1, keepdim=True)`          | `(B*S, K)`           | Ensures weights sum to 1 per token                                    |
| Per-Expert Scaling      | `top_k_weights = top_k_weights * per_expert_scale[top_k_index]`                    | `(B*S, K)`           | `per_expert_scale` is a learnable parameter of shape `(num_experts,)` |

#### **F.2 Experts**

| **Operation**             | **Math Formula**                                                                            | **Shape**                       | **Notes**                                                     |
|---------------------------|---------------------------------------------------------------------------------------------|---------------------------------|---------------------------------------------------------------|
| Expert Mask               | `expert_mask = one_hot(top_k_index, num_classes=num_experts).permute(2, 1, 0)`              | `(num_experts, B*S, K)`         | Identifies which experts are active for which tokens          |
| Expert Hit Detection      | `expert_hit = (expert_mask.sum(dim=(-1, -2)) > 0).nonzero()`                                | `(E,)` where `E <= num_experts` | List of expert indices with at least one token assigned       |
| Expert Input Selection    | `current_state = hidden_states[token_idx]`                                                  | `(num_tokens_for_expert, H)`    | Gathers input tokens for the current expert                   |
| Expert Gate/Up Projection | `gate, up = (current_state @ gate_up_proj[expert_idx]).chunk(2, dim=-1)`                    | `(num_tokens_for_expert, I)`    | `I = intermediate_size` for the expert                        |
| Expert Activation         | `current_hidden_states = act_fn(gate) * up`                                                 | `(num_tokens_for_expert, I)`    |                                                               |
| Expert Down Projection    | `current_hidden_states = current_hidden_states @ down_proj[expert_idx]`                     | `(num_tokens_for_expert, H)`    |                                                               |
| Weight Scaling            | `current_hidden_states = current_hidden_states * top_k_weights[token_idx, top_k_pos, None]` | `(num_tokens_for_expert, H)`    | Scales by router weight                                       |
| Accumulate Outputs        | `final_hidden_states.index_add_(0, token_idx, current_hidden_states)`                       | `(B*S, H)`                      | Sparse update: adds expert outputs to the final output tensor |

### **G. Per-Layer Embeddings (PLE) (if `hidden_size_per_layer_input=True`)**

| **Operation**            | **Math Formula**                                                                             | **Shape**                      | **Notes**                                              |
|--------------------------|----------------------------------------------------------------------------------------------|--------------------------------|--------------------------------------------------------|
| Token-Identity Embedding | `ple_token = embed_tokens_per_layer(input_ids)`                                              | `(B, S, num_layers * ple_dim)` | `ple_dim = hidden_size_per_layer_input` (e.g., 256)    |
| Reshape PLE              | `ple_token = ple_token.reshape(B, S, num_layers, ple_dim)`                                   | `(B, S, num_layers, ple_dim)`  |                                                        |
| Context-Aware Projection | `ple_context = per_layer_model_projection(inputs_embeds) * per_layer_model_projection_scale` | `(B, S, num_layers * ple_dim)` | `per_layer_model_projection_scale = 1.0 / sqrt(H)`     |
| Reshape Context PLE      | `ple_context = ple_context.reshape(B, S, num_layers, ple_dim)`                               | `(B, S, num_layers, ple_dim)`  |                                                        |
| Context PLE Norm         | `ple_context = per_layer_projection_norm(ple_context)`                                       | `(B, S, num_layers, ple_dim)`  | RMSNorm                                                |
| Combine PLE              | `ple = (ple_context + ple_token) * per_layer_input_scale`                                    | `(B, S, num_layers, ple_dim)`  | `per_layer_input_scale = 1.0 / sqrt(2)`                |
| Per-Layer Gate           | `ple_gate = per_layer_input_gate(hidden_states)`                                             | `(B, S, ple_dim)`              | Linear projection                                      |
| Per-Layer Activation     | `ple_gate = act_fn(ple_gate)`                                                                | `(B, S, ple_dim)`              | GELU                                                   |
| Per-Layer Projection     | `ple_proj = ple_gate * ple`                                                                  | `(B, S, ple_dim)`              | Element-wise multiplication with PLE for current layer |
| Final PLE Projection     | `ple_proj = ple_proj @ per_layer_projection.weight`                                          | `(B, S, H)`                    |                                                        |
| PLE Norm                 | `ple_proj = post_per_layer_input_norm(ple_proj)`                                             | `(B, S, H)`                    | RMSNorm                                                |
| Residual + PLE           | `hidden_states = residual + ple_proj`                                                        | `(B, S, H)`                    |                                                        |

### **H. Residual Connections & Layer Scaling**

| **Operation**        | **Math Formula**                               | **Shape**   | **Notes**                                                    |
|----------------------|------------------------------------------------|-------------|--------------------------------------------------------------|
| Residual (Attention) | `hidden_states = residual + attn_output`       | `(B, S, H)` | `residual` is the input to the attention block               |
| Residual (MLP)       | `hidden_states = residual + mlp_output`        | `(B, S, H)` | `residual` is the input to the MLP block                     |
| Layer Scalar         | `hidden_states = hidden_states * layer_scalar` | `(B, S, H)` | `layer_scalar` is a learnable parameter (initialized to 1.0) |

### **I. Final Normalization & Output**

| **Operation**                | **Math Formula**                                                                  | **Shape**            | **Notes**                                       |
|------------------------------|-----------------------------------------------------------------------------------|----------------------|-------------------------------------------------|
| Final Norm                   | `hidden_states = norm(hidden_states)`                                             | `(B, S, H)`          | RMSNorm (same as other layernorms)              |
| Logits Calculation           | `logits = hidden_states @ lm_head.weight`                                         | `(B, S, vocab_size)` | `lm_head.weight` shape: `(H, vocab_size)`       |
| Logit Softcapping (optional) | `logits = logits / softcap`, `logits = tanh(logits)`, `logits = logits * softcap` | `(B, S, vocab_size)` | Only if `final_logit_softcapping` is not `None` |

### **J. Loss Calculation (Training Only)**

| **Operation**      | **Math Formula**                                                       | **Shape** | **Notes**                                                                                   |
|--------------------|------------------------------------------------------------------------|-----------|---------------------------------------------------------------------------------------------|
| Cross-Entropy Loss | `loss = F.cross_entropy(logits.view(-1, vocab_size), labels.view(-1))` | Scalar    | `labels` shape: `(B, S)` or `(B*S,)`. Ignores padding tokens (if `labels` contains `-100`). |

### **K. KV Cache Management (DynamicCache)**

| **Operation**    | **Math Formula**                                              | **Shape**                            | **Notes**                                                                            |
|------------------|---------------------------------------------------------------|--------------------------------------|--------------------------------------------------------------------------------------|
| Cache Update     | `past_key_values.update(key_states, value_states, layer_idx)` | N/A                                  | Concatenates new `key_states` and `value_states` to the cache for the current layer. |
| Cache Retrieval  | `key_states, value_states = past_key_values.get(layer_idx)`   | `(B, num_heads, past_len, head_dim)` | Retrieves cached keys/values for the current layer.                                  |
| Shared KV States | `shared_kv_states[layer_idx] = (key_states, value_states)`    | Dict[int, Tuple[Tensor, Tensor]]     | Stores full-length KV states for layers that share KV (e.g., MoE layers).            |

### **L. Clippable Linear (Optional)**

| **Operation**         | **Math Formula**                                                     | **Shape**             | **Notes**                          |
|-----------------------|----------------------------------------------------------------------|-----------------------|------------------------------------|
| Input Clipping        | `hidden_states = torch.clamp(hidden_states, input_min, input_max)`   | `(..., in_features)`  | Only if `use_clipped_linears=True` |
| Linear Transformation | `hidden_states = hidden_states @ weight`                             | `(..., out_features)` |                                    |
| Output Clipping       | `hidden_states = torch.clamp(hidden_states, output_min, output_max)` | `(..., out_features)` |                                    |

### **M. Masking (Attention Masks)**

| **Operation**       | **Math Formula**                                                                     | **Shape**                  | **Notes**                                                   |
|---------------------|--------------------------------------------------------------------------------------|----------------------------|-------------------------------------------------------------|
| Causal Mask         | `mask[i, j] = (j <= i)` (for causal attention)                                       | `(S, S)` or `(B, 1, S, S)` | Ensures tokens can only attend to past tokens.              |
| Sliding Window Mask | `mask[i, j] = (0 <= i - j < window_size)` (for sliding window attention)             | `(S, S)` or `(B, 1, S, S)` | Limits attention to a fixed window of past tokens.          |
| Bidirectional Mask  | Custom logic for multimodal inputs (e.g., vision tokens can attend bidirectionally). | `(B, 1, S, S)`             | Used in `create_causal_mask_mapping` for multimodal models. |

### **N. Multidimensional RoPE (Sliding Attention)**

| **Operation**       | **Math Formula**                                                            | **Shape**                  | **Notes**                                |
|---------------------|-----------------------------------------------------------------------------|----------------------------|------------------------------------------|
| Split Inputs        | `x_parts = torch.split(x, split_sizes, dim=-1)`                             | List of `(B, S, dim/ndim)` | `split_sizes = [dim/ndim] * ndim`        |
| Split Cos/Sin       | `cos_parts = torch.split(cos, split_sizes, dim=-1)`, same for `sin`         | List of `(B, S, dim/ndim)` |                                          |
| Apply RoPE per Part | `y_parts[k] = apply_rotary_pos_emb(x_parts[k], cos_parts[k], sin_parts[k])` | `(B, S, dim/ndim)`         | Applies RoPE independently to each part. |
| Concatenate Parts   | `y = torch.cat(y_parts, dim=-1)`                                            | `(B, S, dim)`              | Combines the rotated parts.              |
