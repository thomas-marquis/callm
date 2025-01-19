#ifndef MLP_H
#define MLP_H

#include "../core/matrix.h"

typedef struct mlp_t MLP;

MLP *MLP_new(int input_size, int hidden_size, int output_size);

CallmStatusCode MLP_free(MLP *mlp);

Matrix *MLP_forward(MLP *mlp, Matrix *input);

#endif  // !#ifndef MLP_H
