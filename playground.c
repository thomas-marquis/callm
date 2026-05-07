#include <cpuid.h>
#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <x86intrin.h>

#define ARRAY_LENGTH 8
#define ITERATIONS 1000000

int
check_avx2_support()
{
    unsigned int regs[4];
    __cpuid_count(7, 0, regs[0], regs[1], regs[2], regs[3]);
    return (regs[1] & (1 << 5));  // AVX2 bit
}

uint64_t
rdtsc()
{
    return __rdtsc();
}

void
benchmark_avx2(int *a, int *b, int *result)
{
    for (int i = 0; i < ITERATIONS; i++)
    {
        __m256i vec_a = _mm256_loadu_si256((__m256i *) a);
        __m256i vec_b = _mm256_loadu_si256((__m256i *) b);
        __m256i vec_result = _mm256_add_epi32(vec_a, vec_b);
        _mm256_storeu_si256((__m256i *) result, vec_result);
    }
}

void
benchmark_loop(int *a, int *b, int *result)
{
    for (int i = 0; i < ITERATIONS; i++)
    {
        for (int j = 0; j < ARRAY_LENGTH; j++)
        {
            result[j] = a[j] + b[j];
        }
    }
}

int
main()
{
    if (check_avx2_support())
    {
        printf("AVX2 is supported\n");
    }
    else
    {
        printf("AVX2 is not supported\n");
        return EXIT_FAILURE;
    }

    int a[ARRAY_LENGTH] = { 10, 20, 30, 40, 50, 60, 70, 80 };
    int b[ARRAY_LENGTH] = { 5, 5, 5, 5, 5, 5, 5, 5 };
    int result_avx2[ARRAY_LENGTH];
    int result_loop[ARRAY_LENGTH];

    // Warm-up
    benchmark_avx2(a, b, result_avx2);
    benchmark_loop(a, b, result_loop);

    // Benchmark AVX2
    uint64_t start_avx2 = rdtsc();
    benchmark_avx2(a, b, result_avx2);
    uint64_t end_avx2 = rdtsc();
    uint64_t cycles_avx2 = end_avx2 - start_avx2;

    // Benchmark Loop
    uint64_t start_loop = rdtsc();
    benchmark_loop(a, b, result_loop);
    uint64_t end_loop = rdtsc();
    uint64_t cycles_loop = end_loop - start_loop;

    printf("AVX2 cycles: %lu\n", cycles_avx2);
    printf("Loop cycles: %lu\n", cycles_loop);
    printf("AVX2 is %.2fx faster\n", (double) cycles_loop / cycles_avx2);

    // Verify results
    for (int i = 0; i < ARRAY_LENGTH; i++)
    {
        if (result_avx2[i] != result_loop[i])
        {
            printf("Mismatch at index %d: AVX2=%d, Loop=%d\n", i, result_avx2[i], result_loop[i]);
            return 1;
        }
    }
    printf("Results match.\n");

    return 0;
}