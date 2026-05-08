#define _POSIX_C_SOURCE 199309L
#include <bf16.h>
#include <bf16_bench.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 100000
#define ITER 100

// CPU feature detection cache
static bool avx2_available = false;

bool
bf16_cpu_has_avx2_cached()
{
    if (avx2_available == false)
    {
        avx2_available = bf16_cpu_has_avx2();
    }
    return avx2_available;
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
    bf16_t *a = aligned_alloc(32, N * sizeof(bf16_t));
    bf16_t *b = aligned_alloc(32, N * sizeof(bf16_t));
    bf16_t *out_simd = aligned_alloc(32, N * sizeof(bf16_t));
    bf16_t *out_scalar = aligned_alloc(32, N * sizeof(bf16_t));

    for (int i = 0; i < N; i++)
    {
        a[i] = bf16_from_bits(0x3F80);  // 1.0
        b[i] = bf16_from_bits(0x3F00);  // 0.5
    }

    // Benchmark Scalar
    double start_scalar = get_time();
    for (int it = 0; it < ITER; it++)
    {
        bf16_add_arr_scalar(out_scalar, a, b, N);
    }
    double end_scalar = get_time();
    double avg_time_scalar = (end_scalar - start_scalar) / ITER;

    // Benchmark SIMD (AVX2) - if available
    double avg_time_simd = 0.0;
    bool simd_ran = false;

#ifdef CALLM_ENABLE_AVX2
    if (bf16_cpu_has_avx2_cached())
    {
        for (int it = 0; it < 2; it++)
        {
            bf16_add_arr_avx2(out_simd, a, b, N);
        }

        double start_simd = get_time();
        for (int it = 0; it < ITER; it++)
        {
            bf16_add_arr_avx2(out_simd, a, b, N);
        }
        double end_simd = get_time();
        avg_time_simd = (end_simd - start_simd) / ITER;
        simd_ran = true;
    }
#endif

    // Verify results
    for (int i = 0; i < N; i++)
    {
#ifdef CALLM_ENABLE_AVX2
        if (simd_ran && out_simd[i] != out_scalar[i])
        {
            fprintf(stderr, "Error: SIMD and scalar results differ at index %d (simd=0x%04X, scalar=0x%04X)\n", i,
                    bf16_to_bits(out_simd[i]), bf16_to_bits(out_scalar[i]));
            free(a);
            free(b);
            free(out_simd);
            free(out_scalar);
            exit(EXIT_FAILURE);
        }
#else
        (void) out_simd;  // Unused if AVX2 not enabled
#endif
    }

    // Print results
    printf("Benchmarking bf16_add_arr with N=%d, ITER=%d\n", N, ITER);

    printf("Scalar:\n");
    printf("  Average time: %.6f s\n", avg_time_scalar);
    printf("  Throughput: %.2f Mops/s\n", (N / avg_time_scalar) / 1e6);

#ifdef CALLM_ENABLE_AVX2
    if (simd_ran)
    {
        printf("SIMD (AVX2):\n");
        printf("  Average time: %.6f s\n", avg_time_simd);
        printf("  Throughput: %.2f Mops/s\n", (N / avg_time_simd) / 1e6);

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
    }
    else
    {
        printf("SIMD (AVX2): Not available on this CPU\n");
    }
#else
    printf("SIMD (AVX2): Compiled without AVX2 support\n");
#endif

    free(a);
    free(b);
    free(out_simd);
    free(out_scalar);
}

int
main()
{
    benchmark_bf16_add();
    return 0;
}
