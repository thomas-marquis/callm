#ifndef EMBEDDINGS_H
#define EMBEDDINGS_H

#include "../core/safetensors.h"

typedef struct embeddings_lookup EmbeddingsLookup;

EmbeddingsLookup *EmbeddingsLookup_new(Safetensors *st);

void EmbeddingsLookup_free(EmbeddingsLookup *el);

Matrix *EmbeddingsLookup_forward(EmbeddingsLookup *el, int *token_ids, int token_count);

#endif  // !#ifndef EMBEDDINGS_H
