#ifndef TOKENIZER_H
#define TOKENIZER_H
#define PCRE2_CODE_UNIT_WIDTH 8

#include "../utils/errors.h"
#include "../utils/linear_map.h"
#include <pcre2.h>

typedef struct
{
  LinearMap *encoder; // token to id
  LinearMap *decoder; // id to token
  pcre2_code *ordinary_regex;
} Tokenizer;

Tokenizer *Tokenizer_new (char *filepath);

CallmStatusCode Tokenizer_encode (Tokenizer *tokenizer, const char *input_str,
                                  int **token_ids);

CallmStatusCode Tokenizer_free (Tokenizer *tokenizer);

void Tokenizer_print (Tokenizer *tokenizer);

#endif // !#ifndef TOKENIZER_H
