#ifndef SAFETENSORS_C
#define SAFETENSORS_C

#include "safetensors.h"
#include "types.h"
#include "utils.h"
#include <fcntl.h>
#include <jansson.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define HEADER_SIZE_PART_SIZE sizeof(uint64_t)
#define GET_JSON_OBJECT(root, key, obj)                                                                                \
    json_object_get(root, key);                                                                                        \
    if (obj == NULL)                                                                                                   \
    {                                                                                                                  \
        printerr("Error getting %s key from JSON\n", key);                                                             \
        return 1;                                                                                                      \
    }
#define GET_JSON_OBJECT_PANIC(root, key, obj)                                                                          \
    json_object_get(root, key);                                                                                        \
    if (obj == NULL)                                                                                                   \
    {                                                                                                                  \
        printerr("Error getting %s key from JSON\n", key);                                                             \
        exit(1);                                                                                                       \
    }
#define DEFAULT_HASH_TABLE_SIZE 16

static int st_parse_header_layer(json_t *layer, struct st_header_layer *h)
{
    json_t *dtype = GET_JSON_OBJECT(layer, "dtype", dtype);
    json_t *shape = GET_JSON_OBJECT(layer, "shape", shape);
    json_t *data_offset = GET_JSON_OBJECT(layer, "data_offsets", data_offset);

    if (!json_is_string(dtype) || !json_is_array(shape) || !json_is_array(data_offset))
    {
        printerr("error: invalid JSON data\n");
        printerr("dtype: %s\n", json_string_value(dtype));
        printerr("shape: %d\n", (int)json_array_size(shape));
        printerr("data_offsets: %d\n", (int)json_array_size(data_offset));
        return 1;
    }

    char *dtype_str;
    if (json_unpack(dtype, "s", &dtype_str))
    {
        printerr("error: invalid JSON data for dtype\n");
        return 1;
    }
    if (strcmp(dtype_str, "F32") == 0)
    {
        h->dtype = F32;
    }
    else if (strcmp(dtype_str, "BF16") == 0)
    {
        h->dtype = BF16;
    }
    else
    {
        printerr("error: invalid dtype '%s'\n", dtype_str);
        return 1;
    }

    h->shape_size = json_array_size(shape);
    h->shape = (int *)malloc(h->shape_size * sizeof(int));
    CHECK_MALLOC(h->shape, "shape");
    for (int i = 0; i < h->shape_size; i++)
    {
        if (json_unpack(json_array_get(shape, i), "i", &h->shape[i]))
        {
            printerr("error: invalid JSON data for shape at index %d\n", i);
            return 1;
        }
    }

    h->data_offset = (int *)malloc(2 * sizeof(int));
    CHECK_MALLOC(h->data_offset, "data_offset");
    if (json_unpack(json_array_get(data_offset, 0), "i", &h->data_offset[0]))
    {
        printerr("error: invalid JSON data for data_offsets at index 0\n");
        return 1;
    }
    if (json_unpack(json_array_get(data_offset, 1), "i", &h->data_offset[1]))
    {
        printerr("error: invalid JSON data for data_offsets at index 1\n");
        return 1;
    }

    return 0;
}

st_header *new_st_header(const char *file_path)
{
    int fd = open(file_path, O_RDONLY);
    if (fd == -1)
    {
        perror("Error opening file");
        exit(1);
    }

    // Get the file size
    struct stat st;
    if (fstat(fd, &st) == -1)
    {
        perror("fstat");
        close(fd);
        exit(1);
    }
    size_t filesize = st.st_size;

    // read header size
    void *map = mmap(NULL, filesize, PROT_READ, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED)
    {
        printerr("Error mapping file");
        close(fd);
        exit(1);
    }

    uint64_t header_size;
    memcpy(&header_size, map, HEADER_SIZE_PART_SIZE);
    printf("Header size: %lu\n", header_size);
    if (HEADER_SIZE_PART_SIZE + header_size > filesize)
    {
        fprintf(stderr, "Error: header size is larger than file size\n");
        munmap(map, filesize);
        close(fd);
        exit(1);
    }

    // read header
    char *header_content = (char *)malloc(header_size + 1);
    CHECK_MALLOC_PANIC(header_content, "header content");
    memcpy(header_content, (char *)map + HEADER_SIZE_PART_SIZE, header_size);
    header_content[header_size] = '\0';
    printf("Header content: %s\n", header_content);

    // Initialize header
    st_header *h = (st_header *)malloc(sizeof(st_header));
    CHECK_MALLOC_PANIC(h, "new safetensors header");
    h->map = NULL;
    h->map_size = filesize;
    h->header_size = header_size;
    h->layer_table = new_hash_table(DEFAULT_HASH_TABLE_SIZE);

    h->raw_content = (char *)malloc(strlen(header_content) + 1);
    CHECK_MALLOC_PANIC(h->raw_content, "safetensors header content");
    h->raw_content = strcpy(h->raw_content, header_content);

    h->json_root = NULL;
    if (st_header_parse(h, header_content))
    {
        printerr("Error parsing header\n");
        exit(1);
    }

    h->map = map;

    return h;
}

static int st_header_parse(st_header *h, const char *header_content)
{
    json_error_t error;
    json_t *root = json_loads(header_content, 0, &error);
    if (!root)
    {
        printerr("JSON error: on line %d: %s\n", error.line, error.text);
        return 1;
    }

    h->json_root = root;

    return 0;
}

matrix *st_load_matrix(const char *tensor_name, const st_header *header)
{
    // Get layer metadata
    json_t *json_layer = GET_JSON_OBJECT_PANIC(header->json_root, tensor_name, json_layer);
    st_header_layer *layer = (struct st_header_layer *)malloc(sizeof(struct st_header_layer));
    CHECK_MALLOC_PANIC(layer, tensor_name);
    if (st_parse_header_layer(json_layer, layer))
    {
        printerr("Error parsing tensor header %s\n", tensor_name);
        exit(1);
    }

    // Check is a matrix
    if (layer->shape_size != 2)
    {
        printerr("Error: tensor %s is not a matrix\n", tensor_name);
        exit(1);
    }

    // Get data from map
    matrix *m = new_matrix(layer->shape[0], layer->shape[1]);

    int nb_elements = layer->shape[0] * layer->shape[1];
    int start_index = HEADER_SIZE_PART_SIZE + header->header_size + layer->data_offset[0];

    if (layer->dtype == F32)
    {
        printf("Loading float32 matrix\n");

        char *buff = (char *)malloc(nb_elements * sizeof(float));
        CHECK_MALLOC_PANIC(buff, "tmp matrix data buffer");
        memcpy(buff, (char *)header->map + start_index, nb_elements * sizeof(float));

        float *data = (float *)malloc(nb_elements * sizeof(float));
        CHECK_MALLOC_PANIC(data, "matrix float data");
        memcpy(data, buff, nb_elements * sizeof(float));
        free(buff);

        m->data = data;
    }
    else if (layer->dtype == BF16)
    {
        printf("Loading bf16 matrix\n");

        char *buff = (char *)malloc(nb_elements * sizeof(bf16));
        CHECK_MALLOC_PANIC(buff, "tmp matrix data buffer");
        memcpy(buff, (char *)header->map + start_index, nb_elements * sizeof(bf16));

        bf16 *data = (bf16 *)malloc(nb_elements * sizeof(bf16));
        CHECK_MALLOC_PANIC(data, "matrix bf16 data");
        memcpy(data, buff, nb_elements * sizeof(bf16));
        free(buff);

        // convert to float
        float *f_data = (float *)malloc(nb_elements * sizeof(float));
        CHECK_MALLOC_PANIC(f_data, "matrix float data");

        for (int i = 0; i < nb_elements; i++)
        {
            // printf("bf16: %d\n", data[i]); // TODO debug
            f_data[i] = bf16_to_float(data[i]);
            // printf("float: %f\n", f_data[i]); // TODO debug
        }

        m->data = f_data;
        free(data);
    }
    else
    {
        printerr("Error: unsupported dtype %d\n", layer->dtype);
        exit(1);
    }

    return m;
}

int st_get_layer_header_by_name(const st_header *header, const char *layer_name, st_header_layer *layer)
{
    HashTableNode *node = hash_table_get(header->layer_table, layer_name);
    if (node == NULL)
    {

        printerr("Error: layer '%s' not found in header\n", layer_name);
        return 1;
    }
    layer = (st_header_layer *)node->value;
    return 0;
}

int st_header_free(st_header *h)
{
    hash_table_free(h->layer_table);
    free(h->raw_content);
    json_decref(h->json_root);
    if (munmap(h->map, h->map_size))
    {
        printerr("Error unmapping safetensors file\n");
        return 1;
    }
    free(h);
    return 0;
}

#endif
