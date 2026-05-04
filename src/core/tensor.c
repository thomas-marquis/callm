#include "tensor.h"
#include "./memory/block.h"
#include <stdlib.h>
#include <string.h>

struct tensor
{
    size_t ndim;
    size_t *shape;
    size_t size;
    MemBlock *store;
};

Tensor *
Tensor_new(size_t ndim, size_t *shape)
{
    if (ndim != 0 && shape == NULL)
    {
        LOG_ERROR("Shape must not be NULL");
        return NULL;
    }
    Tensor *t = malloc(sizeof(Tensor));
    if (t == NULL)
    {
        LOG_ERROR("Could not allocate memory for tensor");
        return NULL;
    }

    t->ndim = ndim;
    t->shape = NULL;
    if (ndim > 0)
    {
        t->shape = (size_t *) malloc(ndim * sizeof(size_t));
        memcpy(t->shape, shape, ndim);
    }

    t->size = 1;
    for (size_t i = 0; i < ndim; i++)
        t->size *= shape[i];

    Error **err = Error_new_empty_ref();
    t->store = MemBlock_new(ndim, shape, err);
    CHECK_ERROR(err, {
        LOG_ERROR((*err)->message);
        Tensor_free(t);
        return NULL;
    })

    return t;
}

void
Tensor_free(Tensor *t)
{
    if (t == NULL)
        return;
    if (t->store != NULL)
        MemBlock_free(t->store);
    if (t->shape != NULL)
        free(t->shape);
}

size_t *
Tensor_shape(Tensor *t)
{
    return t->shape;
}

size_t
Tensor_size(Tensor *t)
{
    return t->size;
}

size_t
Tensor_ndim(Tensor *t)
{
    return t->ndim;
}

//// TODO: move this in an iterator in the memory block module
// static void
// infer_coords(size_t index, size_t ndim, size_t *shape, size_t *coords)
//{
//     for (size_t dim = 0; dim < ndim; dim++)
//     {
//         size_t dim_size = shape[dim];
//         coords[dim] = index % dim_size;
//         index /= dim_size;
//     }
// }

Tensor *
Tensor_dot(Tensor *t, Tensor *other)
{
    // General verifications
    if (t == NULL || other == NULL)
    {
        LOG_ERROR("Input tensors must not be NULL");
        return NULL;
    }
    if (t->ndim != other->ndim)
    {
        LOG_ERROR("Tensor dimensions do not match");
        return NULL;
    }

    size_t ndim = t->ndim;
    Error **err = Error_new_empty_ref();

    // Special case for 0D tensors => scalar
    if (ndim == 0)
    {
        Tensor *c = Tensor_new(0, NULL);
        if (c == NULL)
            return NULL;
        MemBlock_set_val(t->store, MemBlock_get_val(t->store, NULL, err) * MemBlock_get_val(other->store, NULL, err),
                         NULL, err);
        CHECK_ERROR(err, {
            LOG_ERROR((*err)->message);
            Tensor_free(c);
            return NULL;
        })
        return c;
    }

    // Special case for 1D tensors => vector
    if (ndim == 1)
    {
        Tensor *c = Tensor_new(1, t->shape);
        if (c == NULL)
            return NULL;

        for (size_t i = 0; i < t->shape[0]; i++)
        {
            NEW_STATIC_COORDS(coords, i)
            MemBlock_set_val(t->store,
                             MemBlock_get_val(t->store, coords, err) * MemBlock_get_val(other->store, coords, err),
                             coords, err);
            CHECK_ERROR(err, {
                LOG_ERROR((*err)->message);
                Tensor_free(c);
                return NULL;
            })
        }

        return c;
    }

    // If ndim > 1, validate shape compatibility first.
    if (t->shape[ndim - 2] != other->shape[ndim - 1])
    {
        LOGF_ERROR("Tensor dimensions do not match, got t[-2]=%zu and other[-1]=%zu", t->shape[ndim - 2],
                   other->shape[ndim - 1]);
        return NULL;
    }

    for (size_t i = 0; i < ndim - 2; i++)
    {
        if (t->shape[i] != other->shape[i])
        {
            LOGF_ERROR("Tensor dimensions do not match at index %zu", i);
            return NULL;
        }
    }

    if (*err != NULL)
        Error_free(*err);

    LOG_ERROR("Tensor_dot for ndim > 1 is not implemented yet");
    return NULL;
}