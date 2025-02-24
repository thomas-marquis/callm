#ifndef MATRIX_H
#define MATRIX_H

#include "errors.h"

#define MAT_APPLY_ROW 0
#define MAT_APPLY_COL 1

#if RELEASE_TYPE == DEV
#define ENSURE_SHAPE(M, rows, cols)                                                                                    \
    if (M == NULL || M->data == NULL || M->r != rows || M->c != cols)                                                  \
    {                                                                                                                  \
        LOGF_ERROR("Invalid matrix shape: Expected (%d, %d), got (%d, %d)", rows, cols, M->r, M->c);                   \
        exit(EXIT_FAILURE);                                                                                            \
    }
#else
#define ENSURE_SHAPE(M, rows, cols)
#endif

typedef struct
{
    int r;
    int c;
    size_t size;
    float *data;
} Matrix;

typedef float (*mat_apply_t)(float);
typedef float (*mat_apply_arg_t)(float, void *);
typedef void (*mat_apply_along_t)(float *, int);
typedef float (*mat_reduce_along_t)(float *, int);

Matrix *Matrix_new(int r, int c);

CallmStatusCode Matrix_free(Matrix *M);

CallmStatusCode Matrix_fill(Matrix *M, float *data);

CallmStatusCode Matrix_set(Matrix *M, int row, int col, float value);

float Matrix_get(const Matrix *M, int row, int col, CallmStatusCode *status);

/*
 * Create a new matrix with only one line or columns (depending on the axis) with range values from start to end
 * (excluded) with a step.
 * Example:
 *   Matrix_arange(0, 5, 1, MAT_APPLY_ROW)
 *   => [[0.0, 1.0, 2.0, 3.0, 4.0]] (shape 1 x 5)
 *   Matrix_arange(0, 4, 1, MAT_APPLY_COL)
 *   => [[0.0],
 *       [1.0],
 *       [2.0],
 *       [3.0]] (shape 4 x 1)
 */
Matrix *Matrix_arange(int start, int end, int step, int axis);

/*
 * Given a N x M matrix A and a M x P matrix B, returns the dot product of the two matrices.
 * Output shape is N x P
 */
Matrix *Matrix_dot(const Matrix *A, const Matrix *B);

/*
 * Given an input N x M matrix, applies the given function to matrix's individual elements.
 * Output shape is N x M
 * Returns NULL if both input matrices are not of the same shape
 */
Matrix *Matrix_multiply(const Matrix *A, const Matrix *B);

/*
 * Multiply each rows or columns of a large matrix by the corresponding element of a small matrix.
 * Small matrix must have the same number of rows as the large matrix (depending on the broadcast_direction)
 * and 1 as other dimension.
 * Given a N x M large matrix,
 * - if broadcast_direction == MAT_APPLY_COL, each column of the large matrix is multiplied by the corresponding element
 * - if broadcast_direction == MAT_APPLY_ROW, each row of the large matrix is multiplied by the corresponding element
 * Example:
 * A=[[1],
 *    [2],
 *    [3]]
 *   B=[[1, 1, 1],
 *      [2, 2, 2],
 *      [3, 3, 3]]
 *   => Matrix_multiply_broadcast(A, B, MAT_APPLY_ROW)
 *   => [[1, 1, 1],
 *       [4, 4, 4],
 *       [9, 9, 9]]
 * Output shape is N x M
 */
Matrix *Matrix_multiply_broadcast(const Matrix *A_small, const Matrix *B_large, int broadcast_direction);

/*
 * Given an input N x M matrix, multiplies each element by a scalar.
 * Output shape is N x M
 */
Matrix *Matrix_multiply_scalar(const Matrix *A, float scalar);

/*
 * Given an input N x M matrix, add each element by a scalar.
 * Output shape is N x M
 */
Matrix *Matrix_add_scalar(const Matrix *A, float scalar);

/*
 * Given an input N x M matrix, returns the transpose of the matrix.
 * Output shape is M x N
 */
Matrix *Matrix_transpose(const Matrix *M);

/*
 * Given an input N x M matrix, applies the given function to matrix's individual elements.
 * Output shape is N x M
 */
void Matrix_apply_each(Matrix *M, mat_apply_t f);

/*
 * Given an input N x M matrix, applies the given function to matrix's individual element-wise
 * with an additional argument.
 * Output shape is N x M
 */
void Matrix_apply_each_arg(Matrix *M, mat_apply_arg_t f, void *arg);

/*
 * Given an input N x M matrix, applies the given function to matrix's without changing the shape:
 *
 * - columns if axis == MAT_APPLY_COL => output shape is N x M
 *   Ex:
 *   M=[[1, 2, 3],
 *      [4, 5, 6]]
 *   => apply_along(M, MAT_APPLY_COL, sum)
 *   => M=[[5, 7, 9],
 *         [5, 7, 9]]
 *   => apply_along(M, MAT_APPLY_COL, add_the_max)
 *   => M=[[5, 7, 9],
 *         [8, 10, 12]]
 *
 * - rows if axis == MAT_APPLY_ROW => output shape is N x M
 *   Ex:
 *   M=[[1, 2, 3],
 *      [4, 5, 6]]
 *   => apply_along(M, MAT_APPLY_ROW, sum)
 *   => M=[[6, 6, 6],
 *         [15, 15, 15]]
 *   => apply_along(M, MAT_APPLY_ROW, add_the_max)
 *   => M=[[4, 5, 6],
 *         [10, 11, 12]]
 */
void Matrix_apply_along(Matrix *M, int axis, mat_apply_along_t f);

/*
 * Given an input N x M matrix, applies a function to reduce the matrix along it's:
 * - rows if axis == MAT_APPLY_ROW => output shape is N x 1
 * - columns if axis == MAT_APPLY_COL => output shape is 1 x M
 */
Matrix *Matrix_reduce_along(const Matrix *M, int axis, mat_reduce_along_t f);

/*
 * Simply prints the matrix shape and first elements to the console
 * Specify how many elements to display with display_len argument
 * Display all elements if display_len is -1
 */
void Matrix_print(const Matrix *M, int display_len);

/*
 * Return the json representation of the matrix (array of arrays)
 */
char *Matrix_to_json(const Matrix *M);

/*
 * Given an input N x M matrix, returns a slice of the matrix from the given index to the given number of lines.
 * The nb of line to slice must be less than the number of lines in the matrix plus the starting index
 * Example:
 * M=[[1, 2, 3],
 *    [4, 5, 6],
 *    [7, 8, 9]]
 *   => Matrix_slice_line(M, 0, 2)
 *   => [[1, 2, 3],
 *       [4, 5, 6]]
 * Output shape is nb x M
 */
Matrix *Matrix_slice_line(const Matrix *M, int from, int nb);

/*
 * Given an input N x M matrix, returns a slice of the matrix from the given index to the given number of columns.
 * The nb of columns to slice must be less than the number of columns in the matrix plus the starting index
 * Example:
 * M=[[1, 2, 3],
 *    [4, 5, 6]]
 *   => Matrix_slice_column(M, 0, 2)
 *   => [[1, 2],
 *       [4, 5]]
 * Output shape is N x nb
 */
Matrix *Matrix_slice_column(const Matrix *M, int from, int nb);

/*
 * Compare two matrices element-wise (and compare their shape) and return 1 if they are equal, 0 otherwise
 */
int Matrix_equals(const Matrix *A, Matrix *B);

/*
 *  Given an input N x M matrix, returns a slice of the matrix with the columns specified by their index.
 *  Example:
 *  M=[[1, 2, 3],
 *     [4, 5, 6]]
 *   => Matrix_select_columns(M, (int[]){0, 2}, 2)
 *   => [[1, 3],
 *       [4, 6]]
 *  Output shape is N x nb
 */
Matrix *Matrix_select_columns(const Matrix *M, int *idx, int nb);

/*
 * Given an input N x M matrix, returns a slice of the matrix with the rows specified by their index.
 * Example:
 * M=[[1,  2,  3 ],
 *    [4,  5,  6 ],
 *    [7,  8,  9 ],
 *    [10, 11, 12]]
 *   => Matrix_select_rows(M, (int[]){0, 2}, 2)
 *   => [[1, 2, 3],
 *       [7, 8, 9]]
 *    Output shape is nb x M
 */
Matrix *Matrix_select_rows(const Matrix *M, int *idx, int nb);

/*
 * Concatenate two matrices along a given axis.
 * Example:
 *   A=[[1, 2],
 *      [3, 4]]
 *   B=[[5, 6],
 *      [7, 8]]
 *   => Matrix_concat(A, B, MAT_APPLY_ROW)
 *   => [[1, 2],
 *       [3, 4],
 *       [5, 6],
 *       [7, 8]]
 *    Output shape is 4 x 2
 *    => Matrix_concat(A, B, MAT_APPLY_COL)
 *    => [[1, 2, 5, 6],
 *        [3, 4, 7, 8]]
 *    Output shape is 2 x 4
 */
Matrix *Matrix_concat(const Matrix *A, const Matrix *B, int axis);

#endif  // !#ifndef MATRIX_H
