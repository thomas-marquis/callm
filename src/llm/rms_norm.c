#include "rms_norm.h"
#include <stdlib.h>

#include "../core/safetensors.h"
#include "maths.h"
#include "matrix.h"

struct rms_norm_t
{
    float epsilon;
    Matrix *weights;
};

RMSNorm *
RMSNorm_new(float epsilon, const Safetensors *st, const char *layer_name)
{
    RMSNorm *rms_norm = (RMSNorm *) malloc(sizeof(RMSNorm));

    rms_norm->epsilon = epsilon;

    Matrix *tmp = Safetensors_load_matrix(layer_name, st);
    rms_norm->weights = Matrix_transpose(tmp);
    RETURN_WHEN_NULL(rms_norm->weights, "rms norm weights");
    ENSURE_SHAPE(rms_norm->weights, 1, 2048);

    Matrix_free(tmp);

    return rms_norm;
}

CallmStatusCode
RMSNorm_free(RMSNorm *rms_norm)
{
    if (rms_norm == NULL)
    {
        return ERROR;
    }
    if (rms_norm->weights != NULL)
    {
        Matrix_free(rms_norm->weights);
    }
    free(rms_norm);
    return OK;
}

float
mean_square(float *vals, int size)
{
    float mean = 0;
    for (int i = 0; i < size; i++)
    {
        mean += (vals[i] * vals[i]);
    }
    mean /= size;

    return mean;
}

Matrix *
RMSNorm_forward(RMSNorm *rms_norm, Matrix *input)
{
    Matrix *variances = Matrix_reduce_along(input, MAT_APPLY_ROW, mean_square);
    ENSURE_SHAPE(variances, input->r, 1);

    Matrix *variances_eps = Matrix_add_scalar(variances, rms_norm->epsilon);
    ENSURE_SHAPE(variances_eps, input->r, 1);

    Matrix_apply_each(variances_eps, Q_rsqrt);
    ENSURE_SHAPE(variances_eps, input->r, 1);

    Matrix *hidden_state = Matrix_multiply_broadcast(variances_eps, input, MAT_APPLY_ROW);
    ENSURE_SHAPE(hidden_state, input->r, 2048);

    Matrix *result = Matrix_multiply_broadcast(rms_norm->weights, hidden_state, MAT_APPLY_COL);
    ENSURE_SHAPE(result, input->r, 2048);

    Matrix_free(variances);
    Matrix_free(variances_eps);
    Matrix_free(hidden_state);

    return result;
}
