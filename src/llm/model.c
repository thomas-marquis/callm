#include "model.h"
#include "../core/errors.h"
#include "../core/logging.h"
#include "decoder.h"
#include "embeddings.h"
#include "rotary_embedding.h"
#include <stddef.h>

struct model_t
{
    EmbeddingsLookup *embedding;
    Decoder **decoder_layers;
    size_t decoders_count;
    RotaryEmbedding *rotary;
};

Model *
Model_new(Safetensors *st, const Config *config)
{
    LOG_DEBUG("Loading model...");
    Model *model = (Model *) malloc(sizeof(Model));
    model->embedding = EmbeddingsLookup_new(st);

    model->rotary = RotaryEmbedding_new(config);

    model->decoders_count = config->transformers_bloc_count;
    model->decoder_layers = (Decoder **) malloc(config->transformers_bloc_count * sizeof(Decoder *));
    for (size_t i = 0; i < config->transformers_bloc_count; i++)
    {
        model->decoder_layers[i] = Decoder_new(st, config, i);
    }

    LOG_DEBUG("Model loaded");
    return model;
}

CallmStatusCode
Model_free(Model *model)
{
    LOG_DEBUG("Freeing model...");
    if (model == NULL)
    {
        return OK;
    }
    EmbeddingsLookup_free(model->embedding);

    for (size_t i = 0; i < model->decoders_count; i++)
    {
        Decoder_free(model->decoder_layers[i]);
    }
    free(model->decoder_layers);

    if (model->rotary != NULL)
    {
        RotaryEmbedding_free(model->rotary);
    }

    free(model);
    return OK;
}

Matrix *
Model_forward(Model *model, int *token_ids, int token_count)
{
    Matrix *embeds_in = EmbeddingsLookup_forward(model->embedding, token_ids, token_count);
    RETURN_WHEN_NULL(embeds_in, "Error when embedding input tokens");

    Matrix *hidden_state = embeds_in;
    Matrix *cos = NULL;
    Matrix *sin = NULL;
    Matrix *position_ids = Matrix_arange(0, token_count, 1, MAT_APPLY_ROW);

    // LOGF_DEBUG("Running model with %d tokens position_ids rows=%d, cols=%d", token_count, position_ids->r,
    //            position_ids->c);

    if (RotaryEmbedding_forward(model->rotary, position_ids, &cos, &sin) != OK)
    {
        LOG_ERROR("Error when applying rotary embeddings");
        return NULL;
    }

    for (size_t i = 0; i < model->decoders_count; i++)
    {
        hidden_state = Decoder_forward(model->decoder_layers[i], hidden_state);
        RETURN_WHEN_NULL(hidden_state, "Error when running decoder");
    }

    return hidden_state;
}
