#include "attention.h"
#include "../core/errors.h"
#include "../core/logging.h"

struct attention
{
    Matrix *query;
    Matrix *key;
    Matrix *value;
    Matrix *out_proj;
};

/**
Layer: model.layers.0.input_layernorm.weight            shape: [2048]
Layer: model.layers.0.mlp.down_proj.weight              shape: [2048, 8192]
Layer: model.layers.0.mlp.gate_proj.weight              shape: [8192, 2048]
Layer: model.layers.0.mlp.up_proj.weight                shape: [8192, 2048]
Layer: model.layers.0.post_attention_layernorm.weight           shape: [2048]
Layer: model.layers.0.self_attn.k_proj.weight           shape: [512, 2048]
Layer: model.layers.0.self_attn.o_proj.weight           shape: [2048, 2048]
Layer: model.layers.0.self_attn.q_proj.weight           shape: [2048, 2048]
Layer: model.layers.0.self_attn.v_proj.weight           shape: [512, 2048]
*/

Attention *
Attention_new(Safetensors *st, int layer, const Config *config)
{
    LOGF_DEBUG("Loading attention layer %d...", layer);
    Attention *at = (Attention *) malloc(sizeof(Attention));

    char layer_name[256];

    sprintf(layer_name, "model.layers.%d.self_attn.q_proj.weight", layer);
    at->query = Safetensors_load_matrix(layer_name, st);

    sprintf(layer_name, "model.layers.%d.self_attn.k_proj.weight", layer);
    at->key = Safetensors_load_matrix(layer_name, st);

    sprintf(layer_name, "model.layers.%d.self_attn.v_proj.weight", layer);
    at->value = Safetensors_load_matrix(layer_name, st);

    sprintf(layer_name, "model.layers.%d.self_attn.o_proj.weight", layer);
    at->out_proj = Safetensors_load_matrix(layer_name, st);

    LOGF_DEBUG("Attention layer %d loaded", layer);

    return at;
}

void
Attention_free(Attention *at)
{
    if (at != NULL)
    {
        if (at->query != NULL)
        {
            Matrix_free(at->query);
        }
        if (at->key != NULL)
        {
            Matrix_free(at->key);
        }
        if (at->value != NULL)
        {
            Matrix_free(at->value);
        }
        if (at->out_proj != NULL)
        {
            Matrix_free(at->out_proj);
        }
        free(at);
    }
}

Matrix *
Attention_forward(Attention *at, Matrix *input)
{
    Matrix *query_proj = Matrix_dot(at->query, input);
    RETURN_WHEN_NULL(query_proj, "Failed to compute query projection");

    Matrix *key_proj = Matrix_dot(at->key, input);
    RETURN_WHEN_NULL(key_proj, "Failed to compute key projection");

    Matrix *value_proj = Matrix_dot(at->value, input);
    RETURN_WHEN_NULL(value_proj, "Failed to compute value projection");

    Matrix *key_proj_T = Matrix_transpose(key_proj);
    RETURN_WHEN_NULL(key_proj_T, "Failed to transpose key projection");

    Matrix *scores = Matrix_dot(query_proj, key_proj_T);
    RETURN_WHEN_NULL(scores, "Failed to compute attention scores");

    Matrix_apply_softmax(scores);

    Matrix *context = Matrix_dot(scores, value_proj);
    RETURN_WHEN_NULL(context, "Failed to compute context");

    Matrix *output = Matrix_dot(at->out_proj, context);
    RETURN_WHEN_NULL(output, "Failed to compute output");

    Matrix_free(query_proj);
    Matrix_free(key_proj);
    Matrix_free(value_proj);
    Matrix_free(scores);
    Matrix_free(context);
    Matrix_free(key_proj_T);

    return output;
}
