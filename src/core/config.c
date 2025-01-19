#include "config.h"
#include "errors.h"
#include "logging.h"
#include <jansson.h>
#include <stdlib.h>

Config *
Config_new(const char *file_path)
{
    Config *config = (Config *) malloc(sizeof(Config));
    CHECK_MALLOC_RET_NULL(config, "config");

    json_error_t error;
    json_t *root = json_load_file(file_path, 0, &error);
    if (!root)
    {
        LOGF_ERROR("Error while parsing config file: %s", error.text);
        free(config);
        return NULL;
    }

    json_t *transformers_bloc_count = json_object_get(root, "transformers_bloc_count");
    if (!json_is_integer(transformers_bloc_count))
    {
        LOG_ERROR("Error: transformers_bloc_count is not an integer");
        json_decref(root);
        free(config);
        return NULL;
    }

    config->transformers_bloc_count = json_integer_value(transformers_bloc_count);
    json_decref(root);

    return config;
}

CallmStatusCode
Config_free(Config *config)
{
    if (config == NULL)
    {
        return ERROR;
    }

    free(config);
    return OK;
}
