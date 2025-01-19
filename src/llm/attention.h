#ifndef ATTENTION_H
#define ATTENTION_H

#include "../core/config.h"
#include "../core/matrix.h"
#include "../core/safetensors.h"

typedef struct attention Attention;

Attention *Attention_new(Safetensors *st, int layer, const Config *config);

void Attention_free(Attention *at);

Matrix *Attention_forward(Attention *at, Matrix *input);

#endif  // !#ifndef ATTENTION_H
