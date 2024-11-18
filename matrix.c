#ifndef MATRIX_C
#define MATRIX_C

#include "matrix.h"
#include "lib/errors.h"
#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>

#define BLOCK_SIZE 16

matrix_t *Matrix_new(int r, int c)
{
    matrix_t *M = (matrix_t *)malloc(sizeof(matrix_t));
    M->r = r;
    M->c = c;
    M->data = NULL;
    return M;
}

status_t Matrix_free(matrix_t *M)
{
    free(M->data);
    free(M);
    return OK;
}

status_t Matrix_fill(matrix_t *M, float *data)
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

status_t Matrix_dot(const matrix_t *A, const matrix_t *B, matrix_t *C)
{
    if (A->c != B->r)
    {
        printf("Error: Matrix dimensions do not match\n");
        return ERROR;
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
    return OK;
}

void Matrix_print(const matrix_t *M)
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

// void multiply_matrices_blocked(int **A, int **B, int **C, int m, int n, int p)
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
//                         for (int kk = k; kk < k + BLOCK_SIZE && kk < n; kk++)
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
// void multiply_matrices_SIMD(float **A, float **B, float **C, int m, int n, int p)
// {
//     for (int i = 0; i < m; i++)
//     {
//         for (int j = 0; j < p; j++)
//         {
//             __m256 sum = _mm256_setzero_ps(); // vecteur de somme initialisé à zéro
//             for (int k = 0; k < n; k += 8)
//             {
//                 __m256 a = _mm256_loadu_ps(&A[i][k]); // charge 8 éléments de A
//                 __m256 b = _mm256_loadu_ps(&B[k][j]); // charge 8 éléments de B
//                 sum = _mm256_fmadd_ps(a, b, sum);     // multiplication et addition
//             }
//             // Ajout des éléments du vecteur pour obtenir un scalaire
//             C[i][j] = _mm256_reduce_add_ph(sum);
//         }
//     }
// }
//
// void multiply_matrices_asm(matrix *A, matrix *B, matrix *C)
// {
//     __asm__ volatile("mov %3, %%rsi\n\t"  // Charger n dans rsi (taille de la matrice)
//                      "xor %%r8, %%r8\n\t" // r8 sera notre index pour les lignes
//                      "loop_i:\n\t"
//
//                      "xor %%r9, %%r9\n\t" // r9 sera notre index pour les colonnes
//                      "loop_j:\n\t"
//
//                      "vxorps %%ymm0, %%ymm0, %%ymm0\n\t" // Initialiser le registre YMM0 à 0 (accumulateur pour
//                      C[i][j]) "xor %%r10, %%r10\n\t"              // r10 sera notre index pour la boucle k
//
//                      "loop_k:\n\t"
//                      "vmovups (%0, %%r8, 4), %%ymm1\n\t"      // Charger A[i, k] dans ymm1
//                      "vmovups (%1, %%r9, 4), %%ymm2\n\t"      // Charger B[k, j] dans ymm2
//                      "vfmadd231ps %%ymm1, %%ymm2, %%ymm0\n\t" // C[i, j] += A[i, k] * B[k, j]
//
//                      "add $8, %%r10\n\t"    // k += 8
//                      "cmp %%rsi, %%r10\n\t" // Comparer k avec n
//                      "jl loop_k\n\t"        // Répéter pour chaque élément k
//
//                      "vmovups %%ymm0, (%2, %%r8, 4)\n\t" // Stocker le résultat C[i, j]
//                      "add $8, %%r9\n\t"                  // j += 8
//                      "cmp %%rsi, %%r9\n\t"               // Comparer j avec n
//                      "jl loop_j\n\t"                     // Répéter pour chaque colonne j
//
//                      "add $8, %%r8\n\t"    // i += 8
//                      "cmp %%rsi, %%r8\n\t" // Comparer i avec n
//                      "jl loop_i\n\t"       // Répéter pour chaque ligne i
//                      :
//                      : "r"(A->data), "r"(B->data), "r"(C->data), "r"(C->c)
//                      : "rsi", "r8", "r9", "r10", "ymm0", "ymm1", "ymm2", "memory");
// }

#endif // !MATRIX_C
