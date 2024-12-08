#ifndef LIB_LOGGING_H
#define LIB_LOGGING_H

#define LOGF_INFO(msg, ...)                                                                                            \
    do                                                                                                                 \
    {                                                                                                                  \
        char *formatted_msg;                                                                                           \
        if (asprintf(&formatted_msg, msg, __VA_ARGS__) == -1)                                                          \
        {                                                                                                              \
            fprintf(stderr, "[error] %s:%d in %s(): logging error, imporssible to format message: %s\n", __FILE__,     \
                    __LINE__, __func__, msg);                                                                          \
        }                                                                                                              \
        fprintf(stdout, "[INFO] %s:%d in %s(): %s\n", __FILE__, __LINE__, __func__, formatted_msg);                    \
    } while (0)

#define LOG_INFO(msg)                                                                                                  \
    do                                                                                                                 \
    {                                                                                                                  \
        fprintf(stdout, "[INFO] %s:%d in %s(): %s\n", __FILE__, __LINE__, __func__, msg);                              \
    } while (0)

#define LOGF_ERROR(msg, ...)                                                                                           \
    do                                                                                                                 \
    {                                                                                                                  \
        char *formatted_msg;                                                                                           \
        if (asprintf(&formatted_msg, msg, __VA_ARGS__) == -1)                                                          \
        {                                                                                                              \
            fprintf(stderr, "[error] %s:%d in %s(): logging error, imporssible to format message: %s\n", __FILE__,     \
                    __LINE__, __func__, msg);                                                                          \
        }                                                                                                              \
        fprintf(stderr, "[error] %s:%d in %s(): %s\n", __FILE__, __LINE__, __func__, formatted_msg);                   \
    } while (0)

#define LOG_ERROR(msg)                                                                                                 \
    do                                                                                                                 \
    {                                                                                                                  \
        fprintf(stderr, "[error] %s:%d in %s(): %s\n", __FILE__, __LINE__, __func__, msg);                             \
    } while (0)

#endif // !#ifndef LIB_LOGGING_H
