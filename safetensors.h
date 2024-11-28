#ifndef SAFETENSORS_H
#define SAFETENSORS_H

#include "lib/errors.h"
#include "lib/hash_table.h"
#include "matrix.h"
#include "types.h"
#include <jansson.h>

enum Dtype
{
    F32 = 4,
    BF16 = 2,
};

struct SafetensorsLayer
{
    enum Dtype dtype;
    int *shape;
    int shape_size;
    int *data_offset;
};
typedef struct SafetensorsLayer safetensors_layer_t;

struct Safetensors
{
    char *raw_content;
    hash_table_t *layer_table;
    json_t *json_root;
    void *map;
    int map_size;
    uint64_t header_size;
};
typedef struct Safetensors safetensors_t;

matrix_t *Safetensors_load_matrix(const char *tensor_name, const safetensors_t *header);

status_t Safetensors_get_layer_by_name(const safetensors_t *header, const char *layer_name, safetensors_layer_t *layer);

static status_t Safetensors_parse(safetensors_t *h, const char *header_content);

/**
 * @brief Prints the content of an st_header object.
 */
status_t Safetensors_print(safetensors_t *h);

/**
 * @brief Creates a new st_header object by reading and parsing the header from a file.
 *
 * This function opens a file, maps it into memory, reads the header size and content,
 * and initializes an `st_header` object with the parsed header information.
 *
 * @param file_path The path to the file to be read.
 * @return A pointer to the newly created `st_header` object.
 */
safetensors_t *Safetensors_new(const char *file_path);

/**
 * @brief Frees the memory allocated for an st_header object.
 *
 * This function releases all resources associated with an `st_header` object,
 * including hash tables, raw content, and JSON objects.
 *
 * @param h A pointer to the `st_header` object to be freed.
 * @return status_t OK if the header was successfully freed, ERROR otherwise.
 */
status_t Safetensors_free(safetensors_t *h);

status_t SafetensorsLayer_free(safetensors_layer_t *layer);

#endif
