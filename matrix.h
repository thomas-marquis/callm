#include "lib/errors.h"
#include <math.h>

struct Matrix
{
    int r;
    int c;
    float *data;
};
typedef struct Matrix matrix_t;

/*
 * Function: new_matrix
 * --------------------
 *
 *  Creates a new matrix with r rows and c columns
 */
matrix_t *Matrix_new(int r, int c);

/**
 * @brief Frees the memory allocated for a matrix object.
 *
 * This function releases all resources associated with a `matrix` object,
 * including the data array and the matrix structure itself.
 *
 * @param M A pointer to the `matrix` object to be freed.
 * @return status_t OK if the free was successful, ERROR otherwise.
 */
status_t Matrix_free(matrix_t *M);

/**
 * @brief Function: fill_matrix
 *
 * Fills the matrix with data
 *
 * @param M: matrix to fill
 * @param data: data to fill the matrix with
 * @return status_t OK if the fill was successful, ERROR otherwise.
 */
status_t Matrix_fill(matrix_t *M, float *data);

/*
 * Function: matmult
 * --------------------
 *
 *  Multiplies two matrices A and B and returns the result matrix C
 */
status_t Matrix_dot(const matrix_t *A, const matrix_t *B, matrix_t *C);

/*
 * Function: print_matrix
 * --------------------
 *
 *  Prints the matrix
 */
void Matrix_print(const matrix_t *M);

/*
 * Function: delete_matrix
 * --------------------
 *
 *  Deletes the matrix and frees the memory
 */
// int delete_matrix(matrix *M);
