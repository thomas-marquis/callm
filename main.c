#include "safetensors.h"
#include <fcntl.h>
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>

int main()
{
    char *file_path = "model2.safetensors";
    // char *file_path = "my_tensor.safetensors";
    safetensors_t *h = Safetensors_new(file_path);

    // matrix_t *M = Safetensors_load_matrix("my_tensor", h);
    // Matrix_print(M);
    // Matrix_free(M);
    //
    // matrix_t *N = Safetensors_load_matrix("my_tensor_float32", h);
    // Matrix_print(N);
    // Matrix_free(N);

    Safetensors_free(h);
    return 0;
}
