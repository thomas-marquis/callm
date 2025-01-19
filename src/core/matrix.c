#include "matrix.h"
#include "logging.h"
#include <immintrin.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define BLOCK_SIZE 16

Matrix *
Matrix_new(int r, int c)
{
    Matrix *M = (Matrix *) malloc(sizeof(Matrix));
    M->r = r;
    M->c = c;
    M->data = (float *) malloc(r * c * sizeof(float));
    return M;
}

CallmStatusCode
Matrix_free(Matrix *M)
{
    free(M->data);
    free(M);
    return OK;
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

void
Matrix_apply_softmax(Matrix *M)
{
    float max = 0;
    for (int i = 0; i < M->r * M->c; i++)
    {
        if (M->data[i] > max)
        {
            max = M->data[i];
        }
    }
    float sum = 0;
    for (int i = 0; i < M->r * M->c; i++)
    {
        M->data[i] = exp(M->data[i] - max);
        sum += M->data[i];
    }
    for (int i = 0; i < M->r * M->c; i++)
    {
        M->data[i] /= sum;
    }
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

void
Matrix_print(const Matrix *M)
{
    printf("M(%dx%d)=\n", M->r, M->c);
    for (int i = 0; i < M->r; i++)
    {
        for (int j = 0; j < M->c; j++)
        {
            printf("%f ", M->data[i * M->c + j]);
        }
        printf("\n");
    }
    printf("\n");
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
