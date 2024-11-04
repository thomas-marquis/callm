#include "matrix.h"
#include <stdio.h>
#include <stdlib.h>

matrix *new_matrix(int r, int c)
{
    matrix *M = (matrix *)malloc(sizeof(matrix));
    M->r = r;
    M->c = c;
    M->data = (float_t *)malloc(r * c * sizeof(float_t));
    return M;
}

int fill_matrix(matrix *M, float_t *data)
{
    for (int i = 0; i < M->r; i++)
    {
        for (int j = 0; j < M->c; j++)
        {
            M->data[i * M->c + j] = data[i * M->c + j];
        }
    }
    return 0;
}

int matmult(const matrix *A, const matrix *B, matrix *C)
{
    if (A->c != B->r)
    {
        printf("Error: Matrix dimensions do not match\n");
        return 1;
    }

    for (int a_row = 0; a_row < A->r; a_row++)
    {
        for (int b_col = 0; b_col < B->c; b_col++)
        {
            C->data[a_row * C->c + b_col] = 0;
            for (int k = 0; k < A->c; k++)
            {
                C->data[a_row * C->c + b_col] += A->data[a_row * A->c + k] * B->data[k * B->c + b_col];
            }
        }
    }
    return 0;
}

void print_matrix(const matrix *M)
{
    for (int i = 0; i < M->r; i++)
    {
        for (int j = 0; j < M->c; j++)
        {
            printf("%f ", M->data[i * M->c + j]);
        }
        printf("\n");
    }
}
