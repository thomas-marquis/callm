#ifndef SAFETENSORS_C
#define SAFETENSORS_C

#include "safetensors.h"
#include "utils.h"
#include <jansson.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define HEADER_SIZE_PART_SIZE 8
#define GET_JSON_OBJECT(root, key, obj)                                                                                \
    json_object_get(root, key);                                                                                        \
    if (obj == NULL)                                                                                                   \
    {                                                                                                                  \
        printerr("Error getting %s key from JSON\n", key);                                                             \
        return 1;                                                                                                      \
    }

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

static int st_parse_header(const char *header_content, st_header *h)
{
    json_error_t error;
    json_t *root = json_loads(header_content, 0, &error);
    if (!root)
    {
        printerr("JSON error: on line %d: %s\n", error.line, error.text);
        return 1;
    }

    json_t *hidden_bias = GET_JSON_OBJECT(root, "hidden.bias", hidden_bias);
    h->hidden_bias = (struct st_header_layer *)malloc(sizeof(struct st_header_layer));
    CHECK_MALLOC(h->hidden_bias, "hidden bias");
    if (st_parse_header_layer(hidden_bias, h->hidden_bias))
    {
        printerr("Error parsing hidden bias\n");
        return 1;
    }

    json_t *hidden_weights = GET_JSON_OBJECT(root, "hidden.weight", hidden_weights);
    h->hidden_weights = (struct st_header_layer *)malloc(sizeof(struct st_header_layer));
    CHECK_MALLOC(h->hidden_weights, "hidden weights");
    if (st_parse_header_layer(hidden_weights, h->hidden_weights))
    {
        printerr("Error parsing hidden weights\n");
        return 1;
    }

    json_t *output_bias = GET_JSON_OBJECT(root, "output.bias", output_bias);
    h->output_bias = (struct st_header_layer *)malloc(sizeof(struct st_header_layer));
    CHECK_MALLOC(h->output_bias, "output bias");
    if (st_parse_header_layer(output_bias, h->output_bias))
    {
        printerr("Error parsing output bias\n");
        return 1;
    }

    json_t *output_weights = GET_JSON_OBJECT(root, "output.weight", output_weights);
    h->output_weights = (struct st_header_layer *)malloc(sizeof(struct st_header_layer));
    CHECK_MALLOC(h->output_weights, "output weights");
    if (st_parse_header_layer(output_weights, h->output_weights))
    {
        printerr("Error parsing output weights\n");
        return 1;
    }

    json_decref(root);

    return 0;
}

int st_read_header(const int fd, st_header *h)
{
    // read header size
    uint64_t *size_map = mmap(NULL, HEADER_SIZE_PART_SIZE, PROT_READ, MAP_SHARED, fd, 0);
    if (size_map == MAP_FAILED)
    {
        printerr("Error mapping file");
        close(fd);
        return 1;
    }

    uint64_t header_size = *size_map;
    printf("Header size: %lu\n", header_size);

    if (munmap(size_map, HEADER_SIZE_PART_SIZE) == -1)
    {
        printerr("Error unmapping file");
        return 1;
    }

    // read header
    char *header_map = mmap(NULL, header_size, PROT_READ, MAP_SHARED, fd, 0);
    if (header_map == MAP_FAILED)
    {
        printerr("Error mapping file");
        close(fd);
        return 1;
    }

    char *header_content = (char *)malloc(header_size + 1);
    CHECK_MALLOC(header_content, "header content");

    printf("Header content: ");
    for (int i = 0; i < header_size; i++)
    {
        header_content[i] = header_map[i + HEADER_SIZE_PART_SIZE];
    }
    header_content[header_size] = '\0';
    printf("Header content: %s\n", header_content);

    if (munmap(header_map, header_size) == -1)
    {
        printerr("Error unmapping file");
        return 1;
    }

    // parse header content
    if (st_parse_header(header_content, h))
    {
        return 1;
    }
    free(header_content);
    return 0;
}

#endif
