#include "matrix.h"
#include "logging.h"
#include <immintrin.h>
#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>

#define BLOCK_SIZE 16

Matrix *
Matrix_new(int r, int c)
{
    Matrix *M = (Matrix *) malloc(sizeof(Matrix));
    M->r = r;
    M->c = c;
    M->size = r * c;
    M->data = (float *) malloc(r * c * sizeof(float));
    return M;
}

CallmStatusCode
Matrix_free(Matrix *M)
{
    if (M == NULL)
    {
        return OK;
    }
    free(M->data);
    free(M);
    return OK;
}

CallmStatusCode
Matrix_set(Matrix *M, int row, int col, float value)
{
    if (row >= M->r || col >= M->c)
    {
        return ERROR;
    }
    M->data[row * M->c + col] = value;
    return OK;
}

float
Matrix_get(const Matrix *M, int row, int col, CallmStatusCode *status)
{
    if (row >= M->r || col >= M->c)
    {
        *status = ERROR;
        return 0;
    }
    if (status != NULL)
    {
        *status = OK;
    }
    return M->data[row * M->c + col];
}

CallmStatusCode
Matrix_fill(Matrix *M, float *data)
{
    for (int i = 0; i < M->r; i++)
    {
        for (int j = 0; j < M->c; j++)
        {
            M->data[i * M->c + j] = data[i * M->c + j];
        }
    }
    return OK;
}

Matrix *
Matrix_arange(int start, int end, int step, int axis)
{
    int nb = (end - start) / step;
    Matrix *M;
    if (axis == MAT_APPLY_ROW)
    {
        M = Matrix_new(1, nb);
    }
    else if (axis == MAT_APPLY_COL)
    {
        M = Matrix_new(nb, 1);
    }
    else
    {
        LOG_ERROR("Invalid axis");
        return NULL;
    }

    for (int i = 0; i < nb; i++)
    {
        M->data[i] = start + i * step;
    }
    return M;
}

Matrix *
Matrix_dot(const Matrix *A, const Matrix *B)
{
    Matrix *C = Matrix_new(A->r, B->c);
    if (A->c != B->r)
    {
        LOG_ERROR("Matrix dimensions do not match");
        return NULL;
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
    return C;
}

Matrix *
Matrix_multiply(const Matrix *A, const Matrix *B)
{
    if ((A->r != B->r) || (A->c != B->c))
    {
        LOG_ERROR("Matrix dimensions do not match");
        return NULL;
    }

    Matrix *C = Matrix_new(A->r, A->c);
    for (int i = 0; i < C->r; i++)
    {
        for (int j = 0; j < C->c; j++)
        {
            C->data[i * C->c + j] = A->data[i * A->c + j] * B->data[i * B->c + j];
        }
    }
    return C;
}

Matrix *
Matrix_multiply_broadcast(const Matrix *A_small, const Matrix *B_large, int broadcast_direction)
{
    if (broadcast_direction == MAT_APPLY_COL)
    {
        if (A_small->c != B_large->c)
        {
            LOGF_ERROR("Large matrix nb of columns do not match: expected %d, got %d", A_small->c, B_large->c);
            return NULL;
        }
        Matrix *C = Matrix_new(B_large->r, B_large->c);
        for (int i = 0; i < C->c; i++)
        {
            for (int j = 0; j < C->r; j++)
            {
                C->data[j * C->c + i] = A_small->data[i] * B_large->data[j * B_large->c + i];
            }
        }
        return C;
    }
    else if (broadcast_direction == MAT_APPLY_ROW)
    {
        if (A_small->r != B_large->r)
        {
            LOGF_ERROR("Large matrix nb of rows do not match: expected %d, got %d", A_small->r, B_large->r);
            return NULL;
        }
        Matrix *C = Matrix_new(B_large->r, B_large->c);
        for (int i = 0; i < C->r; i++)
        {
            for (int j = 0; j < C->c; j++)
            {
                C->data[i * C->c + j] = A_small->data[i] * B_large->data[i * B_large->c + j];
            }
        }
        return C;
    }
    else
    {
        LOG_ERROR("Invalid broadcast direction");
        return NULL;
    }
}

Matrix *
Matrix_multiply_scalar(const Matrix *A, float scalar)
{
    Matrix *C = Matrix_new(A->r, A->c);
    for (int i = 0; i < C->r; i++)
    {
        for (int j = 0; j < C->c; j++)
        {
            C->data[i * C->c + j] = A->data[i * A->c + j] * scalar;
        }
    }
    return C;
}

Matrix *
Matrix_add_scalar(const Matrix *A, float scalar)
{
    Matrix *C = Matrix_new(A->r, A->c);
    for (int i = 0; i < C->r; i++)
    {
        for (int j = 0; j < C->c; j++)
        {
            C->data[i * C->c + j] = A->data[i * A->c + j] + scalar;
        }
    }
    return C;
}

void
Matrix_apply_each(Matrix *M, mat_apply_t f)
{
    for (int i = 0; i < M->r; i++)
    {
        for (int j = 0; j < M->c; j++)
        {
            M->data[i * M->c + j] = f(M->data[i * M->c + j]);
        }
    }
}

void
Matrix_apply_each_arg(Matrix *M, mat_apply_arg_t f, void *arg)
{
    for (int i = 0; i < M->r; i++)
    {
        for (int j = 0; j < M->c; j++)
        {
            M->data[i * M->c + j] = f(M->data[i * M->c + j], arg);
        }
    }
}

void
Matrix_apply_along(Matrix *M, int axis, mat_apply_along_t f)
{
    if (axis == MAT_APPLY_COL)
    {
        for (int j = 0; j < M->c; j++)
        {
            float col[M->r];
            for (int i = 0; i < M->r; i++)
            {
                col[i] = M->data[i * M->c + j];
            }
            f(col, M->r);
            for (int i = 0; i < M->r; i++)
            {
                M->data[i * M->c + j] = col[i];
            }
        }
    }
    else if (axis == MAT_APPLY_ROW)
    {

        for (int i = 0; i < M->r; i++)
        {
            f(&M->data[i * M->c], M->c);
        }
    }
    else
    {
        LOG_ERROR("Invalid axis");
    }
}

Matrix *
Matrix_reduce_along(const Matrix *M, int axis, mat_reduce_along_t f)
{
    Matrix *result;
    if (axis == MAT_APPLY_COL)
    {
        result = Matrix_new(1, M->c);
        for (int j = 0; j < M->c; j++)
        {
            float col[M->r];
            for (int i = 0; i < M->r; i++)
            {
                col[i] = M->data[i * M->c + j];
            }
            result->data[j] = f(col, M->r);
        }
        ENSURE_SHAPE(result, 1, M->c);
    }
    else if (axis == MAT_APPLY_ROW)
    {
        result = Matrix_new(M->r, 1);
        for (int i = 0; i < M->r; i++)
        {
            result->data[i] = f(&M->data[i * M->c], M->c);
        }

        ENSURE_SHAPE(result, M->r, 1);
    }
    else
    {
        LOG_ERROR("Invalid axis");
        return NULL;
    }
    return result;
}

Matrix *
Matrix_transpose(const Matrix *M)
{
    if (M == NULL || M->data == NULL)
    {
        return NULL;
    }

    Matrix *result = Matrix_new(M->c, M->r);
    for (int i = 0; i < M->r; i++)
    {
        for (int j = 0; j < M->c; j++)
        {
            result->data[j * M->r + i] = M->data[i * M->c + j];
        }
    }
    return result;
}

Matrix *
Matrix_slice_line(const Matrix *M, int from, int nb)
{
    if (from + nb > M->r)
    {
        LOG_ERROR("Error: slice out of bounds");
        return NULL;
    }
    Matrix *M_slice = Matrix_new(nb, M->c);
    for (int i = 0; i < nb; i++)
    {
        for (int j = 0; j < M->c; j++)
        {
            M_slice->data[i * M->c + j] = M->data[(from + i) * M->c + j];
        }
    }
    return M_slice;
}

Matrix *
Matrix_slice_column(const Matrix *M, int from, int nb)
{
    if (from + nb > M->c)
    {
        LOG_ERROR("Error: slice out of bounds");
        return NULL;
    }
    Matrix *M_slice = Matrix_new(M->r, nb);
    for (int i = 0; i < M->r; i++)
    {
        for (int j = 0; j < nb; j++)
        {
            M_slice->data[i * nb + j] = M->data[i * M->c + from + j];
        }
    }
    return M_slice;
}

Matrix *
Matrix_select_columns(const Matrix *M, int *idx, int nb)
{
    Matrix *M_slice = Matrix_new(M->r, nb);
    int i;
    int curr_idx;
    for (i = 0; i < nb; i++)
    {
        curr_idx = idx[i];
        for (int j = 0; j < M->r; j++)
        {
            M_slice->data[j * nb + i] = M->data[j * M->c + curr_idx];
        }
    }
    return M_slice;
}

Matrix *
Matrix_select_rows(const Matrix *M, int *idx, int nb)
{
    Matrix *M_slice = Matrix_new(nb, M->c);
    int i;
    int curr_idx;
    for (i = 0; i < nb; i++)
    {
        curr_idx = idx[i];
        for (int j = 0; j < M->c; j++)
        {
            M_slice->data[i * M->c + j] = M->data[curr_idx * M->c + j];
        }
    }
    return M_slice;
}

void
Matrix_print(const Matrix *M, int display_len)
{
    int nb_lines;
    if (display_len == -1 || display_len > M->r)
    {
        nb_lines = M->r;
    }
    else
    {
        nb_lines = display_len;
    }
    int nb_cols;
    if (display_len == -1 || display_len > M->c)
    {
        nb_cols = M->c;
    }
    else
    {
        nb_cols = display_len;
    }

    printf("M(%dx%d)=\n", M->r, M->c);
    for (int i = 0; i < nb_lines; i++)
    {
        for (int j = 0; j < nb_cols; j++)
        {
            printf("%f ", M->data[i * M->c + j]);
        }
        if (nb_cols < M->c)
        {
            printf("...");
        }
        printf("\n");
    }
    if (nb_lines < M->r)
    {
        printf("...\n");
    }
    printf("\n");
}

char *
Matrix_to_json(const Matrix *M)
{
    if (M == NULL)
    {
        return NULL;
    }

    json_t *json_matrix = json_array();
    if (!json_matrix)
    {
        return NULL;
    }

    for (int i = 0; i < M->r; i++)
    {
        json_t *json_row = json_array();
        if (!json_row)
        {
            json_decref(json_matrix);
            return NULL;
        }

        for (int j = 0; j < M->c; j++)
        {
            json_t *json_value = json_real(M->data[i * M->c + j]);
            if (!json_value)
            {
                json_decref(json_row);
                json_decref(json_matrix);
                return NULL;
            }
            json_array_append_new(json_row, json_value);
        }

        json_array_append_new(json_matrix, json_row);
    }

    char *json_str = json_dumps(json_matrix, JSON_ENCODE_ANY);
    json_decref(json_matrix);

    return json_str;
}

int
Matrix_equals(const Matrix *A, Matrix *B)
{
    if ((A->c != B->c) || (A->r != B->r))
    {
        return 0;
    }

    for (int i = 0; i < A->r * A->c; i++)
    {
        if (A->data[i] != B->data[i])
        {
            return 0;
        }
    }

    return 1;
}

Matrix *
Matrix_concat(const Matrix *A, const Matrix *B, int axis)
{
    Matrix *C;
    if (axis == MAT_APPLY_COL)
    {
        if (A->r != B->r)
        {
            LOG_ERROR("Matrix_concat: matrices dimensions do not match for axis col");
            return NULL;
        }
        C = Matrix_new(A->r, A->c + B->c);
        for (int i = 0; i < C->r; i++)
        {
            for (int j = 0; j < A->c; j++)
            {
                C->data[i * C->c + j] = A->data[i * A->c + j];
            }
            for (int j = 0; j < B->c; j++)
            {
                C->data[i * C->c + A->c + j] = B->data[i * B->c + j];
            }
        }
    }
    else if (axis == MAT_APPLY_ROW)
    {
        if (A->c != B->c)
        {
            LOG_ERROR("Matrix_concat: matrices dimensions do not match for axis row");
            return NULL;
        }
        C = Matrix_new(A->r + B->r, A->c);
        for (int i = 0; i < A->r; i++)
        {
            for (int j = 0; j < C->c; j++)
            {
                C->data[i * C->c + j] = A->data[i * A->c + j];
            }
        }
        for (int i = 0; i < B->r; i++)
        {
            for (int j = 0; j < C->c; j++)
            {
                C->data[(A->r + i) * C->c + j] = B->data[i * B->c + j];
            }
        }
    }
    else
    {
        LOG_ERROR("Matrix_concat : invalid axis");
        return NULL;
    }
    return C;
}

// void multiply_matrices_blocked(int **A, int **B, int **C, int m, int n, int
// p)
// {
//     for (int i = 0; i < m; i += BLOCK_SIZE)
//     {
//         for (int j = 0; j < p; j += BLOCK_SIZE)
//         {
//             for (int k = 0; k < n; k += BLOCK_SIZE)
//             {
//                 // Multiplication des blocs
//                 for (int ii = i; ii < i + BLOCK_SIZE && ii < m; ii++)
//                 {
//                     for (int jj = j; jj < j + BLOCK_SIZE && jj < p; jj++)
//                     {
//                         int sum = 0;
//                         for (int kk = k; kk < k + BLOCK_SIZE && kk < n;
//                         kk++)
//                         {
//                             sum += A[ii][kk] * B[kk][jj];
//                         }
//                         C[ii][jj] += sum;
//                     }
//                 }
//             }
//         }
//     }
// }
//
// void multiply_matrices_SIMD(float **A, float **B, float **C, int m, int n,
// int p)
// {
//     for (int i = 0; i < m; i++)
//     {
//         for (int j = 0; j < p; j++)
//         {
//             __m256 sum = _mm256_setzero_ps(); // vecteur de somme initialisé
//             à zéro for (int k = 0; k < n; k += 8)
//             {
//                 __m256 a = _mm256_loadu_ps(&A[i][k]); // charge 8 éléments
//                 de A
//                 __m256 b = _mm256_loadu_ps(&B[k][j]); // charge 8 éléments
//                 de B sum = _mm256_fmadd_ps(a, b, sum);     // multiplication
//                 et addition
//             }
//             // Ajout des éléments du vecteur pour obtenir un scalaire
//             C[i][j] = _mm256_reduce_add_ph(sum);
//         }
//     }
// }
//
// void multiply_matrices_asm(matrix *A, matrix *B, matrix *C)
// {
//     __asm__ volatile("mov %3, %%rsi\n\t"  // Charger n dans rsi (taille de
//     la matrice)
//                      "xor %%r8, %%r8\n\t" // r8 sera notre index pour les
//                      lignes "loop_i:\n\t"
//
//                      "xor %%r9, %%r9\n\t" // r9 sera notre index pour les
//                      colonnes "loop_j:\n\t"
//
//                      "vxorps %%ymm0, %%ymm0, %%ymm0\n\t" // Initialiser le
//                      registre YMM0 à 0 (accumulateur pour C[i][j]) "xor
//                      %%r10, %%r10\n\t"              // r10 sera notre index
//                      pour la boucle k
//
//                      "loop_k:\n\t"
//                      "vmovups (%0, %%r8, 4), %%ymm1\n\t"      // Charger
//                      A[i, k] dans ymm1 "vmovups (%1, %%r9, 4), %%ymm2\n\t"
//                      // Charger B[k, j] dans ymm2 "vfmadd231ps %%ymm1,
//                      %%ymm2, %%ymm0\n\t" // C[i, j] += A[i, k] * B[k, j]
//
//                      "add $8, %%r10\n\t"    // k += 8
//                      "cmp %%rsi, %%r10\n\t" // Comparer k avec n
//                      "jl loop_k\n\t"        // Répéter pour chaque élément k
//
//                      "vmovups %%ymm0, (%2, %%r8, 4)\n\t" // Stocker le
//                      résultat C[i, j] "add $8, %%r9\n\t"                  //
//                      j += 8 "cmp %%rsi, %%r9\n\t"               // Comparer
//                      j avec n "jl loop_j\n\t"                     // Répéter
//                      pour chaque colonne j
//
//                      "add $8, %%r8\n\t"    // i += 8
//                      "cmp %%rsi, %%r8\n\t" // Comparer i avec n
//                      "jl loop_i\n\t"       // Répéter pour chaque ligne i
//                      :
//                      : "r"(A->data), "r"(B->data), "r"(C->data), "r"(C->c)
//                      : "rsi", "r8", "r9", "r10", "ymm0", "ymm1", "ymm2",
//                      "memory");
// }
