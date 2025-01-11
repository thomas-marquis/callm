#ifndef LIB_ERRORS_H
#define LIB_ERRORS_H

#include <stdio.h>

#define printerr(...) fprintf(stderr, __VA_ARGS__)
#define CHECK_MALLOC(ptr, msg)                                                                                         \
    if (ptr == NULL)                                                                                                   \
    {                                                                                                                  \
        printerr("Error allocating memory for %s\n", msg);                                                             \
        return 1;                                                                                                      \
    }
#define CHECK_MALLOC_PANIC(ptr, msg)                                                                                   \
    if (ptr == NULL)                                                                                                   \
    {                                                                                                                  \
        printerr("Error allocating memory for %s\n", msg);                                                             \
        exit(1);                                                                                                       \
    }
#define CHECK_STATUS(status, msg, ...)                                                                                 \
    if (status != OK)                                                                                                  \
    {                                                                                                                  \
        printf(stderr, msg, __VA_ARGS__);                                                                              \
        return ERROR;                                                                                                  \
    }
#define CHECK_STATUS_PANIC(status, msg, ...)                                                                           \
    if (status != OK)                                                                                                  \
    {                                                                                                                  \
        printf(stderr, msg, __VA_ARGS__);                                                                              \
        exit(1);                                                                                                       \
    }

typedef enum
{
    ERROR,
    OK,
    NOT_IMPLEMENTED
} CallmStatusCode;

char *CallmStatusCode_string(CallmStatusCode code);

#endif  // !#ifndef LIB_ERRORS_H
