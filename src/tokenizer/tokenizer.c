#include "./tokenizer.h"
#include "../utils/errors.h"
#include "../utils/json.h"
#include "../utils/linked_list.h"
#include "../utils/logging.h"
#include "../utils/uthash.h"
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

static PCRE2_SPTR TOKENIZATION_REGEXP = (PCRE2_SPTR) "(?i:(?:'s|'t|'re|'ve|'m|'ll|'d))|[^\r\n\\w]?\\w+|\\d{1,3}|"
                                                     " "
                                                     "?[^\\s\\w]+[\r\n]*|\\s*[\r\n]+|\\s+(?!\\S)|\\s+";

typedef struct
{
    int id;
    char *token;
    UT_hash_handle hh;
} Token;

struct Tokenizer
{
    Token *encoder;  // token to id
    Token *decoder;  // id to token
    pcre2_code *ordinary_regex;
};

static char *
load_file_content(char *file_path)
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

    char *buffer = (char *) malloc((file_size + 1) * sizeof(char));
    CHECK_MALLOC_PANIC(buffer, "create tokenizer content buffer");

    size_t read_size = fread(buffer, sizeof(char), file_size, file);
    assert(read_size == file_size && "Inconsistent tokenizer file size on reading.");
    buffer[file_size] = '\0';

    fclose(file);

    char *content = (char *) malloc((file_size + 1) * sizeof(char));
    strcpy(content, buffer);

    free(buffer);
    return content;
}

static CallmStatusCode
parse_tokenizer_file(char *content, Token *encoder)
{
    json_error_t error;
    json_t *root = json_loads(content, 0, &error);
    if (!root)
    {
        LOGF_ERROR("JSON error: on line %d: %s", error.line, error.text);
        return ERROR;
    }

    json_t *model_object = GET_JSON_OBJECT(root, "model", model_object);
    json_t *vocab_object = GET_JSON_OBJECT(model_object, "vocab", vocab_object);

    LOG_INFO("Parsing vocab object");
    char *key = NULL;
    json_t *value = NULL;
    int i = 0;
    json_object_foreach(vocab_object, key, value)
    {
        Token *t;
        t = (Token *) malloc(sizeof(Token));
        int t_id;
        if (json_unpack(value, "i", &t_id) == -1)
        {
            LOGF_ERROR("Failed to parse token in input encoder at index %d", i);
            return ERROR;
        }
        t->id = t_id;
        t->token = key;
        HASH_ADD_STR(encoder, token, t);
        i++;
    }

    LOGF_INFO("Parsed %d items from vocab object", HASH_COUNT(encoder));

    return OK;
}

struct get_regexp_res
{
    pcre2_code *re;
    CallmStatusCode status;
};

static struct get_regexp_res
get_regex()
{
    pcre2_code *re;
    PCRE2_SIZE erroffset;
    int errcode;
    PCRE2_UCHAR8 buffer[128];

    re = pcre2_compile(TOKENIZATION_REGEXP, PCRE2_ZERO_TERMINATED, PCRE2_CASELESS | PCRE2_UTF, &errcode, &erroffset,
                       NULL);
    if (re == NULL)
    {
        pcre2_get_error_message(errcode, buffer, sizeof(buffer));
        LOGF_ERROR("PCRE compilation failed at offset %d: %s", (int) erroffset, buffer);
        return (struct get_regexp_res){ NULL, ERROR };
    }
    return (struct get_regexp_res){ re, OK };
}

Tokenizer *
Tokenizer_new(char *file_path)
{
    LOG_INFO("Creating tokenizer from file");
    char *json_content = load_file_content(file_path);

    Tokenizer *tokenizer = (Tokenizer *) malloc(sizeof(struct Tokenizer));
    tokenizer->decoder = NULL;
    tokenizer->encoder = NULL;

    LOG_INFO("Creating encoder from json");
    if (parse_tokenizer_file(json_content, tokenizer->encoder) != OK)
    {
        LOG_ERROR("Fail to parse encoder from json");
        exit(1);
    }

    LOG_INFO("Encoder created. Creating decoder");
    Token *t;
    for (t = tokenizer->encoder; t != NULL; t = t->hh.next)
    {
        HASH_ADD_INT(tokenizer->decoder, id, t);
    }

    LOG_INFO("Decoder created. Compiling ordinary tokens regex");
    struct get_regexp_res res = get_regex();
    if (res.status != OK)
    {
        LOG_ERROR("Failed to compile ordinary regex");
        exit(1);
    }
    tokenizer->ordinary_regex = res.re;

    // construire la regexp des tokens spéciaux
    // encoder avec les tokens spéciaux
    // trier le decoder des tokens normaux ??

    return tokenizer;
}

CallmStatusCode
Tokenizer_free(Tokenizer *tokenizer)
{
    if (tokenizer->decoder != NULL)
    {
        HASH_CLEAR(hh, tokenizer->encoder);
    }
    if (tokenizer->encoder != NULL)
    {
        HASH_CLEAR(hh, tokenizer->decoder);
    }
    if (tokenizer->ordinary_regex != NULL)
    {
        pcre2_code_free(tokenizer->ordinary_regex);
    }
    return OK;
}

static CallmStatusCode
split_text(char *input_str, pcre2_code *re, LinkedList *out)
{
    assert(out != NULL && "Output list is NULL");

    uint32_t option_bits;
    uint32_t newline;
    int crlf_is_newline;
    int utf8;

    /* Before running the loop, check for UTF-8 and whether CRLF is a valid newline
    sequence. First, find the options with which the regex was compiled and extract
    the UTF state. */
    (void) pcre2_pattern_info(re, PCRE2_INFO_ALLOPTIONS, &option_bits);
    utf8 = (option_bits & PCRE2_UTF) != 0;

    /* Now find the newline convention and see whether CRLF is a valid newline
    sequence. */
    (void) pcre2_pattern_info(re, PCRE2_INFO_NEWLINE, &newline);
    crlf_is_newline = newline == PCRE2_NEWLINE_ANY || newline == PCRE2_NEWLINE_CRLF || newline == PCRE2_NEWLINE_ANYCRLF;

    int subject_length = strlen(input_str);
    int offset = 0;

    pcre2_match_data *match_data = pcre2_match_data_create_from_pattern(re, NULL);

    int rc;
    PCRE2_SIZE *ovector;

    LOG_INFO("Start encoding...");
    rc = pcre2_match(re, (PCRE2_SPTR) input_str, subject_length, offset, 0, match_data, NULL);
    if (rc < 0)
    {
        switch (rc)
        {
        case PCRE2_ERROR_NOMATCH:
            LOG_ERROR("No match");
            break;
        /*
        Handle other special cases
        */
        default:
            LOGF_ERROR("Matching error %d", rc);
            break;
        }
        pcre2_match_data_free(match_data); /* Release memory used for the match */
        return ERROR;
    }
    ovector = pcre2_get_ovector_pointer(match_data);
    LOGF_INFO("Match succeeded at offset %d", (int) ovector[0]);

    /* The output vector wasn't big enough. This should not happen, because we used
    pcre2_match_data_create_from_pattern() above. */
    if (rc == 0)
    {
        LOG_ERROR("ovector was not big enough for all the captured substrings");
    }

    /* Show substrings stored in the output vector by number. Obviously, in a real
    application you might want to do things other than print them. */
    for (int i = 0; i < rc; i++)
    {
        PCRE2_SPTR substring_start = (PCRE2_SPTR) input_str + ovector[2 * i];
        size_t substring_length = ovector[2 * i + 1] - ovector[2 * i];
        LOGF_INFO("%2d: %.*s", i, (int) substring_length, (char *) substring_start);

        char *buffer = (char *) malloc(substring_length * sizeof(char) + 1);
        CHECK_MALLOC(buffer, "error allocating string buffer for first match");
        strcpy(buffer, (char *) substring_start);
        LinkedList_add(out, buffer);
    }

    while (1)
    {
        PCRE2_SIZE start_offset = ovector[1]; /* Start after the last match */
        uint32_t options = 0;

        rc = pcre2_match(re,                     /* the compiled pattern */
                         (PCRE2_SPTR) input_str, /* the subject string */
                         subject_length,         /* the length of the subject */
                         start_offset,           /* starting offset in the subject */
                         options,                /* options */
                         match_data,             /* block for storing the result */
                         NULL);

        if (rc == PCRE2_ERROR_NOMATCH)
        {
            if (options == 0)
            {
                break; /* All matches found */
            }

            ovector[1] = start_offset + 1; /* Advance one code unit */

            if (crlf_is_newline &&                   /* If CRLF is newline & */
                start_offset < subject_length - 1 && /* we are at CRLF, */
                input_str[start_offset] == '\r' && input_str[start_offset + 1] == '\n')
            {
                ovector[1] += 1; /* Advance by one more. */
            }
            else if (utf8)                          /* Otherwise, ensure we */
            {                                       /* advance a whole UTF-8 */
                while (ovector[1] < subject_length) /* character. */
                {
                    if ((input_str[ovector[1]] & 0xc0) != 0x80)
                    {
                        break;
                    }
                    ovector[1] += 1;
                }
            }
            continue; /* Go round the loop again */
        }

        if (rc < 0)
        {
            LOGF_ERROR("Matching error %d", rc);
            pcre2_match_data_free(match_data);
            return ERROR;
        }

        LOGF_INFO("Match succeeded again at offset %d", (int) ovector[0]);
        if (rc == 0)
        {
            LOG_ERROR("ovector was not big enough for all the captured substrings");
        }

        for (int i = 0; i < rc; i++)
        {
            PCRE2_SPTR substring_start = (PCRE2_SPTR) input_str + ovector[2 * i];
            size_t substring_length = ovector[2 * i + 1] - ovector[2 * i];
            LOGF_INFO("%2d: %.*s", i, (int) substring_length, (char *) substring_start);

            char *buffer = (char *) malloc(substring_length * sizeof(char) + 1);
            CHECK_MALLOC(buffer, "error allocating string buffer");
            strcpy(buffer, (char *) substring_start);
            LinkedList_add(out, buffer);
        }
    }

    pcre2_match_data_free(match_data);

    return OK;
}

CallmStatusCode
Tokenizer_encode(Tokenizer *tokenizer, const char *input_str, int **token_ids)
{

    LinkedList *text_parts = LinkedList_new();
    if (split_text((char *) input_str, tokenizer->ordinary_regex, text_parts) != OK)
    {
        LOG_ERROR("Error splitting text");
        return ERROR;
    }

    if (text_parts == NULL)
    {
        LOG_ERROR("IS NULL");
    }

    LOG_INFO("Printing text parts");
    LOGF_INFO("Size: %zu", LinkedList_size(text_parts));

    char *head_value = (char *) LinkedList_get_head_value(text_parts);
    LOGF_INFO("Head value: %s", head_value);

    LINKED_LIST_ITER(text_parts, item)
    {
        printf("item: %s\n", (char *) LinkedList_get_head_value(item));
    }

    LinkedList *tokens_ids;
    // LinkedIntList_print(tokens_ids);
    // LinkedIntList_free(tokens_ids);

    // Etape d'encodage sans tokens spéciaux:
    // appliquer la regexp et ittérer sur les find
    // récupérer l'id du token correspondant dans l'encoder s'il existe
    // Sinon,
    return OK;
}

void
Tokenizer_print(Tokenizer *tokenizer)
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
        printf("encoder_size=%d, ", HASH_COUNT(tokenizer->encoder));
    }
    if (tokenizer->decoder == NULL)
    {
        printf("decoder=NULL)");
    }
    else
    {
        printf("decoder_size=%d)", HASH_COUNT(tokenizer->decoder));
    }
    printf("\n");
}
