#include "./tokenizer.h"
#include "../utils/dynamic_list.h"
#include "../utils/errors.h"
#include "../utils/json.h"
#include "../utils/linear_map.h"
#include "../utils/logging.h"
#include <assert.h>
#include <fcntl.h>
#include <jansson.h>
#include <pcre2.h>
#include <regex.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ENCODER_HASH_TABLE_DEFAULT_SIZE 256
#define REG_MAX_GROUP_MATCHES 10 * 2

static PCRE2_SPTR TOKENIZATION_REGEXP
    = (PCRE2_SPTR) "(?i:(?:'s|'t|'re|'ve|'m|'ll|'d))|[^\r\n\\w]?\\w+|\\d{1,3}|"
                   " "
                   "?[^\\s\\w]+[\r\n]*|\\s*[\r\n]+|\\s+(?!\\S)|\\s+";

static char *
load_file_content (char *file_path)
{
  FILE *file = fopen (file_path, "r");
  if (file == NULL)
    {
      printerr ("Impossible to open tokenizer json file");
      exit (1);
    }

  fseek (file, 0, SEEK_END);
  long file_size = ftell (file);
  rewind (file);

  char *buffer = (char *)malloc ((file_size + 1) * sizeof (char));
  CHECK_MALLOC_PANIC (buffer, "create tokenizer content buffer");

  size_t read_size = fread (buffer, sizeof (char), file_size, file);
  assert (read_size == file_size
          && "Inconsistent tokenizer file size on reading.");
  buffer[file_size] = '\0';

  fclose (file);

  char *content = (char *)malloc ((file_size + 1) * sizeof (char));
  strcpy (content, buffer);

  free (buffer);
  return content;
}

static CallmStatusCode
parse_tokenizer_file (char *content, LinearMap *encoder)
{
  json_error_t error;
  json_t *root = json_loads (content, 0, &error);
  if (!root)
    {
      LOGF_ERROR ("JSON error: on line %d: %s", error.line, error.text);
      return ERROR;
    }

  json_t *model_object = GET_JSON_OBJECT (root, "model", model_object);
  json_t *vocab_object = GET_JSON_OBJECT (model_object, "vocab", vocab_object);

  LOG_INFO ("Parsing vocab object");
  char *key = NULL;
  void *value = NULL;
  int i = 0;
  json_object_foreach (vocab_object, key, value)
  {
    if (LinearMap_insert (encoder, key, value, sizeof (value)) != OK)
      {
        LOGF_ERROR ("Failed to insert vocab item at index %d", i);
        return ERROR;
      }
    i++;
  }

  LOGF_INFO ("Parsed %d items from vocab object", encoder->size);

  return OK;
}

struct get_regexp_res
{
  pcre2_code *re;
  CallmStatusCode status;
};

static struct get_regexp_res
get_regex ()
{
  int errornumber;
  PCRE2_SIZE erroffset;
  pcre2_code *re;

  // Compile the regex pattern
  re = pcre2_compile (TOKENIZATION_REGEXP, PCRE2_ZERO_TERMINATED,
                      PCRE2_CASELESS | PCRE2_UTF, &errornumber, &erroffset,
                      NULL);
  if (re == NULL)
    {
      PCRE2_UCHAR buffer[256];
      pcre2_get_error_message (errornumber, buffer, sizeof (buffer));
      LOGF_ERROR ("PCRE compilation failed at offset %d: %s", (int)erroffset,
                  buffer);
      return (struct get_regexp_res){ NULL, ERROR };
    }
  return (struct get_regexp_res){ re, OK };
}

Tokenizer *
Tokenizer_new (char *file_path)
{
  LOG_INFO ("Creating tokenizer from file");
  char *json_content = load_file_content (file_path);

  Tokenizer *tokenizer = (Tokenizer *)malloc (sizeof (Tokenizer));
  tokenizer->decoder = LinearMap_new ();
  tokenizer->encoder = LinearMap_new ();

  LOG_INFO ("Creating encoder from json");
  if (parse_tokenizer_file (json_content, tokenizer->encoder) != OK)
    {
      LOG_ERROR ("Fail to parse encoder from json");
      exit (1);
    }

  LOG_INFO ("Encoder created. Creating decoder");
  for (int i = 0; i < tokenizer->encoder->size; i++)
    {
      LinearMapItem *item = &tokenizer->encoder->items[i];
      LinearMap_insert (tokenizer->decoder, item->value, item->key,
                        sizeof (item->key));
    }

  LOG_INFO ("Decoder created. Compiling ordinary tokens regex");
  struct get_regexp_res res = get_regex ();
  if (res.status != OK)
    {
      LOG_ERROR ("Failed to compile ordinary regex");
      exit (1);
    }
  tokenizer->ordinary_regex = res.re;

  // construire la regexp des tokens spéciaux
  // encoder avec les tokens spéciaux
  // trier le decoder des tokens normaux ??

  return tokenizer;
}

CallmStatusCode
Tokenizer_free (Tokenizer *tokenizer)
{
  if (tokenizer->decoder != NULL)
    {
      LinearMap_free (tokenizer->decoder);
    }
  if (tokenizer->encoder != NULL)
    {
      LinearMap_free (tokenizer->encoder);
    }
  if (tokenizer->ordinary_regex != NULL)
    {
      pcre2_code_free (tokenizer->ordinary_regex);
    }
  return OK;
}

CallmStatusCode
Tokenizer_encode (Tokenizer *tokenizer, const char *input_str, int **token_ids)
{
  int subject_length = strlen (input_str);
  int offset = 0;
  int rc; // In order to sotre each group match
  // pcre2_match_data *match_data;
  pcre2_match_data *match_data
      = pcre2_match_data_create_from_pattern (tokenizer->ordinary_regex, NULL);
  PCRE2_SIZE *ovector;

  DynamicList *tokens_ids = DynamicList_new (sizeof (int));

  while ((rc = pcre2_match (tokenizer->ordinary_regex, (PCRE2_SPTR)input_str,
                            subject_length, offset, 0, match_data, NULL))
         >= 0)
    {
      for (int i = 0; i < rc; i++)
        {
          const char *substring_start = input_str + ovector[2 * i];
          int substring_length = ovector[2 * i + 1] - ovector[2 * i];
          char *substring = strndup (substring_start, substring_length);

          int *tok_id = (int *)LinearMap_get (tokenizer->encoder, substring);
          if (DynamicList_append (tokens_ids, tok_id) != OK)
            {
              printf ("Error appending token id to dynamic list\n");
              return ERROR;
            }
          if (tok_id == NULL)
            {
              printf ("Not found %s\n", substring);
            }
          else
            {
              printf ("Found %s -> %d\n", substring, *tok_id);
            }
        }

      // Update the offset to find the next match
      offset = ovector[1];
    }
  pcre2_match_data_free (match_data);

  // if (rc == PCRE_ERROR_NOMATCH)
  // {
  //     fprintf(stderr, "Regex match failed\n");
  // }
  // else if (rc < 0)
  // {
  //     printf("Error while matching: %d\n", rc);
  // }
  DynamicList_print (tokens_ids);
  DynamicList_free (tokens_ids);

  // Etape d'encodage sans tokens spéciaux:
  // appliquer la regexp et ittérer sur les find
  // récupérer l'id du token correspondant dans l'encoder s'il existe
  // Sinon,
  return OK;
}

void
Tokenizer_print (Tokenizer *tokenizer)
{
  if (tokenizer == NULL)
    {
      printf ("Tokenizer(NULL)\n");
      return;
    }
  printf ("Tokenizer(");
  if (tokenizer->encoder == NULL)
    {
      printf ("encoder=NULL, ");
    }
  else
    {
      printf ("encoder_size=%d, ", tokenizer->encoder->size);
    }
  if (tokenizer->decoder == NULL)
    {
      printf ("decoder=NULL)");
    }
  else
    {
      printf ("decoder_size=%d)", tokenizer->decoder->size);
    }
  printf ("\n");
}
