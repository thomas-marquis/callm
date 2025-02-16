#include "rotary_embedding.h"
#include "matrix.h"
#include <math.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

struct rotary_embedding_t
{
    Matrix *inv_freg;
    float attn_scaling;
};

static void
compute_default_rope_parameters(const Config *config, Matrix **out_inv_freq, float *attn_factor)
{

    float base = config->rope_theta;
    float partial_rotary_factor = 1.0;
    int dim = (int) config->head_dim * partial_rotary_factor;

    Matrix *inv_freq = Matrix_arange(0, dim, 2, MAT_APPLY_ROW);

    for (size_t i = 0; i < inv_freq->size; i++)
    {
        inv_freq->data[i] = 1.0 / pow(base, (float) inv_freq->data[i] / dim);
    }

    *out_inv_freq = inv_freq;
    *attn_factor = 1.0;
}

static void
compute_llama3_parameters(const Config *config, Matrix **out_inv_freq, float *out_attn_scaling)
{
    Matrix *inv_freq = NULL;
    compute_default_rope_parameters(config, &inv_freq, out_attn_scaling);

    float factor = config->rope_scaling_factor;
    float low_freq_factor = config->rope_scaling_low_freq_factor;
    float high_freq_factor = config->rope_scaling_high_freq_factor;
    int old_context_len = config->rope_scaling_original_max_position_embeddings;

    float low_freq_wavelen = old_context_len / low_freq_factor;
    float high_freq_wavelen = old_context_len / high_freq_factor;

    Matrix *inv_freq_llama = Matrix_new(inv_freq->c, 1);
    for (size_t i = 0; i < inv_freq->size; i++)
    {
        float wavelen = 2 * M_PI / inv_freq->data[i];
        if (wavelen > low_freq_wavelen)
        {
            inv_freq_llama->data[i] = inv_freq->data[i] / factor;
        }
        else if (wavelen < high_freq_wavelen)
        {
            inv_freq_llama->data[i] = inv_freq->data[i];
        }
        else
        {
            float smooth_factor = (old_context_len / wavelen - low_freq_factor) / (high_freq_factor - low_freq_factor);
            inv_freq_llama->data[i]
                = (1 - smooth_factor) * inv_freq->data[i] / factor + smooth_factor * inv_freq->data[i];
        }
    }

    Matrix_free(inv_freq);
    *out_inv_freq = inv_freq_llama;
}

RotaryEmbedding *
RotaryEmbedding_new(const Config *config)
{
    RotaryEmbedding *re = malloc(sizeof(RotaryEmbedding));
    Matrix *inv_freq = NULL;
    float attn_scaling;
    compute_llama3_parameters(config, &inv_freq, &attn_scaling);

    re->attn_scaling = attn_scaling;
    re->inv_freg = inv_freq;

    return re;
}

CallmStatusCode
RotaryEmbedding_free(RotaryEmbedding *re)
{
    if (re == NULL)
    {
        return OK;
    }
    free(re);
    return OK;
}

static float
cos_f_and_scale(float x, void *arg)
{
    return cos(x) * (*(float *) arg);
}

static float
sin_f_and_scale(float x, void *arg)
{
    return sin(x) * (*(float *) arg);
}

CallmStatusCode
RotaryEmbedding_forward(RotaryEmbedding *re, Matrix *position_ids, Matrix **out_cos, Matrix **out_sin)
{
    Matrix *inv_freq_expanded = re->inv_freg;
    ENSURE_SHAPE(inv_freq_expanded, 32, 1);

    Matrix *position_ids_expanded = position_ids;
    ENSURE_SHAPE(position_ids_expanded, 1, position_ids->c);

    Matrix *freqs = Matrix_dot(inv_freq_expanded, position_ids_expanded);
    ENSURE_SHAPE(freqs, 32, position_ids->c);

    // Matrix_free(inv_freq_expanded);
    // Matrix_free(position_ids_expanded);

    Matrix *embs = Matrix_concat(freqs, freqs, MAT_APPLY_COL);
    Matrix_free(freqs);

    *out_cos = Matrix_new(embs->r, embs->c);
    Matrix_apply_each_arg(*out_cos, cos_f_and_scale, &re->attn_scaling);

    *out_sin = Matrix_new(embs->r, embs->c);
    Matrix_apply_each_arg(*out_sin, sin_f_and_scale, &re->attn_scaling);

    return OK;
}
