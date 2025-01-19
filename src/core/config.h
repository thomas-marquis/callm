#ifndef CONFIG_H
#define CONFIG_H

#include "errors.h"
#include <stddef.h>

typedef struct config_t
{
    size_t transformers_bloc_count;
} Config;

Config *Config_new(const char *file_path);

CallmStatusCode Config_free(Config *config);

#endif  // !#ifndef CONFIG_H
