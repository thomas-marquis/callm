#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <bf16.h>

#define N 1000000
#define ITER 10

double get_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

int main() {
    bf16_t *a = malloc(N * sizeof(bf16_t));
    bf16_t *b = malloc(N * sizeof(bf16_t));
    bf16_t *out = malloc(N * sizeof(bf16_t));

    for (int i = 0; i < N; i++) {
        a[i] = bf16_from_bits(0x3F80);
        b[i] = bf16_from_bits(0x3F00);
    }

    printf("Benchmarking bf16_add_arr with N=%d, ITER=%d\n", N, ITER);

    double start = get_time();
    for (int it = 0; it < ITER; it++) {
        bf16_add_arr(out, a, b, N);
    }
    double end = get_time();

    double total_time = end - start;
    double avg_time = total_time / ITER;
    printf("Total time: %.6f s\n", total_time);
    printf("Average time: %.6f s\n", avg_time);
    printf("Throughput: %.2f Mops/s\n", (N / avg_time) / 1e6);

    free(a);
    free(b);
    free(out);

    return 0;
}
