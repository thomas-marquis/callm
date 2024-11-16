#include "safetensors.h"
#include "utils.h"
#include <fcntl.h>
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>

int main()
{
    // char *file_path = "model2.safetensors";
    char *file_path = "my_tensor.safetensors";
    st_header *h = new_st_header(file_path);

    matrix *M = st_load_matrix("my_tensor", h);
    matrix_print(M);
    matrix_free(M);

    matrix *N = st_load_matrix("my_tensor_float32", h);
    matrix_print(N);
    matrix_free(N);

    st_header_free(h);
    return 0;
}
