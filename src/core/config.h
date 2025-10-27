#ifndef CONFIG_H
#define CONFIG_H

#include "../shared/errors.h"
#include <stddef.h>

typedef enum RopeScalingType
{
    LLAMA3
} RopeScalingType;

typedef struct config_t
{
    size_t transformers_bloc_count;
    float rms_norm_eps;
    float rope_scaling_factor;
    float rope_scaling_high_freq_factor;
    float rope_scaling_low_freq_factor;
    int rope_scaling_original_max_position_embeddings;
    RopeScalingType rope_scaling_type;
    float rope_theta;
    int head_dim;
} Config;

Config *Config_new(const char *file_path);

CallmStatusCode Config_free(Config *config);

#endif  // !#ifndef CONFIG_H
