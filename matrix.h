#include <math.h>

struct matrix
{
    int r;
    int c;
    float_t *data;
};
typedef struct matrix matrix;

/*
 * Function: new_matrix
 * --------------------
 *
 *  Creates a new matrix with r rows and c columns
 */
matrix *new_matrix(int r, int c);

/*
 * Function: fill_matrix
 * --------------------
 *
 *  Fills the matrix with data
 */
int fill_matrix(matrix *M, float_t *data);

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
void print_matrix(const matrix *M);

/*
 * Function: delete_matrix
 * --------------------
 *
 *  Deletes the matrix and frees the memory
 */
// int delete_matrix(matrix *M);
