#ifndef MLP_H
#define MLP_H

#include "../core/config.h"
#include "../core/matrix.h"
#include "../core/safetensors.h"

typedef struct mlp_t MLP;

MLP *MLP_new(Safetensors *st, const Config *config, unsigned int layer_idx);

CallmStatusCode MLP_free(MLP *mlp);

Matrix *MLP_forward(MLP *mlp, Matrix *input);

#endif  // !#ifndef MLP_H
