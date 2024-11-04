#ifndef SAFETENSORS_H
#define SAFETENSORS_H

#include <jansson.h>

enum Dtype
{
    F32 = 4,
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
    struct st_header_layer *layers[];
};
typedef struct st_header st_header;

int st_read_header(const int fd, st_header *h);

int st_get_layer_header(const void *map, const char *layer_name, st_header_layer *layer);

#endif
