#define _POSIX_C_SOURCE 199309L
#include <bf16.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 1000000
#define ITER 10

// Temporarily override the AVX2 detection to force scalar mode
static bool original_avx2_support;
static bool force_scalar_mode = false;

bool
bf16_cpu_has_avx2_override()
{
    return original_avx2_support && !force_scalar_mode;
}

// Save the original function pointer and replace it
void
setup_benchmark()
{
    original_avx2_support = bf16_cpu_has_avx2();
}

double
get_time()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

void
benchmark_bf16_add()
{
#define N 10000000
#define ITER 100

    bf16_t *a = aligned_alloc(32, N * sizeof(bf16_t));
    bf16_t *b = aligned_alloc(32, N * sizeof(bf16_t));
    bf16_t *out_simd = aligned_alloc(32, N * sizeof(bf16_t));
    bf16_t *out_scalar = aligned_alloc(32, N * sizeof(bf16_t));

    for (int i = 0; i < N; i++)
    {
        a[i] = bf16_from_bits(0x3F80);  // 1.0
        b[i] = bf16_from_bits(0x3F00);  // 0.5
    }

    // Warm-up
    for (int it = 0; it < 2; it++)
    {
        bf16_add_arr(out_simd, a, b, N);
    }

    // Benchmark SIMD (AVX2 enabled)
    double start_simd = get_time();
    for (int it = 0; it < ITER; it++)
    {
        bf16_add_arr(out_simd, a, b, N);
    }
    double end_simd = get_time();
    double avg_time_simd = (end_simd - start_simd) / ITER;

    // Benchmark Scalar (AVX2 disabled)
    force_scalar_mode = true;
    for (int it = 0; it < 2; it++)
    {
        bf16_add_arr(out_scalar, a, b, N);
    }

    double start_scalar = get_time();
    for (int it = 0; it < ITER; it++)
    {
        bf16_add_arr(out_scalar, a, b, N);
    }
    double end_scalar = get_time();
    double avg_time_scalar = (end_scalar - start_scalar) / ITER;

    force_scalar_mode = false;  // Reset

    // Verify results
    for (int i = 0; i < N; i++)
    {
        if (out_simd[i] != out_scalar[i])
        {
            fprintf(stderr, "Error: SIMD and scalar results differ at index %d\n", i);
            free(a);
            free(b);
            free(out_simd);
            free(out_scalar);
            exit(EXIT_FAILURE);
        }
    }

    // Print results
    printf("Benchmarking bf16_add_arr with N=%d, ITER=%d\n", N, ITER);
    printf("SIMD (AVX2):\n");
    printf("  Average time: %.6f s\n", avg_time_simd);
    printf("  Throughput: %.2f Mops/s\n", (N / avg_time_simd) / 1e6);

    printf("Scalar:\n");
    printf("  Average time: %.6f s\n", avg_time_scalar);
    printf("  Throughput: %.2f Mops/s\n", (N / avg_time_scalar) / 1e6);

    double speedup = avg_time_scalar / avg_time_simd;
    printf("Speedup (SIMD vs Scalar): %.2fx\n", speedup);

    if (speedup <= 1.0)
    {
        fprintf(stderr, "Error: SIMD is NOT faster than scalar (speedup: %.2fx)\n", speedup);
        free(a);
        free(b);
        free(out_simd);
        free(out_scalar);
        exit(EXIT_FAILURE);
    }

    free(a);
    free(b);
    free(out_simd);
    free(out_scalar);
}

int
main()
{
    bf16_t *a = malloc(N * sizeof(bf16_t));
    bf16_t *b = malloc(N * sizeof(bf16_t));
    bf16_t *out_simd = malloc(N * sizeof(bf16_t));
    bf16_t *out_scalar = malloc(N * sizeof(bf16_t));

    for (int i = 0; i < N; i++)
    {
        a[i] = bf16_from_bits(0x3F80);
        b[i] = bf16_from_bits(0x3F00);
    }

    printf("Benchmarking bf16_add_arr with N=%d, ITER=%d\n", N, ITER);

    double start = get_time();
    for (int it = 0; it < ITER; it++)
    {
        bf16_add_arr(out_simd, a, b, N);
    }
    double end = get_time();

    double total_time = end - start;
    double avg_time = total_time / ITER;
    printf("Total time: %.6f s\n", total_time);
    printf("Average time: %.6f s\n", avg_time);
    printf("Throughput: %.2f Mops/s\n", (N / avg_time) / 1e6);

    free(a);
    free(b);
    free(out_simd);
    free(out_scalar);

    return 0;
}
