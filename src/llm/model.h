#ifndef MODEL_H
#define MODEL_H

#include "../core/config.h"
#include "../core/errors.h"
#include "../core/matrix.h"
#include "../core/safetensors.h"

typedef struct model_t Model;

Model *Model_new(Safetensors *st, const Config *config);

CallmStatusCode Model_free(Model *model);

Matrix *Model_forward(Model *model, int *token_ids, int token_count);

#endif  // !#ifndef MODEL_H
