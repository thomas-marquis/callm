#ifndef SAFETENSORS_H
#define SAFETENSORS_H

#include "matrix.h"
#include "types.h"
#include <jansson.h>

enum Dtype
{
    F32 = 4,
    BF16 = 2,
};

struct st_header_layer
{
    enum Dtype dtype;
    int *shape;
    int shape_size;
    int *data_offset;
};
typedef struct st_header_layer st_header_layer;

struct st_header
{
    char *raw_content;
    HashTable *layer_table;
    json_t *json_root;
    void *map;
    int map_size;
    uint64_t header_size;
};
typedef struct st_header st_header;

int st_read_header(const int fd, st_header *h);

int st_get_layer_header(const void *map, const char *layer_name, st_header_layer *layer);

matrix *st_load_matrix(const char *tensor_name, const st_header *header);

int st_get_layer_header_by_name(const st_header *header, const char *layer_name, st_header_layer *layer);

static int st_header_parse(st_header *h, const char *header_content);

/**
 * @brief Creates a new st_header object by reading and parsing the header from a file.
 *
 * This function opens a file, maps it into memory, reads the header size and content,
 * and initializes an `st_header` object with the parsed header information.
 *
 * @param file_path The path to the file to be read.
 * @return A pointer to the newly created `st_header` object.
 */
st_header *new_st_header(const char *file_path);

/**
 * @brief Frees the memory allocated for an st_header object.
 *
 * This function releases all resources associated with an `st_header` object,
 * including hash tables, raw content, and JSON objects.
 *
 * @param h A pointer to the `st_header` object to be freed.
 * @return Returns 0 on success.
 */
int st_header_free(st_header *h);

#endif
