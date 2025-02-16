#include "decoder.h"
#include "../core/errors.h"
#include "../core/logging.h"
#include "../core/matrix.h"
#include "../core/safetensors.h"
#include "attention.h"
#include "mlp.h"
#include "rms_norm.h"
#include <stddef.h>

struct decoder_t
{
    RMSNorm *input_layernorm;
    Attention *attn;
    MLP *mlp;
    size_t norm_hidden_size;
    float norm_eps;
    Matrix *input_layer_norm;
    Matrix *post_attention_layer_norm;
};

Decoder *
Decoder_new(Safetensors *st, const Config *config, unsigned int layer_idx)
{
    char layer_name[256];

    LOG_DEBUG("Loading decoder...");
    Decoder *decoder = (Decoder *) malloc(sizeof(Decoder));

    decoder->attn = Attention_new(st, config, layer_idx);
    RETURN_WHEN_NULL(decoder->attn, "new attention");

    decoder->mlp = MLP_new(st, config, layer_idx);
    RETURN_WHEN_NULL(decoder->mlp, "new mlp");

    sprintf(layer_name, "model.layers.%d.input_layernorm.weight", layer_idx);
    decoder->input_layernorm = RMSNorm_new(config->rms_norm_eps, st, layer_name);

    sprintf(layer_name, "model.layers.%d.post_attention_layernorm.weight", layer_idx);
    decoder->post_attention_layer_norm = Safetensors_load_matrix(layer_name, st);
    RETURN_WHEN_NULL(decoder->post_attention_layer_norm, "post attention layer norm");
    ENSURE_SHAPE(decoder->post_attention_layer_norm, 2048, 1);

    LOG_DEBUG("Decoder loaded");
    return decoder;
}

CallmStatusCode
Decoder_free(Decoder *decoder)
{
    LOG_DEBUG("Freeing decoder...");
    if (decoder == NULL)
    {
        return ERROR;
    }

    Attention_free(decoder->attn);
    MLP_free(decoder->mlp);

    if (decoder->input_layer_norm != NULL)
    {
        Matrix_free(decoder->input_layer_norm);
    }

    if (decoder->post_attention_layer_norm != NULL)
    {
        Matrix_free(decoder->post_attention_layer_norm);
    }

    free(decoder);
    return OK;
}

Matrix *
Decoder_forward(Decoder *decoder, Matrix *hidden_state)
{
    Matrix *normed_hidden_state = RMSNorm_forward(decoder->input_layernorm, hidden_state);
    ENSURE_SHAPE(normed_hidden_state, hidden_state->r, 2048);

    Matrix *attn_out = Attention_forward(decoder->attn, normed_hidden_state);
    RETURN_WHEN_NULL(attn_out, "Error when running attention");
    Matrix_free(normed_hidden_state);

    Matrix *mlp_out = MLP_forward(decoder->mlp, attn_out);
    RETURN_WHEN_NULL(mlp_out, "Error when running mlp");
    Matrix_free(attn_out);

    // Normaliszation

    return mlp_out;
}
