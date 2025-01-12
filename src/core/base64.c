#include "base64.h"
#include "errors.h"
#include "logging.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define PADDING_CHAR '='
#define B64_TABE_SIZE 64
#define B64_BUFF_INIT_SIZE 16

static const char b64_table[]
    = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
        'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
        's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/' };

struct b64_buffer
{
    unsigned char *buff;
    size_t len;
    size_t capacity;
};

CallmStatusCode
b64_buffer_new(struct b64_buffer **b64_buff)
{
    struct b64_buffer *b64 = malloc(sizeof(struct b64_buffer));
    CHECK_MALLOC(b64, "base64 buffer");

    b64->buff = malloc(B64_BUFF_INIT_SIZE);
    CHECK_MALLOC(b64->buff, "base64 buffer inner buffer");

    b64->len = 0;
    b64->capacity = B64_BUFF_INIT_SIZE;

    *b64_buff = b64;
    return OK;
}

void
b64_buffer_free(struct b64_buffer *b64_buff)
{
    if (b64_buff == NULL)
    {
        return;
    }
    if (b64_buff->buff != NULL)
    {
        free(b64_buff->buff);
    }
    free(b64_buff);
}

CallmStatusCode
b64_buffer_append(struct b64_buffer *b64_buff, unsigned char c)
{
    if (b64_buff->len == b64_buff->capacity)
    {
        b64_buff->capacity *= 2;
        b64_buff->buff = realloc(b64_buff->buff, b64_buff->capacity);
        CHECK_MALLOC(b64_buff->buff, "base64 buffer inner buffer realloc");
    }
    b64_buff->buff[b64_buff->len++] = c;
    return OK;
}

unsigned char *
b64_buffer_to_string(struct b64_buffer *b64_buff)
{
    unsigned char *res = malloc(b64_buff->len + 1);
    CHECK_MALLOC_RET_NULL(res, "base64 buffer to string");
    memcpy(res, b64_buff->buff, b64_buff->len);
    res[b64_buff->len] = '\0';
    return res;
}

static inline int
is_base64(unsigned char c)
{
    return (isalnum(c) || (c == '+') || (c == '/'));
}

static inline void
translate_to_b64_table_index(unsigned char *carr)
{
    int i;
    int j;
    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < B64_TABE_SIZE; j++)
        {
            if (carr[i] == b64_table[j])
            {
                carr[i] = j;
                break;
            }
        }
    }
}

static inline void
decode_single_char(unsigned char *buff, unsigned char *tmp)
{
    buff[0] = (tmp[0] << 2) + ((tmp[1] & 0x30) >> 4);
    buff[1] = ((tmp[1] & 0xf) << 4) + ((tmp[2] & 0x3c) >> 2);
    buff[2] = ((tmp[2] & 0x3) << 6) + tmp[3];
}

static inline void
fill_with_empty_char(unsigned char curr_part[4], int start_idx)
{
    for (int i = start_idx; i < 4; ++i)
    {
        curr_part[i] = '\0';
    }
}

static inline CallmStatusCode
append_to_b64_buffer(struct b64_buffer *b64_buffer, unsigned char *decoded_chars, int *curr_decoded_idx)
{
    for (*curr_decoded_idx = 0; *curr_decoded_idx < 3; (*curr_decoded_idx)++)
    {
        if (decoded_chars[*curr_decoded_idx] == '\0')
        {
            break;
        }
        if (b64_buffer_append(b64_buffer, decoded_chars[*curr_decoded_idx]) != OK)
        {
            LOG_ERROR("Error appending to base64 buffer");
            return ERROR;
        }
    }
    return OK;
}

unsigned char *
base64_decode(const char *encoded_src, int *out_len)
{
    size_t len = strlen(encoded_src);
    int curr_part_pos = 0;
    int curr_decoded_idx = 0;
    int curr_src_pos = 0;
    unsigned char current_part[4];
    unsigned char decoded_chars[3];
    struct b64_buffer *b64_buffer = NULL;

    if (b64_buffer_new(&b64_buffer) != OK)
    {
        LOG_ERROR("Error creating base64 buffer");
        return NULL;
    }

    while (len--)
    {
        if (encoded_src[curr_src_pos] == '=')
        {
            break;
        }
        if (!is_base64(encoded_src[curr_src_pos]))
        {
            return NULL;
        }

        current_part[curr_part_pos++] = encoded_src[curr_src_pos++];

        if (curr_part_pos == 4)
        {
            translate_to_b64_table_index(current_part);
            decode_single_char(decoded_chars, current_part);
            CHECK_STATUS_RET_NULL(append_to_b64_buffer(b64_buffer, decoded_chars, &curr_decoded_idx),
                                  "Error appending to base64 buffer", NULL);
            curr_part_pos = 0;
        }
    }
    if (curr_part_pos > 0)
    {
        fill_with_empty_char(current_part, curr_part_pos);
        translate_to_b64_table_index(current_part);
        decode_single_char(decoded_chars, current_part);
        CHECK_STATUS_RET_NULL(append_to_b64_buffer(b64_buffer, decoded_chars, &curr_decoded_idx),
                              "Error appending remainder to base64 buffer", NULL);
    }

    if (out_len != NULL)
    {
        *out_len = b64_buffer->len;
    }

    unsigned char *res = b64_buffer_to_string(b64_buffer);
    b64_buffer_free(b64_buffer);
    return res;
}
