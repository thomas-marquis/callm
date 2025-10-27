#include "block.h"
#include <stdlib.h>
#include "../../shared/errors.h"

#include <string.h>

struct memblock
{
    float *data;
    size_t size;
    size_t *shape;
    size_t ndim;
    size_t *index_scale;
};

MemBlock *
MemBlock_new(size_t ndim, size_t *shape, Error **err)
{
    MemBlock *b = malloc(sizeof(MemBlock));
    if (b == NULL)
    {
        Error_with_message(err, "Could not allocate memory for a new memory block");
        return NULL;
    }

    b->ndim = ndim;
    b->shape = NULL;
    if (ndim > 0)
    {
        b->shape = (size_t *) malloc(ndim * sizeof(size_t));
        memcpy(b->shape, shape, ndim);
    }

    b->size = 1;
    for (size_t i = 0; i < ndim; i++)
        b->size *= shape[i];

    b->data = (float *) malloc(b->size * sizeof(float));
    if (b->data == NULL)
    {
        Error_with_message(err, "Could not allocate memory for tensor data");
        MemBlock_free(b);
        return NULL;
    }

    b->index_scale = (size_t *) malloc(ndim * sizeof(size_t));
    for (size_t i = 0; i < ndim; i++)
    {
        b->index_scale[i] = 1;
        for (size_t j = i + 1; j < ndim; j++)
        {
            b->index_scale[i] *= shape[j];
        }
    }

    return b;
}

void
MemBlock_free(MemBlock *b)
{
    if (b == NULL)
        return;
    if (b->shape != NULL)
        free(b->shape);
    if (b->data != NULL)
        free(b->data);
    if (b->index_scale != NULL)
        free(b->index_scale);
    free(b);
}

size_t
MemBlock_size(MemBlock *b)
{
    return b->size;
}

size_t*
MemBlock_shape(MemBlock *b)
{
    return b->shape;
}

size_t
MemBlock_ndim(MemBlock *b)
{
    return b->ndim;
}

unsigned int
get_index_from_coordinates(MemBlock *b, size_t *coords, Error **err)
{
    unsigned int index = 0;
    for (int i = 0; i < b->ndim; i++)
    {
        size_t c = coords[i];
        if (c >= b->shape[i])
        {
            Error_with_message(err, "Index out of bounds");
            return 0;
        }
        index += b->index_scale[i] * c;
    }
    return index;
}

void
MemBlock_set_val(MemBlock *b, float val, size_t* coords, Error **err)
{
    unsigned int index = get_index_from_coordinates(b, coords, err);
    if (*err != NULL)
        return;

    b->data[index] = val;
}

float
MemBlock_get_val(MemBlock *b, size_t* coords, Error **err)
{
    unsigned int index = get_index_from_coordinates(b, coords, err);
    if (*err != NULL)
        return 0;

    return b->data[index];
}
