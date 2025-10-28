#include "block.h"
#include <stdlib.h>
#include "../../shared/errors.h"

#include <string.h>

static unsigned int
get_index_from_coordinates(MemBlock *b, size_t *coords, Error **err);

struct memblock_iterator
{
    size_t current_iteration;
    size_t max_iterations;
    size_t *current_coords;

    MemBlock *block;
    size_t *mask;
};

struct memblock
{
    float *data;
    size_t size;
    size_t *shape;
    size_t ndim;
    size_t *index_scale;

    unsigned int iterator_ref_cnt;
    MemBlockIterator **iterators;
    unsigned int iterators_size;
};

int
MemBlockIterator_next(MemBlockIterator *i)
{
    if (i->current_iteration >= i->max_iterations)
        return -1;

    for (size_t d = i->block->ndim; d != 0; d--)
    {
        if (i->mask[d-1] == 0)
            continue;
        i->current_coords[d-1]++;
        if (i->current_coords[d-1] < i->block->shape[d-1])
        {
            i->current_iteration++;
            return 0;
        }
        i->current_coords[d-1] = 0;
    }

    return -1;
}

void
MemBlockIterator_set_val(MemBlockIterator *i, float val, size_t* coords, Error **err)
{
    size_t coord_idx = 0;
    for (size_t d = i->block->ndim; d != 0; d--)
    {
        if (i->mask[d] == 0)
        {
            i->current_coords[d] = coords[coord_idx];
            coord_idx++;
        }
    }

    unsigned int index = get_index_from_coordinates(i->block, i->current_coords, err);
    if (*err != NULL)
        return;

    i->block->data[index] = val;
}

float
MemBlockIterator_get_val(MemBlockIterator *i, size_t* coords, Error **err)
{
    size_t coord_idx = 0;
    for (size_t d = i->block->ndim; d != 0; d--)
    {
        if (i->mask[d] == 0)
        {
            i->current_coords[d] = coords[coord_idx];
            coord_idx++;
        }
    }

    unsigned int index = get_index_from_coordinates(i->block, i->current_coords, err);
    if (*err != NULL)
        return 0;

    return i->block->data[index];
}

MemBlock *
MemBlock_new(size_t ndim, size_t *shape, Error **err)
{
    MemBlock *b = malloc(sizeof(MemBlock));
    if (b == NULL)
    {
        Error_with_message(err, "Could not allocate memory for a new memory block");
        return NULL;
    }

    b->iterator_ref_cnt = 0;
    b->iterators_size = 0;
    b->iterators = NULL;

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
    if (b->iterators != NULL && b->iterator_ref_cnt > 0)
        for (size_t i = 0; i < b->iterator_ref_cnt; i++)
            MemBlock_iterator_free(b, b->iterators[i]);
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

static unsigned int
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

MemBlockIterator *
MemBlock_iterate(MemBlock *b, size_t *shape_mask, Error **err)
{
    for (size_t i = 0; i < b->ndim; i++)
    {
        if (shape_mask[i] != 0 || shape_mask[i] != 1)
        {
            Error_with_message(err, "Shape mask must be 0 or 1");
            return NULL;
        }
    }

    // TODO: remove this when the behavior will be generalized
    if (b->ndim <= 2)
    {
        Error_with_message(err, "For now, iterator doesn't supported blocks with ndim <= 3");
        return NULL;
    }
    for (size_t i = 0; i < b->ndim - 2; i++)
    {
        if (shape_mask[i] != 1)
        {
            Error_with_message(err, "For now, the only supported mask is {1, 1, ..., 1, 0, 0}");
            return NULL;
        }
    }
    if (shape_mask[b->ndim - 2] != 1 || shape_mask[b->ndim - 1] != 0)
    {
        Error_with_message(err, "For now, the only supported mask is {1, 1, ..., 1, 0, 0}");
        return NULL;
    }

    MemBlockIterator *i = malloc(sizeof(MemBlockIterator));
    b->iterator_ref_cnt++;
    if (b->iterators == NULL)
    {
        b->iterators = malloc(sizeof(MemBlockIterator *));
        if (b->iterators == NULL)
        {
            b->iterator_ref_cnt--;
            Error_with_message(err, "Could not allocate memory for iterators");
            return NULL;
        }
        b->iterators_size = 1;
    }
    else if (b->iterator_ref_cnt > b->iterators_size)
    {
        b->iterators = realloc(b->iterators, sizeof(MemBlockIterator *) * b->iterator_ref_cnt);
        if (b->iterators == NULL)
        {
            b->iterator_ref_cnt--;
            Error_with_message(err, "Could not reallocate memory for iterators");
            return NULL;
        }
    }

    i->block = b;

    i->mask = (size_t *) malloc(b->ndim * sizeof(size_t));
    memcpy(i->mask, shape_mask, b->ndim);

    i->current_coords = (size_t *) malloc(b->ndim * sizeof(size_t));
    for (size_t j = 0; j < b->ndim; j++)
        i->current_coords[j] = 0;

    i->max_iterations = 1;
    for (size_t j = 0; j < b->ndim; j++)
    {
        if (shape_mask[j] == 1)
            i->max_iterations *= b->shape[j];
    }

    b->iterators[b->iterator_ref_cnt - 1] = i;

    return i;
}

void
MemBlock_iterator_free(MemBlock *b, MemBlockIterator *i)
{
    if (b == NULL || i == NULL)
        return;

    if (b->iterator_ref_cnt > 0 && b->iterators != NULL)
    {
        MemBlockIterator *it;
        for (unsigned int j = 0; j < b->iterator_ref_cnt; j++)
        {
            it = b->iterators[j];
            if (it == i)
            {
                if (it->mask != NULL)
                    free(it->mask);
                if (it->current_coords != NULL)
                    free(it->current_coords);
                free(it);

                b->iterators[j] = b->iterators[b->iterator_ref_cnt - 1];
                b->iterator_ref_cnt--;
            }
        }
    }

    if (b->iterator_ref_cnt == 0 && b->iterators != NULL)
    {
        free(b->iterators);
        b->iterators = NULL;
        b->iterators_size = 0;
    }

    free(i);
}
