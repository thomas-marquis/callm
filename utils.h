#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

#define printerr(...) fprintf(stderr, __VA_ARGS__)
#define CHECK_MALLOC(ptr, msg)                                                                                         \
    if (ptr == NULL)                                                                                                   \
    {                                                                                                                  \
        printerr("Error allocating memory for %s\n", msg);                                                             \
        return 1;                                                                                                      \
    }

#endif
