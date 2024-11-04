#include "safetensors.h"
#include "utils.h"
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

int main()
{
    int fd;
    char *file_path = "model2.safetensors";

    fd = open(file_path, O_RDONLY);
    if (fd == -1)
    {
        perror("Error opening file");
        return 1;
    }

    st_header *h = (st_header *)malloc(sizeof(st_header));
    CHECK_MALLOC(h, "base header");

    if (st_read_header(fd, h))
    {
        return 1;
    }

    free(h);
    close(fd);
    return 0;
}
