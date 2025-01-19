#include "mlp.h"
#include "../core/errors.h"
#include "../core/logging.h"
#include "../core/matrix.h"
#include <stdlib.h>

struct mlp_t
{
    Matrix *input_weights;
    Matrix *hidden_weights;
    Matrix *output_weights;
};

MLP *
MLP_new(int input_size, int hidden_size, int output_size)
{
    MLP *mlp = (MLP *) malloc(sizeof(MLP));
    if (mlp == NULL)
    {
        LOG_ERROR("Failed to allocate memory for MLP");
        return NULL;
    }

    mlp->input_weights = Matrix_new(input_size, hidden_size);
    if (mlp->input_weights == NULL)
    {
        LOG_ERROR("Failed to allocate memory for input weights");
        free(mlp);
        return NULL;
    }

    mlp->hidden_weights = Matrix_new(hidden_size, output_size);
    if (mlp->hidden_weights == NULL)
    {
        LOG_ERROR("Failed to allocate memory for hidden weights");
        Matrix_free(mlp->input_weights);
        free(mlp);
        return NULL;
    }

    mlp->output_weights = Matrix_new(output_size, 1);
    if (mlp->output_weights == NULL)
    {
        LOG_ERROR("Failed to allocate memory for output weights");
        Matrix_free(mlp->input_weights);
        Matrix_free(mlp->hidden_weights);
        free(mlp);
        return NULL;
    }

    return mlp;
}

CallmStatusCode
MLP_free(MLP *mlp)
{
    if (mlp == NULL)
    {
        return ERROR;
    }

    Matrix_free(mlp->input_weights);
    Matrix_free(mlp->hidden_weights);
    Matrix_free(mlp->output_weights);
    free(mlp);

    return OK;
}

Matrix *
MLP_forward(MLP *mlp, Matrix *input)
{
    Matrix *hidden = Matrix_dot(mlp->input_weights, input);
    RETURN_WHEN_NULL(hidden, "Failed to compute hidden layer");

    // Matrix_apply_activation(hidden, RELU);
    Matrix *output = Matrix_dot(mlp->hidden_weights, hidden);
    RETURN_WHEN_NULL(output, "Failed to compute output layer");

    Matrix_free(hidden);
    return output;
}
