#ifndef TOKENIZER_C
#define TOKENIZER_C

#include "tokenizer.h"
#include "../lib/errors.h"
#include "../lib/json.h"
#include "../lib/logging.h"
#include <assert.h>
#include <fcntl.h>
#include <jansson.h>
#include <regex.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ENCODER_HASH_TABLE_DEFAULT_SIZE 256
#define REG_MAX_GROUP_MATCHES 10 * 2

static const char *TOKENIZATION_REGEXP =
    "(?i:(?:'s|'t|'re|'ve|'m|'ll|'d))|[^\r\n\\w]?\\w+|\\d{1,3}| ?[^\\s\\w]+[\r\n]*|\\s*[\r\n]+|\\s+(?!\\S)|\\s+";

static char *load_file_content(char *file_path)
{
    FILE *file = fopen(file_path, "r");
    if (file == NULL)
    {
        printerr("Impossible to open tokenizer json file");
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    char *buffer = (char *)malloc((file_size + 1) * sizeof(char));
    CHECK_MALLOC_PANIC(buffer, "create tokenizer content buffer");

    size_t read_size = fread(buffer, sizeof(char), file_size, file);
    assert(read_size == file_size && "Inconsistent tokenizer file size on reading.");
    buffer[file_size] = '\0';

    fclose(file);

    char *content = (char *)malloc((file_size + 1) * sizeof(char));
    strcpy(content, buffer);

    free(buffer);
    return content;
}

static status_t parse_tokenizer_file(char *content, linear_map_t *encoder)
{
    json_error_t error;
    json_t *root = json_loads(content, 0, &error);
    if (!root)
    {
        char *msg;
        asprintf(&msg, "JSON error: on line %d: %s\n", error.line, error.text);
        log_error(msg);
        free(msg);
        return ERROR;
    }

    json_t *model_object = GET_JSON_OBJECT(root, "model", model_object);
    json_t *vocab_object = GET_JSON_OBJECT(model_object, "vocab", vocab_object);

    log_info("Parsing vocab object");
    char *key = NULL;
    void *value = NULL;
    int i = 0;
    json_object_foreach(vocab_object, key, value)
    {
        if (LinearMap_insert(encoder, key, value, sizeof(value)) != OK)
        {
            char *msg;
            asprintf(&msg, "Failed to insert vocab item at index %d", i);
            log_error(msg);
            free(msg);
            return ERROR;
        }
        i++;
    }

    char *msg;
    asprintf(&msg, "Parsed %d items from vocab object", encoder->size);
    log_info(msg);
    free(msg);

    return OK;
}

struct get_regexp_res
{
    pcre *re;
    status_t status;
};

static struct get_regexp_res get_regex()
{
    const char *error;
    int erroffset;
    pcre *re;

    // Compile the regex pattern
    re = pcre_compile(TOKENIZATION_REGEXP, PCRE_UTF8 | PCRE_CASELESS, &error, &erroffset, NULL);
    if (re == NULL)
    {
        printf("PCRE compilation failed at offset %d: %s\n", erroffset, error);
        return (struct get_regexp_res){NULL, ERROR};
    }
    return (struct get_regexp_res){re, OK};
}

tokenizer_t *Tokenizer_new(char *file_path)
{
    log_info("Creating tokenizer from file");
    char *json_content = load_file_content(file_path);

    tokenizer_t *tokenizer = (tokenizer_t *)malloc(sizeof(tokenizer_t));
    tokenizer->decoder = LinearMap_new();
    tokenizer->encoder = LinearMap_new();

    log_info("Creating encoder from json");
    if (parse_tokenizer_file(json_content, tokenizer->encoder) != OK)
    {
        log_error("Fail to parse encoder from json");
        exit(1);
    }

    log_info("Encoder created. Creating decoder");
    for (int i = 0; i < tokenizer->encoder->size; i++)
    {
        linear_map_item_t *item = &tokenizer->encoder->items[i];
        LinearMap_insert(tokenizer->decoder, item->value, item->key, sizeof(item->key));
    }

    log_info("Decoder created. Compiling ordinary tokens regex");
    struct get_regexp_res res = get_regex();
    if (res.status != OK)
    {
        log_error("Failed to compile ordinary regex");
        exit(1);
    }
    tokenizer->ordinary_regex = res.re;

    // construire la regexp des tokens spéciaux
    // encoder avec les tokens spéciaux
    // trier le decoder des tokens normaux ??

    return tokenizer;
}

status_t Tokenizer_free(tokenizer_t *tokenizer)
{
    if (tokenizer->decoder != NULL)
    {
        LinearMap_free(tokenizer->decoder);
    }
    if (tokenizer->encoder != NULL)
    {
        LinearMap_free(tokenizer->encoder);
    }
    if (tokenizer->ordinary_regex != NULL)
    {
        pcre_free(tokenizer->ordinary_regex);
    }
    return OK;
}

//// TODO: To move in a separate file

typedef struct
{
    void *data;
    size_t item_size;
    size_t size;
    size_t capacity;
} dynamicList_t;

dynamicList_t *DynamicList_new(size_t item_size)
{
    dynamicList_t *list = (dynamicList_t *)malloc(sizeof(dynamicList_t));
    list->data = NULL;
    list->size = 0;
    list->capacity = 10; // TODO magic  number
    list->item_size = item_size;
    list->data = (void *)malloc(list->capacity * sizeof(item_size));
    return list;
}

status_t DynamicList_append(dynamicList_t *list, void *item)
{
    if (list->size == list->capacity)
    {
        list->capacity *= 2;
        list->data = realloc(list->data, list->capacity * list->item_size);
        CHECK_MALLOC(list->data, "DynamicList_append");
    }
    memcpy(list->data + list->size * list->item_size, item, list->item_size);
    list->size++;
    return OK;
}

status_t dynamicList_free(dynamicList_t *list)
{
    if (list->data != NULL)
    {
        free(list->data);
    }
    free(list);
    return OK;
}

status_t dynamicList_print(dynamicList_t *list)
{
    printf("DynamicList(");
    printf("size=%zu, ", list->size);
    printf("capacity=%zu)", list->capacity);
    printf("\n");
    printf("First 10 elements:\n");
    for (int i = 0; i < 10; i++)
    {
        printf("%d: %d\n", i, *((int *)(list->data + i * sizeof(int)))); // TODO use generic type here
    }
    return OK;
}

//// end TODO

status_t Tokenizer_encode(tokenizer_t *tokenizer, const char *input_str, int **token_ids)
{
    int ovector[REG_MAX_GROUP_MATCHES];
    int subject_length = strlen(input_str);
    int offset = 0;
    int rc; // In order to sotre each group match

    dynamicList_t *tokens_ids = DynamicList_new(sizeof(int));

    while ((rc = pcre_exec(tokenizer->ordinary_regex, NULL, input_str, subject_length, offset, 0, ovector,
                           REG_MAX_GROUP_MATCHES)) >= 0)
    {
        for (int i = 0; i < rc; i++)
        {
            const char *substring_start = input_str + ovector[2 * i];
            int substring_length = ovector[2 * i + 1] - ovector[2 * i];
            char *substring = strndup(substring_start, substring_length);

            int *tok_id = (int *)LinearMap_get(tokenizer->encoder, substring);
            if (DynamicList_append(tokens_ids, tok_id) != OK)
            {
                printf("Error appending token id to dynamic list\n");
                return ERROR;
            }
            if (tok_id == NULL)
            {
                printf("Not found %s\n", substring);
            }
            else
            {
                printf("Found %s -> %d\n", substring, *tok_id);
            }
        }

        // Update the offset to find the next match
        offset = ovector[1];
    }

    if (rc == PCRE_ERROR_NOMATCH)
    {
        fprintf(stderr, "Regex match failed\n");
    }
    else if (rc < 0)
    {
        printf("Error while matching: %d\n", rc);
    }
    dynamicList_print(tokens_ids);
    dynamicList_free(tokens_ids);

    // Etape d'encodage sans tokens spéciaux:
    // appliquer la regexp et ittérer sur les find
    // récupérer l'id du token correspondant dans l'encoder s'il existe
    // Sinon,
    return OK;
}

void Tokenizer_print(tokenizer_t *tokenizer)
{
    if (tokenizer == NULL)
    {
        printf("Tokenizer(NULL)\n");
        return;
    }
    printf("Tokenizer(");
    if (tokenizer->encoder == NULL)
    {
        printf("encoder=NULL, ");
    }
    else
    {
        printf("encoder_size=%d, ", tokenizer->encoder->size);
    }
    if (tokenizer->decoder == NULL)
    {
        printf("decoder=NULL)");
    }
    else
    {
        printf("decoder_size=%d)", tokenizer->decoder->size);
    }
    printf("\n");
}

#endif // !#ifndef TOKENIZER_C
