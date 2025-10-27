#ifndef ROTARY_EMBEDDING_H
#define ROTARY_EMBEDDING_H

#include "../core/config.h"
#include "../core/matrix.h"
#include "../shared/errors.h"

typedef struct rotary_embedding_t RotaryEmbedding;

RotaryEmbedding *RotaryEmbedding_new(const Config *config);

CallmStatusCode RotaryEmbedding_free(RotaryEmbedding *re);

CallmStatusCode RotaryEmbedding_forward(RotaryEmbedding *re, Matrix *position_ids, Matrix **out_cos, Matrix **out_sin);

#endif  // !#ifndef ROTARY_EMBEDDING_H
