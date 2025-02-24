#include "embeddings.h"
#include "matrix.h"

#define EMBEDDINGS_LAYER_NAME "model.embed_tokens.weight"

struct embeddings_lookup
{
    Matrix *embeddings;
};

/**
Layer: model.embed_tokens.weight                shape: [128256, 2048]
*/
EmbeddingsLookup *
EmbeddingsLookup_new(Safetensors *st)
{
    LOG_DEBUG("Loading embeddings lookup table...");
    EmbeddingsLookup *el = (EmbeddingsLookup *) malloc(sizeof(EmbeddingsLookup));
    Matrix *emb_mat = Safetensors_load_matrix(EMBEDDINGS_LAYER_NAME, st);
    ENSURE_SHAPE(emb_mat, 128256, 2048);

    el->embeddings = emb_mat;
    return el;
    LOG_DEBUG("Embeddings lookup table loaded");
}

void
EmbeddingsLookup_free(EmbeddingsLookup *el)
{
    LOG_DEBUG("Freeing embeddings lookup table...");
    if (el != NULL)
    {
        if (el->embeddings != NULL)
        {
            Matrix_free(el->embeddings);
        }
        free(el);
    }
}

Matrix *
EmbeddingsLookup_forward(EmbeddingsLookup *el, int *token_ids, int token_count)
{
    Matrix *embeddings = Matrix_select_rows(el->embeddings, token_ids, token_count);
    ENSURE_SHAPE(embeddings, token_count, 2048);

    return embeddings;
}
