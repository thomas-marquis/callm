#ifndef MATRIX_H
#define MATRIX_H

#include "../utils/errors.h"
#include <math.h>

typedef struct
{
    int r;
    int c;
    float *data;
} Matrix;

/*
 * Function: new_matrix
 * --------------------
 *
 *  Creates a new matrix with r rows and c columns
 */
Matrix *Matrix_new(int r, int c);

/**
 * @brief Frees the memory allocated for a matrix object.
 *
 * This function releases all resources associated with a `matrix` object,
 * including the data array and the matrix structure itself.
 *
 * @param M A pointer to the `matrix` object to be freed.
 * @return status_t OK if the free was successful, ERROR otherwise.
 */
CallmStatusCode Matrix_free(Matrix *M);

/**
 * @brief Function: fill_matrix
 *
 * Fills the matrix with data
 *
 * @param M: matrix to fill
 * @param data: data to fill the matrix with
 * @return status_t OK if the fill was successful, ERROR otherwise.
 */
CallmStatusCode Matrix_fill(Matrix *M, float *data);

/*
 * Function: matmult
 * --------------------
 *
 *  Multiplies two matrices A and B and returns the result matrix C
 */
CallmStatusCode Matrix_dot(const Matrix *A, const Matrix *B, Matrix *C);

/*
 * Function: print_matrix
 * --------------------
 *
 *  Prints the matrix
 */
void Matrix_print(const Matrix *M);

Matrix *Matrix_slice_line(const Matrix *M, int from, int nb);

Matrix *Matrix_slice_column(const Matrix *M, int from, int nb);

/*
 * Function: delete_matrix
 * --------------------
 *
 *  Deletes the matrix and frees the memory
 */
// int delete_matrix(matrix *M);
#endif  // MATRIX_H
