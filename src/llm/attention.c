#include "attention.h"
#include "../core/matrix.h"
#include "../monitor/probe.h"
#include "../shared/errors.h"
#include "../shared/logging.h"

struct attention
{
    Matrix *query;
    Matrix *key;
    Matrix *value;
    Matrix *out_proj;
    unsigned int layer_idx;
};

/**
Layer: model.layers.0.self_attn.q_proj.weight           shape: [2048, 2048]
Layer: model.layers.0.self_attn.k_proj.weight           shape: [512, 2048]
Layer: model.layers.0.self_attn.v_proj.weight           shape: [512, 2048]
Layer: model.layers.0.self_attn.o_proj.weight           shape: [2048, 2048]
*/

Attention *
Attention_new(Safetensors *st, const Config *config, unsigned int layer_idx)
{
    LOGF_DEBUG("Loading attention layer %d...", layer_idx);
    Attention *at = (Attention *) malloc(sizeof(Attention));

    at->layer_idx = layer_idx;

    char layer_name[256];

    sprintf(layer_name, "model.layers.%d.self_attn.q_proj.weight", layer_idx);
    at->query = Safetensors_load_matrix(layer_name, st);
    ENSURE_SHAPE(at->query, 2048, 2048);

    sprintf(layer_name, "model.layers.%d.self_attn.k_proj.weight", layer_idx);
    at->key = Safetensors_load_matrix(layer_name, st);
    ENSURE_SHAPE(at->key, 512, 2048);

    sprintf(layer_name, "model.layers.%d.self_attn.v_proj.weight", layer_idx);
    at->value = Safetensors_load_matrix(layer_name, st);
    ENSURE_SHAPE(at->value, 512, 2048);

    sprintf(layer_name, "model.layers.%d.self_attn.o_proj.weight", layer_idx);
    at->out_proj = Safetensors_load_matrix(layer_name, st);
    ENSURE_SHAPE(at->out_proj, 2048, 2048);

    LOGF_DEBUG("Attention layer %d loaded", layer_idx);

    return at;
}

void
Attention_free(Attention *at)
{
    if (at != NULL)
    {
        if (at->query != NULL)
            Matrix_free(at->query);
        if (at->key != NULL)
            Matrix_free(at->key);
        if (at->value != NULL)
            Matrix_free(at->value);
        if (at->out_proj != NULL)
            Matrix_free(at->out_proj);
        free(at);
    }
}

/*
 * Apply projection heads to the hidden state
 */
static Matrix **
apply_projection_heads(const Matrix *hidden_state, const Matrix *weights_T, size_t nb_heads, size_t head_dim,
                       const char *probe_label)
{
    int nb_tokens = hidden_state->r;

    Matrix *proj = Matrix_dot(hidden_state, weights_T);
    RETURN_WHEN_NULL(proj, "Failed to compute projection");

    // Probe_send_matrix(proj, probe_label);
    // ENSURE_SHAPE(proj, nb_tokens, 2048);

    Matrix **per_token_mat = malloc(nb_tokens * sizeof(Matrix *));
    for (int i = 0; i < nb_tokens; i++)
    {
        Matrix *tmp = Matrix_slice_line(proj, i, 1);
        Matrix *per_token = Matrix_new(nb_heads, head_dim);  // Equivalent to reshape
        Matrix_fill(per_token, tmp->data);
        per_token_mat[i] = per_token;
        Matrix_free(tmp);
    }
    Matrix_free(proj);

    Matrix **heads = malloc(nb_heads * sizeof(Matrix *));
    for (int i = 0; i < nb_heads; i++)
    {
        Matrix *head = Matrix_new(nb_tokens, head_dim);
        float *head_data = malloc(nb_tokens * head_dim * sizeof(float));
        for (int j = 0; j < nb_tokens; j++)
        {
            Matrix *current_line = Matrix_slice_line(per_token_mat[j], i, 1);
            for (int k = 0; k < head_dim; k++)
                head_data[j * head_dim + k] = current_line->data[k];
            Matrix_free(current_line);
        }
        Matrix_fill(head, head_data);
        free(head_data);
        Matrix_print(head, 0);
        heads[i] = head;
    }

    // cleanup per token temp matrices
    for (int i = 0; i < nb_tokens; i++)
        Matrix_free(per_token_mat[i]);
    free(per_token_mat);

    return heads;
}

void
free_projection_heads(Matrix **heads, size_t nb_heads)
{
    for (int i = 0; i < nb_heads; i++)
        Matrix_free(heads[i]);
    free(heads);
}

void
send_projection_heads(Matrix **heads, size_t nb_heads, const char *label)
{
    char message_label[256];
    for (int i = 0; i < nb_heads; i++)
    {
        sprintf(message_label, "%s_%d", label, i);
        Probe_send_matrix(heads[i], message_label);
    }
}

void
rotate_half(Matrix **xs, size_t nb)
{
    int n;
    int m;
    int x1len;
    int x2len;
    Matrix *x;
    for (int i = 0; i < nb; i++)
    {
        x = xs[i];
        n = x->r;
        m = x->c;
        x1len = m / 2;
        x2len = m - x1len;
        for (int j = 0; j < n; j++)
        {
            float *x1 = malloc(x1len * sizeof(float));
            float *x2 = malloc(x2len * sizeof(float));
            for (int k = 0; k < m; k++)
                if (k < x1len)
                    x1[k] = x->data[j * m + k];
                else
                    x2[k - x1len] = -(x->data[j * m + k]);
            for (int k = 0; k < m; k++)
                if (k < x1len)
                    x->data[j * m + k] = x2[k];
                else
                    x->data[j * m + k] = x1[k - x1len];
            free(x1);
            free(x2);
        }
    }
}

Matrix *
Attention_forward(Attention *at, Matrix *input)
{
    Matrix *query_weights_T = Matrix_transpose(at->query);
    Matrix *key_weights_T = Matrix_transpose(at->key);
    Matrix *value_weights_T = Matrix_transpose(at->value);

    const char label[64];

    LOG_INFO("Compute query heads projections");
    Matrix **query_heads = apply_projection_heads(input, query_weights_T, 32, 64, "query_heads");
    // send_projection_heads(query_heads, 32, "query_heads projected");

    LOG_INFO("Compute key heads projections");
    Matrix **key_heads = apply_projection_heads(input, key_weights_T, 32, 64, "key_heads");
    // send_projection_heads(key_heads, 32, "key_heads projected");

    LOG_INFO("Compute value heads projections");
    Matrix **value_heads = apply_projection_heads(input, value_weights_T, 32, 64, "value_heads");
    // send_projection_heads(value_heads, 32, "value_heads projected");

    Matrix_free(query_weights_T);
    Matrix_free(value_weights_T);

    Matrix **qk_proj = malloc(32 * sizeof(Matrix *));
    char message_label[256];
    for (int i = 0; i < 32; i++)
    {
        Matrix *q_T = Matrix_transpose(query_heads[i]);
        Matrix *kq = Matrix_dot(key_heads[i], q_T);
        RETURN_WHEN_NULL(kq, "Failed to compute query-key projection");
        Matrix_free(q_T);
        qk_proj[i] = kq;
        // Matrix_print(kq, -1);

        sprintf(message_label, "qk_proj_%d", i);
        // Probe_send_matrix(kq, message_label);
    }

    Matrix *key_proj = Matrix_dot(input, key_weights_T);
    RETURN_WHEN_NULL(key_proj, "Failed to compute key projection");
    ENSURE_SHAPE(key_proj, input->r, 512);

    Matrix_free(key_weights_T);

    free_projection_heads(query_heads, 32);
    free_projection_heads(key_heads, 32);
    free_projection_heads(value_heads, 32);

    free_projection_heads(qk_proj, 32);

    Matrix_free(key_proj);

    return NULL;  // TODO: Implement the rest of the forward pass

    // Matrix *attn_mask = Matrix_dot()

    // Matrix *value_proj = Matrix_dot(at->value, hidden_state_T);
    // RETURN_WHEN_NULL(value_proj, "Failed to compute value projection");
    //
    // Matrix *key_proj_T = Matrix_transpose(key_proj);
    // RETURN_WHEN_NULL(key_proj_T, "Failed to transpose key projection");
    //
    // Matrix *scores = Matrix_dot(query_proj, key_proj_T);
    // RETURN_WHEN_NULL(scores, "Failed to compute attention scores");
    //
    // Matrix_apply_along(scores, MAT_APPLY_COL, softmax);
    //
    // Matrix *context = Matrix_dot(scores, value_proj);
    // RETURN_WHEN_NULL(context, "Failed to compute context");
    //
    // Matrix *output = Matrix_dot(at->out_proj, context);
    // RETURN_WHEN_NULL(output, "Failed to compute output");
    //
    //
    //
    // Matrix_free(value_proj);
    // Matrix_free(scores);
    // Matrix_free(context);
    // Matrix_free(key_proj_T);
    //
    // return output;
}
