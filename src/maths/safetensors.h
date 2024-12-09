#ifndef SAFETENSORS_H
#define SAFETENSORS_H

#include "../utils/errors.h"
#include "../utils/hash_table.h"
#include "./bf16.h"
#include "matrix.h"
#include <jansson.h>

enum Dtype
{
  F32 = 4,
  BF16 = 2,
};

typedef struct
{
  enum Dtype dtype;
  int *shape;
  int shape_size;
  int *data_offset;
} SafetensorsLayer;

typedef struct
{
  char *raw_content;
  HashTable *layer_table;
  json_t *json_root;
  void *map;
  int map_size;
  uint64_t header_size;
} Safetensors;

Matrix *Safetensors_load_matrix (const char *tensor_name,
                                 const Safetensors *header);

CallmStatusCode Safetensors_get_layer_by_name (const Safetensors *header,
                                               const char *layer_name,
                                               SafetensorsLayer *layer);

static CallmStatusCode Safetensors_parse (Safetensors *h,
                                          const char *header_content);

/**
 * @brief Prints the content of an st_header object.
 */
CallmStatusCode Safetensors_print (Safetensors *h);

/**
 * @brief Creates a new st_header object by reading and parsing the header from
 * a file.
 *
 * This function opens a file, maps it into memory, reads the header size and
 * content, and initializes an `st_header` object with the parsed header
 * information.
 *
 * @param file_path The path to the file to be read.
 * @return A pointer to the newly created `st_header` object.
 */
Safetensors *Safetensors_new (const char *file_path);

/**
 * @brief Frees the memory allocated for an st_header object.
 *
 * This function releases all resources associated with an `st_header` object,
 * including hash tables, raw content, and JSON objects.
 *
 * @param h A pointer to the `st_header` object to be freed.
 * @return CallmStatusCode OK if the header was successfully freed, ERROR
 * otherwise.
 */
CallmStatusCode Safetensors_free (Safetensors *h);

CallmStatusCode SafetensorsLayer_free (SafetensorsLayer *layer);

#endif
