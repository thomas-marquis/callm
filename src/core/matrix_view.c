#include "matrix_view.h"
#include "errors.h"
#include "matrix.h"

struct matrix_view
{
    Matrix *M;
};

MatrixView *
MatrixView_new(Matrix *M)
{
    return NULL;
}

CallmStatusCode
MatrixView_select_rows_by_nb(MatrixView *M, int from_row, int nb_rows)
{
    return NOT_IMPLEMENTED;
}

CallmStatusCode
MatrixView_select_columns_by_nb(MatrixView *M, int from_col, int nb_cols)
{
    return NOT_IMPLEMENTED;
}

CallmStatusCode
MatrixView_select_rows_by_indexes(MatrixView *M, int *rows_idx)
{
    return NOT_IMPLEMENTED;
}

CallmStatusCode
MatrixView_select_columns_by_indexes(MatrixView *M, int *cols_idx)
{
    return NOT_IMPLEMENTED;
}

CallmStatusCode
MatrixView_free(MatrixView *M)
{
    return NOT_IMPLEMENTED;
}
