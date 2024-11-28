#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "../lib/errors.h"
#include "../lib/linear_map.h"
#include <pcre.h>

typedef struct Tokenizer
{
    linear_map_t *encoder; // token to id
    linear_map_t *decoder; // id to token
    pcre *ordinary_regex;
} tokenizer_t;

tokenizer_t *Tokenizer_new(char *filepath);

status_t Tokenizer_encode(tokenizer_t *tokenizer, const char *input_str, int **token_ids);

status_t Tokenizer_free(tokenizer_t *tokenizer);

void Tokenizer_print(tokenizer_t *tokenizer);

#endif // !#ifndef TOKENIZER_H
