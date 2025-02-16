#include "errors.h"
#include "logging.h"
#include <stdio.h>
#include <stdlib.h>

#define TRANSPOSED 1
#define NOT_TRANSPOSED 0

typedef struct
{
    size_t *shape;
    size_t ndim;
    size_t size;
    float *data;
} Tensor;

typedef struct
{
    Tensor *T;
    char is_transposed;
    size_t transpose_axes[2];
} TensorView;

Tensor *
Tensor_new(size_t ndim, size_t *shape)
{
    Tensor *T = (Tensor *) malloc(sizeof(Tensor));
    T->ndim = ndim;
    T->shape = malloc(ndim * sizeof(size_t));
    size_t size = 1;
    for (size_t i = 0; i < ndim; i++)
    {
        T->shape[i] = shape[i];
        size *= shape[i];
    }
    T->size = size;
    T->data = (float *) malloc(size * sizeof(float));
    return T;
}

TensorView *
TensorView_from_tensor(Tensor *T)
{
    if (T == NULL)
    {
        LOG_ERROR("Input tensor must not be NULL");
        return NULL;
    }
    TensorView *tv = (TensorView *) malloc(sizeof(TensorView));
    tv->T = T;
    tv->is_transposed = NOT_TRANSPOSED;
    tv->transpose_axes[0] = 0;
    tv->transpose_axes[1] = 0;
    return tv;
}

void
Tensor_free(Tensor *T)
{
    if (T == NULL)
    {
        return;
    }
    free(T->shape);
    free(T->data);
    free(T);
}

void
TensorView_free(TensorView *tv)
{
    if (tv == NULL)
    {
        return;
    }
    free(tv);
}

void
Tensor_fill(Tensor *T, float *data)
{
    for (size_t i = 0; i < T->size; i++)
    {
        T->data[i] = data[i];
    }
}

CallmStatusCode
Tensor_dot(const Tensor *A, const Tensor *B, Tensor *out)
{
    if (A == NULL || B == NULL || out == NULL)
    {
        LOG_ERROR("Input and output tensors must not be NULL");
        return ERROR;
    }
    if (A->ndim != 2 || B->ndim != 2)
    {
        LOG_ERROR("Only 2D tensors are supported for matrix multiplication");
        return ERROR;
    }
    if (out->ndim != 2)
    {
        LOG_ERROR("Output tensor must be 2D");
        return ERROR;
    }
    if (A->shape[A->ndim - 1] != B->shape[B->ndim - 2])
    {
        LOGF_ERROR("Tensor dimensions do not match: %zu != %zu", A->shape[A->ndim - 1], B->shape[B->ndim - 2]);
        return ERROR;
    }
    if (out->shape[0] != A->shape[0] || out->shape[1] != B->shape[1])
    {
        LOGF_ERROR("Output tensor dimensions do not match: %zu != %zu, %zu != %zu", out->shape[0], A->shape[0],
                   out->shape[1], B->shape[1]);
        return ERROR;
    }

    size_t shape[2] = { A->shape[0], B->shape[1] };
    for (size_t i = 0; i < shape[0]; i++)
    {
        for (size_t j = 0; j < shape[1]; j++)
        {
            out->data[i * shape[1] + j] = 0;
            for (size_t k = 0; k < A->shape[1]; k++)
            {
                out->data[i * shape[1] + j] += A->data[i * shape[1] + k] * B->data[k * shape[1] + j];
            }
        }
    }
    return OK;
}

// static CallmStatusCode
// _Tensor_dot_transposed(const Tensor *A, const Tensor *B, char is_A_transposed, char is_B_transposed, Tensor *out)
// {
//     if (is_A_transposed == NOT_TRANSPOSED && is_B_transposed == NOT_TRANSPOSED)
//     {
//         return Tensor_dot(A, B, out);
//     }
//
//     if (A == NULL || B == NULL || out == NULL)
//     {
//         LOG_ERROR("Input and output tensors must not be NULL");
//         return ERROR;
//     }
//     if (A->ndim != 2 || B->ndim != 2)
//     {
//         LOG_ERROR("Only 2D tensors are supported for matrix multiplication");
//         return ERROR;
//     }
//     if (out->ndim != 2)
//     {
//         LOG_ERROR("Output tensor must be 2D");
//         return ERROR;
//     }
//
//     return OK;
// }

CallmStatusCode
TensorView_dot(const TensorView *tv1, const TensorView *tv2, Tensor *out)
{
    return Tensor_dot(tv1->T, tv2->T, out);
}

CallmStatusCode
Tensor_multiply(const Tensor *A, const Tensor *B, Tensor *out)
{
    if (A == NULL || B == NULL || out == NULL)
    {
        LOG_ERROR("Input and output tensors must not be NULL");
        return ERROR;
    }
    if (out->ndim != A->ndim || out->ndim != B->ndim)
    {
        LOG_ERROR("Output tensor must have the same number of dimensions as the input tensors");
        return ERROR;
    }
    for (size_t i = 0; i < A->ndim; i++)
    {
        if (A->shape[i] != B->shape[i])
        {
            LOGF_ERROR("Tensor dimensions of input tensors do not match: %zu != %zu", A->shape[i], B->shape[i]);
            return ERROR;
        }
        if (A->shape[i] != out->shape[i])
        {
            LOGF_ERROR("Tensor dimensions of input and output tensors do not match: %zu != %zu", A->shape[i],
                       out->shape[i]);
            return ERROR;
        }
    }

    for (int i = 0; i < A->size; i++)
    {
        out->data[i] = A->data[i] * B->data[i];
    }

    return OK;
}

CallmStatusCode
Tensor_multiply_scalar(const Tensor *A, float scalar, Tensor *out)
{
    if (A == NULL || out == NULL)
    {
        LOG_ERROR("Input and output tensors must not be NULL");
        return ERROR;
    }
    if (out->ndim != A->ndim)
    {
        LOG_ERROR("Output tensor must have the same number of dimensions as the input tensor");
        return ERROR;
    }
    for (size_t i = 0; i < A->ndim; i++)
    {
        if (A->shape[i] != out->shape[i])
        {
            LOGF_ERROR("Tensor dimensions of input and output tensors do not match: %zu != %zu", A->shape[i],
                       out->shape[i]);
            return ERROR;
        }
    }

    for (int i = 0; i < A->size; i++)
    {
        out->data[i] = A->data[i] * scalar;
    }
    return OK;
}

CallmStatusCode
Tensor_add_scalar(const Tensor *A, float scalar, Tensor *out)
{
    if (A == NULL || out == NULL)
    {
        LOG_ERROR("Input and output tensors must not be NULL");
        return ERROR;
    }
    if (out->ndim != A->ndim)
    {
        LOG_ERROR("Output tensor must have the same number of dimensions as the input tensor");
        return ERROR;
    }
    for (size_t i = 0; i < A->ndim; i++)
    {
        if (A->shape[i] != out->shape[i])
        {
            LOGF_ERROR("Tensor dimensions of input and output tensors do not match: %zu != %zu", A->shape[i],
                       out->shape[i]);
            return ERROR;
        }
    }

    for (int i = 0; i < A->size; i++)
    {
        out->data[i] = A->data[i] + scalar;
    }
    return OK;
}

void
Tensor_print(const Tensor *T)
{
    int size = 1;
    for (int i = 0; i < T->ndim; i++)
    {
        size *= T->shape[i];
    }
    printf("Tensor(");
    for (int i = 0; i < T->ndim; i++)
    {
        printf("%zu", T->shape[i]);
        if (i < T->ndim - 1)
        {
            printf(", ");
        }
    }
    printf(")=\n");
    for (int i = 0; i < size; i++)
    {
        printf("%f ", T->data[i]);
        if ((i + 1) % T->shape[T->ndim - 1] == 0)
        {
            printf("\n");
        }
    }
    printf("\n");
}

int
Tensor_equals(const Tensor *A, const Tensor *B)
{
    if (A->ndim != B->ndim)
    {
        return 0;
    }
    for (int i = 0; i < A->ndim; i++)
    {
        if (A->shape[i] != B->shape[i])
        {
            return 0;
        }
    }
    int size = 1;
    for (int i = 0; i < A->ndim; i++)
    {
        size *= A->shape[i];
    }
    for (int i = 0; i < size; i++)
    {
        if (A->data[i] != B->data[i])
        {
            return 0;
        }
    }
    return 1;
}
