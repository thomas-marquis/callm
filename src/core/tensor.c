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

size_t*
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
//static void
//infer_coords(size_t index, size_t ndim, size_t *shape, size_t *coords)
//{
//    for (size_t dim = 0; dim < ndim; dim++)
//    {
//        size_t dim_size = shape[dim];
//        coords[dim] = index % dim_size;
//        index /= dim_size;
//    }
//}

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
        MemBlock_set_val(
            t->store,
            MemBlock_get_val(t->store, NULL, err) * MemBlock_get_val(other->store, NULL, err),
            NULL,
            err
        );
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
            MemBlock_set_val(
                t->store,
                MemBlock_get_val(t->store, coords, err) * MemBlock_get_val(other->store, coords, err),
                coords,
                err
            );
            CHECK_ERROR(err, {
                LOG_ERROR((*err)->message);
                Tensor_free(c);
                return NULL;
            })
        }

        return c;
    }

    // TODO: special case for 3D tensor => matrix
    // TODO: special case for 4D tensor

    // If ndim > 1, check shape
    if (t->shape[ndim - 2] != other->shape[ndim - 1])
    {
        LOGF_ERROR("Tensor dimensions do not match, got t[-2]=%d and other[-1]", t->shape[ndim - 2], other->shape[ndim - 1]);
        return NULL;
    }

    for (size_t i = 0; i < ndim - 2; i++)
    {
        if (t->shape[i] != other->shape[i])
        {
            LOGF_ERROR("Tensor dimensions do not match at index %d", i);
            return NULL;
        }
    }

    // init the new C tensor
    size_t *c_shape = malloc(ndim * sizeof(size_t));
    if (c_shape == NULL)
    {
        LOG_ERROR("Could not allocate memory for shape");
        return NULL;
    }
    memcpy(c_shape, t->shape, ndim);
    c_shape[ndim - 1] = other->shape[ndim - 1];
    Tensor *c = Tensor_new(ndim, c_shape);
    if (c == NULL)
        return NULL;
    free(c_shape);

    float acc;
    size_t *t_coords = malloc(ndim - 1 * sizeof(size_t));
    size_t *other_coords = malloc(ndim - 1 * sizeof(size_t));

    for (size_t i = 0; i < t->shape[ndim - 2]; i++)
    {
        for (size_t j = 0; j < other->shape[ndim - 1]; j++)
        {
            t_coords[ndim - 1] = i;
            other_coords[ndim - 2] = j;

            size_t n_common_coords = 1;
            for (size_t dim_idx = 0; dim_idx < ndim - 2; dim_idx++)
            {
                n_common_coords *= t->shape[dim_idx];
            }

            for (size_t coord_idx = 0; coord_idx < n_common_coords; coord_idx++)
            {
                infer_coords(coord_idx, ndim - 2, t->shape, t_coords);
                for (size_t c = 0; c < ndim - 2; c++)
                {
                    acc += t->data[t_coords[c]] * other->data[other_coords[c]];  // TODO: use a memory block here
                }
            }


//            for (size_t dim_idx = 0; dim_idx < ndim - 2; dim_idx++)
//            {
//                // t_coords[dim_idx] = dim_idx;
//                // other_coords[dim_idx] = dim_idx;
//
//                size_t dim_size = t->shape[dim_idx];
//                for (size_t k = 0; k < dim_size; k++)
//                {
//                    t_coords[dim_idx] = k;
//                    other_coords[dim_idx] = k;
//
//                }
//                // c->data[i * other->shape[other->ndim - 1] + j] += t->data[i * t->shape[t->ndim - 1] + k] * other->data[k * other->shape[other->ndim - 1] + j];
//            }
        }
    }

    free(t_coords);
    free(other_coords);
    if (*err != NULL)
        Error_free(*err);

    return c;
}