#ifndef LIB_LOGGING_H
#define LIB_LOGGING_H

#define log_info(msg) fprintf(stdout, "[INFO] %s:%d in %s(): %s\n", __FILE__, __LINE__, __func__, msg)

#define log_error(msg) fprintf(stderr, "[ERROR] %s:%d in %s(): %s\n", __FILE__, __LINE__, __func__, msg)

#endif // !#ifndef LIB_LOGGING_H
