#include "config.h"
#include "../shared/errors.h"
#include "../shared/logging.h"
#include <jansson.h>
#include <stdlib.h>
#include <string.h>

#define HANDLE_NOT_INT(value, key)                                                                                     \
    if (!json_is_integer(value))                                                                                       \
    {                                                                                                                  \
        LOGF_ERROR("%s is not an integer", key);                                                                       \
        json_decref(root);                                                                                             \
        free(config);                                                                                                  \
        return NULL;                                                                                                   \
    }

#define HANDLE_NOT_FLOAT(value, key)                                                                                   \
    if (!json_is_real(value))                                                                                          \
    {                                                                                                                  \
        LOGF_ERROR("%s is not a float", key);                                                                          \
        json_decref(root);                                                                                             \
        free(config);                                                                                                  \
        return NULL;                                                                                                   \
    }

#define HANDLE_NOT_OBJECT(value, key)                                                                                  \
    if (!json_is_object(value))                                                                                        \
    {                                                                                                                  \
        LOGF_ERROR("%s is not an object", key);                                                                        \
        json_decref(root);                                                                                             \
        free(config);                                                                                                  \
        return NULL;                                                                                                   \
    }

#define HANDLE_NOT_STRING(value, key)                                                                                  \
    if (!json_is_string(value))                                                                                        \
    {                                                                                                                  \
        LOGF_ERROR("%s is not a string", key);                                                                         \
        json_decref(root);                                                                                             \
        free(config);                                                                                                  \
        return NULL;                                                                                                   \
    }

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
    HANDLE_NOT_INT(transformers_bloc_count, "transformers_bloc_count");
    config->transformers_bloc_count = json_integer_value(transformers_bloc_count);
    json_decref(transformers_bloc_count);

    json_t *head_dim = json_object_get(root, "head_dim");
    HANDLE_NOT_INT(head_dim, "head_dim");
    config->head_dim = json_integer_value(head_dim);
    json_decref(head_dim);

    json_t *rms_norm_eps = json_object_get(root, "rms_norm_eps");
    HANDLE_NOT_FLOAT(rms_norm_eps, "rms_norm_eps");
    config->rms_norm_eps = json_real_value(rms_norm_eps);
    json_decref(rms_norm_eps);

    // Rope scaling
    json_t *rope_scaling = json_object_get(root, "rope_scaling");
    HANDLE_NOT_OBJECT(rope_scaling, "rope_scaling");

    json_t *rope_scaling_factor = json_object_get(rope_scaling, "factor");
    HANDLE_NOT_FLOAT(rope_scaling_factor, "rope_scaling.factor");
    config->rope_scaling_factor = json_real_value(rope_scaling_factor);
    json_decref(rope_scaling_factor);

    json_t *rope_scaling_high_freq_factor = json_object_get(rope_scaling, "high_freq_factor");
    HANDLE_NOT_FLOAT(rope_scaling_high_freq_factor, "rope_scaling.high_freq_factor");
    config->rope_scaling_high_freq_factor = json_real_value(rope_scaling_high_freq_factor);
    json_decref(rope_scaling_high_freq_factor);

    json_t *rope_scaling_low_freq_factor = json_object_get(rope_scaling, "low_freq_factor");
    HANDLE_NOT_FLOAT(rope_scaling_low_freq_factor, "rope_scaling.low_freq_factor");
    config->rope_scaling_low_freq_factor = json_real_value(rope_scaling_low_freq_factor);
    json_decref(rope_scaling_low_freq_factor);

    json_t *rope_scaling_original_max_position_embeddings
        = json_object_get(rope_scaling, "original_max_position_embeddings");
    HANDLE_NOT_INT(rope_scaling_original_max_position_embeddings, "rope_scaling.original_max_position_embeddings");
    config->rope_scaling_original_max_position_embeddings
        = json_integer_value(rope_scaling_original_max_position_embeddings);
    json_decref(rope_scaling_original_max_position_embeddings);

    json_t *rope_scaling_type = json_object_get(rope_scaling, "rope_type");
    HANDLE_NOT_STRING(rope_scaling_type, "rope_scaling.rope_type");
    const char *rope_type = json_string_value(rope_scaling_type);
    if (strcmp(rope_type, "llama3") == 0)
    {
        config->rope_scaling_type = LLAMA3;
    }
    else
    {
        LOGF_ERROR("Unknown rope type: %s", rope_type);
        json_decref(root);
        free(config);
        return NULL;
    }
    json_decref(rope_scaling_type);

    json_decref(rope_scaling);
    // End of rope scaling

    json_t *rope_theta = json_object_get(root, "rope_theta");
    HANDLE_NOT_FLOAT(rope_theta, "rope_theta");
    config->rope_theta = json_real_value(rope_theta);
    json_decref(rope_theta);

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
