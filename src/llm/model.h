#ifndef MODEL_H
#define MODEL_H

#include "../core/config.h"
#include "../core/matrix.h"
#include "../core/safetensors.h"
#include "../shared/errors.h"

typedef struct model_t Model;

Model *Model_new(Safetensors *st, const Config *config);

CallmStatusCode Model_free(Model *model);

Matrix *Model_forward(Model *model, int *token_ids, int token_count);

Matrix *Model_embed_inputs(Model *model, int *token_ids, int token_count, Matrix **cos, Matrix **sin);

#endif  // !#ifndef MODEL_H
