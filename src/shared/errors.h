#ifndef LIB_ERRORS_H
#define LIB_ERRORS_H

#include "logging.h"
#include <stdio.h>

#define printerr(...) fprintf(stderr, __VA_ARGS__)
#define CHECK_MALLOC(ptr, msg)                                                                                         \
    if (ptr == NULL)                                                                                                   \
    {                                                                                                                  \
        LOGF_ERROR("Error allocating memory for %s", msg);                                                             \
        return 1;                                                                                                      \
    }
#define CHECK_MALLOC_RET_NULL(ptr, msg)                                                                                \
    if (ptr == NULL)                                                                                                   \
    {                                                                                                                  \
        LOGF_ERROR("Error allocating memory for %s", msg);                                                             \
        return NULL;                                                                                                   \
    }
#define CHECK_MALLOC_PANIC(ptr, msg)                                                                                   \
    if (ptr == NULL)                                                                                                   \
    {                                                                                                                  \
        LOGF_ERROR("Error allocating memory for %s", msg);                                                             \
        exit(EXIT_FAILURE);                                                                                            \
    }
#define CHECK_STATUS(status, msg, ...)                                                                                 \
    if (status != OK)                                                                                                  \
    {                                                                                                                  \
        printf(stderr, msg, __VA_ARGS__);                                                                              \
        return ERROR;                                                                                                  \
    }
#define CHECK_STATUS_RET_NULL(status, msg, ...)                                                                        \
    if (status != OK)                                                                                                  \
    {                                                                                                                  \
        printf(stderr, msg, __VA_ARGS__);                                                                              \
        return NULL;                                                                                                   \
    }
#define CHECK_STATUS_PANIC(status, msg, ...)                                                                           \
    if (status != OK)                                                                                                  \
    {                                                                                                                  \
        printf(stderr, msg, __VA_ARGS__);                                                                              \
        exit(EXIT_FAILURE);                                                                                            \
    }

#define RETURN_WHEN_NULL(ptr, msg)                                                                                     \
    do                                                                                                                 \
    {                                                                                                                  \
        if ((ptr) == NULL)                                                                                             \
        {                                                                                                              \
            LOG_ERROR(msg);                                                                                            \
            return NULL;                                                                                               \
        }                                                                                                              \
    } while (0)

typedef enum
{
    ERROR,
    OK,
    NOT_IMPLEMENTED
} CallmStatusCode;

char *CallmStatusCode_string(CallmStatusCode code);

typedef struct
{
    char *message;
} Error;

Error**
Error_new_empty_ref();

void
Error_free(Error *err);

void
Error_with_message(Error **err, const char *message);

#define CHECK_ERROR(err_ref, on_err) \
    do \
    { \
        if (*err_ref != NULL) \
        on_err \
        Error_free(*err_ref);\
    } while(0); \


#endif  // !#ifndef LIB_ERRORS_H
