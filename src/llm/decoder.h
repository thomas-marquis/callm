#ifndef DECODER_H
#define DECODER_H

#include "../core/config.h"
#include "../core/matrix.h"
#include "../core/safetensors.h"
#include "../shared/errors.h"

typedef struct decoder_t Decoder;

Decoder *Decoder_new(Safetensors *st, const Config *config, unsigned int loayer_idx);

CallmStatusCode Decoder_free(Decoder *decoder);

Matrix *Decoder_forward(Decoder *decoder, Matrix *hidden_state);

#endif  // !#ifndef DECODER_H
