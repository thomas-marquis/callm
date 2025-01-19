#include "model.h"
#include "../core/errors.h"
#include "../core/logging.h"
#include "attention.h"
#include "embeddings.h"
#include <stddef.h>

struct model_t
{
    EmbeddingsLookup *embedding;
    Attention **attention_layers;
    size_t attention_layers_nb;
};

Model *
Model_new(Safetensors *st, const Config *config)
{
    LOG_DEBUG("Loading model...");
    Model *model = (Model *) malloc(sizeof(Model));
    model->embedding = EmbeddingsLookup_new(st);

    model->attention_layers = (Attention **) malloc(config->transformers_bloc_count * sizeof(Attention *));

    for (size_t i = 0; i < config->transformers_bloc_count; i++)
    {
        model->attention_layers[i] = Attention_new(st, i, config);
    }
    model->attention_layers_nb = config->transformers_bloc_count;

    LOG_DEBUG("Model loaded");
    return model;
}

CallmStatusCode
Model_free(Model *model)
{
    LOG_DEBUG("Freeing model...");
    if (model == NULL)
    {
        return ERROR;
    }
    EmbeddingsLookup_free(model->embedding);

    for (size_t i = 0; i < model->attention_layers_nb; i++)
    {
        Attention_free(model->attention_layers[i]);
    }

    free(model->attention_layers);

    free(model);
    return OK;
}

Matrix *
Model_forward(Model *model, int *token_ids, int token_count)
{
    Matrix *embeds_in = EmbeddingsLookup_forward(model->embedding, token_ids, token_count);
    RETURN_WHEN_NULL(embeds_in, "Error when embedding input tokens");

    return NULL;
}
