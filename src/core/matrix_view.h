#ifndef MATRIX_VIEW_H
#define MATRIX_VIEW_H

#include "../shared/errors.h"
#include "matrix.h"

typedef struct matrix_view MatrixView;

MatrixView *MatrixView_new(Matrix *M);

CallmStatusCode MatrixView_select_rows_by_nb(MatrixView *M, int from_row, int nb_rows);

CallmStatusCode MatrixView_select_columns_by_nb(MatrixView *M, int from_col, int nb_cols);

CallmStatusCode MatrixView_select_rows_by_indexes(MatrixView *M, int *rows_idx);

CallmStatusCode MatrixView_select_columns_by_indexes(MatrixView *M, int *cols_idx);

CallmStatusCode MatrixView_free(MatrixView *M);

#endif  // !MATRIX_VIEW_H
