#include "mlp.h"
#include "../core/matrix.h"
#include "../shared/errors.h"
#include "../shared/logging.h"
#include <stdio.h>
#include <stdlib.h>

struct mlp_t
{
    Matrix *down_weights;
    Matrix *gate_weights;
    Matrix *up_weights;
};

/**
Layer: model.layers.0.mlp.down_proj.weight              shape: [2048, 8192]
Layer: model.layers.0.mlp.gate_proj.weight              shape: [8192, 2048]
Layer: model.layers.0.mlp.up_proj.weight                shape: [8192, 2048]
*/
MLP *
MLP_new(Safetensors *st, const Config *config, unsigned int layer_idx)
{
    int input_size, hidden_size, output_size;  // TODO: load them from config

    MLP *mlp = (MLP *) malloc(sizeof(MLP));
    RETURN_WHEN_NULL(mlp, "Failed to allocate MLP");

    char layer_name[256];

    sprintf(layer_name, "model.layers.%d.mlp.down_proj.weight", layer_idx);
    mlp->down_weights = Safetensors_load_matrix(layer_name, st);
    RETURN_WHEN_NULL(mlp->down_weights, "Failed to load down weights");

    sprintf(layer_name, "model.layers.%d.mlp.gate_proj.weight", layer_idx);
    mlp->gate_weights = Safetensors_load_matrix(layer_name, st);
    RETURN_WHEN_NULL(mlp->gate_weights, "Failed to load gate weights");

    sprintf(layer_name, "model.layers.%d.mlp.up_proj.weight", layer_idx);
    mlp->up_weights = Safetensors_load_matrix(layer_name, st);
    RETURN_WHEN_NULL(mlp->up_weights, "Failed to load up weights");

    return mlp;
}

CallmStatusCode
MLP_free(MLP *mlp)
{
    if (mlp == NULL)
    {
        return ERROR;
    }

    Matrix_free(mlp->down_weights);
    Matrix_free(mlp->gate_weights);
    Matrix_free(mlp->up_weights);
    free(mlp);

    return OK;
}

Matrix *
MLP_forward(MLP *mlp, Matrix *input)
{
    Matrix *hidden = Matrix_dot(mlp->down_weights, input);
    RETURN_WHEN_NULL(hidden, "Failed to compute hidden layer");

    // Matrix_apply_activation(hidden, RELU);
    Matrix *output = Matrix_dot(mlp->gate_weights, hidden);
    RETURN_WHEN_NULL(output, "Failed to compute output layer");

    Matrix_free(hidden);
    return output;
}
