#ifndef SAFETENSORS_C
#define SAFETENSORS_C

#include "safetensors.h"
#include "lib/errors.h"
#include "types.h"
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

static status_t SafetensorsLayer_parse(json_t *layer, struct SafetensorsLayer *h)
{
    json_t *dtype = GET_JSON_OBJECT(layer, "dtype", dtype);
    json_t *shape = GET_JSON_OBJECT(layer, "shape", shape);
    json_t *data_offset = GET_JSON_OBJECT(layer, "data_offsets", data_offset);

    if (!json_is_string(dtype) || !json_is_array(shape) || !json_is_array(data_offset))
    {
        printerr("error: invalid JSON data\n");
        if (json_is_string(dtype))
        {
            printerr("dtype: %s\n", json_string_value(dtype));
        }
        if (json_is_array(shape))
        {
            printerr("shape: %d\n", (int)json_array_size(shape));
        }
        if (json_is_array(data_offset))
        {
            printerr("data_offsets: %d\n", (int)json_array_size(data_offset));
        }
        return ERROR;
    }

    char *dtype_str;
    if (json_unpack(dtype, "s", &dtype_str))
    {
        printerr("error: invalid JSON data for dtype\n");
        return ERROR;
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
        return ERROR;
    }

    h->shape_size = json_array_size(shape);
    h->shape = (int *)malloc(h->shape_size * sizeof(int));
    CHECK_MALLOC(h->shape, "shape");
    for (int i = 0; i < h->shape_size; i++)
    {
        if (json_unpack(json_array_get(shape, i), "i", &h->shape[i]))
        {
            printerr("error: invalid JSON data for shape at index %d\n", i);
            return ERROR;
        }
    }

    h->data_offset = (int *)malloc(2 * sizeof(int));
    CHECK_MALLOC(h->data_offset, "data_offset");
    if (json_unpack(json_array_get(data_offset, 0), "i", &h->data_offset[0]))
    {
        printerr("error: invalid JSON data for data_offsets at index 0\n");
        return ERROR;
    }
    if (json_unpack(json_array_get(data_offset, 1), "i", &h->data_offset[1]))
    {
        printerr("error: invalid JSON data for data_offsets at index 1\n");
        return ERROR;
    }

    return OK;
}

safetensors_t *Safetensors_new(const char *file_path)
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

    // Initialize header
    safetensors_t *h = (safetensors_t *)malloc(sizeof(safetensors_t));
    CHECK_MALLOC_PANIC(h, "new safetensors header");
    h->map = NULL;
    h->map_size = filesize;
    h->header_size = header_size;
    h->layer_table = new_hash_table(DEFAULT_HASH_TABLE_SIZE);

    h->raw_content = (char *)malloc(strlen(header_content) + 1);
    CHECK_MALLOC_PANIC(h->raw_content, "safetensors header content");
    h->raw_content = strcpy(h->raw_content, header_content);

    h->json_root = NULL;
    CHECK_STATUS_PANIC(Safetensors_parse(h, header_content), "Error parsing header\n", NULL);

    h->map = map;
    Safetensors_print(h);

    return h;
}

static status_t Safetensors_print(safetensors_t *h)
{
    char *key = NULL;
    void *value = NULL;
    json_object_foreach(h->json_root, key, value)
    {
        if (strcmp(key, "__metadata__") == 0)
        {
            continue;
        }

        printf("Layer: %s\t\t", key);
        json_t *json_layer = GET_JSON_OBJECT_PANIC(h->json_root, key, json_layer);
        safetensors_layer_t *layer = (struct SafetensorsLayer *)malloc(sizeof(struct SafetensorsLayer));
        CHECK_MALLOC_PANIC(layer, key);
        CHECK_STATUS(SafetensorsLayer_parse(json_layer, layer), "Error parsing tensor header %s\n", key)

        printf("shape: [");
        for (int i = 0; i < layer->shape_size; i++)
        {
            printf("%d", layer->shape[i]);
            if (i < layer->shape_size - 1)
            {
                printf(", ");
            }
        }
        printf("]");
        printf("\n");
        SafetensorsLayer_free(layer);
    }

    return OK;
}

static status_t Safetensors_parse(safetensors_t *h, const char *header_content)
{
    json_error_t error;
    json_t *root = json_loads(header_content, 0, &error);
    if (!root)
    {
        printerr("JSON error: on line %d: %s\n", error.line, error.text);
        return ERROR;
    }

    h->json_root = root;

    return OK;
}

matrix_t *Safetensors_load_matrix(const char *tensor_name, const safetensors_t *header)
{
    // Get layer metadata
    json_t *json_layer = GET_JSON_OBJECT_PANIC(header->json_root, tensor_name, json_layer);
    safetensors_layer_t *layer = (struct SafetensorsLayer *)malloc(sizeof(struct SafetensorsLayer));
    CHECK_MALLOC_PANIC(layer, tensor_name);
    CHECK_STATUS_PANIC(SafetensorsLayer_parse(json_layer, layer), "Error parsing tensor header %s\n", tensor_name)

    // Check is a matrix
    if (layer->shape_size != 2)
    {
        printerr("Error: tensor %s is not a matrix\n", tensor_name);
        exit(1);
    }

    // Get data from map
    matrix_t *m = Matrix_new(layer->shape[0], layer->shape[1]);

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

        char *buff = (char *)malloc(nb_elements * sizeof(bf16_t));
        CHECK_MALLOC_PANIC(buff, "tmp matrix data buffer");
        memcpy(buff, (char *)header->map + start_index, nb_elements * sizeof(bf16_t));

        bf16_t *data = (bf16_t *)malloc(nb_elements * sizeof(bf16_t));
        CHECK_MALLOC_PANIC(data, "matrix bf16 data");
        memcpy(data, buff, nb_elements * sizeof(bf16_t));
        free(buff);

        // convert to float
        float *f_data = (float *)malloc(nb_elements * sizeof(float));
        CHECK_MALLOC_PANIC(f_data, "matrix float data");

        for (int i = 0; i < nb_elements; i++)
        {
            f_data[i] = bf16_to_float(data[i]);
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

status_t Safetensors_get_layer_by_name(const safetensors_t *h, const char *layer_name, safetensors_layer_t *layer)
{
    hash_table_node_t *node = hash_table_get(h->layer_table, layer_name);
    if (node == NULL)
    {

        printerr("Error: layer '%s' not found in header\n", layer_name);
        return ERROR;
    }
    layer = (safetensors_layer_t *)node->value;
    return OK;
}

status_t Safetensors_free(safetensors_t *h)
{
    hash_table_free(h->layer_table);
    free(h->raw_content);
    json_decref(h->json_root);
    if (munmap(h->map, h->map_size))
    {
        printerr("Error unmapping safetensors file\n");
        return ERROR;
    }
    free(h);
    return OK;
}

status_t SafetensorsLayer_free(safetensors_layer_t *layer)
{
    free(layer->shape);
    free(layer->data_offset);
    free(layer);
    return OK;
}

#endif // !SAFETENSORS_C
