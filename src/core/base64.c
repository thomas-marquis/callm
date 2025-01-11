#include "base64.h"
#include <ctype.h>
#include <string.h>

#define PADDING_CHAR '='
#define B64_TABE_SIZE 64

static const char b64_table[]
    = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
        'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
        's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/' };

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

unsigned char *
base64_decode(const char *encoded_src, int src_len, int *out_len)
{
    return NULL;
}
