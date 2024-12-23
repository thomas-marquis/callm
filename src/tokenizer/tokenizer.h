#ifndef TOKENIZER_H
#define TOKENIZER_H
#define PCRE2_CODE_UNIT_WIDTH 8

#include "../utils/errors.h"
#include <pcre2.h>

typedef struct Tokenizer Tokenizer;

Tokenizer *Tokenizer_new(char *filepath);

CallmStatusCode Tokenizer_encode(Tokenizer *tokenizer, const char *input_str, int **token_ids);

CallmStatusCode Tokenizer_free(Tokenizer *tokenizer);

void Tokenizer_print(Tokenizer *tokenizer);

#endif  // !#ifndef TOKENIZER_H
