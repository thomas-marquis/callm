#ifndef RMS_NORM_H
#define RMS_NORM_H

#include "../core/errors.h"
#include "../core/matrix.h"
#include "../core/safetensors.h"

typedef struct rms_norm_t RMSNorm;

RMSNorm *RMSNorm_new(float epsilon, const Safetensors *st, const char *layer_name);

CallmStatusCode RMSNorm_free(RMSNorm *rms_norm);

Matrix *RMSNorm_forward(RMSNorm *rms_norm, Matrix *input);

#endif  // !#ifndef RMS_NORM_H
