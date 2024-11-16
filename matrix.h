#include <math.h>

struct matrix
{
    int r;
    int c;
    float *data;
};
typedef struct matrix matrix;

/*
 * Function: new_matrix
 * --------------------
 *
 *  Creates a new matrix with r rows and c columns
 */
matrix *new_matrix(int r, int c);

/**
 * @brief Frees the memory allocated for a matrix object.
 *
 * This function releases all resources associated with a `matrix` object,
 * including the data array and the matrix structure itself.
 *
 * @param M A pointer to the `matrix` object to be freed.
 * @return Returns 0 on success.
 */
int matrix_free(matrix *M);

/*
 * Function: fill_matrix
 * --------------------
 *
 *  Fills the matrix with data
 */
int fill_matrix(matrix *M, float *data);

/*
 * Function: matmult
 * --------------------
 *
 *  Multiplies two matrices A and B and returns the result matrix C
 */
int matmult(const matrix *A, const matrix *B, matrix *C);

/*
 * Function: print_matrix
 * --------------------
 *
 *  Prints the matrix
 */
void matrix_print(const matrix *M);

/*
 * Function: delete_matrix
 * --------------------
 *
 *  Deletes the matrix and frees the memory
 */
// int delete_matrix(matrix *M);
