#ifndef TENSOR_C
#define TENSOR_C

#include "tensor.h"
#include "utils.h"
#include <stdlib.h>

int new_tensor_from_map(void *map, tensor_f32 *t, int begin_pos, int end_pos)
{
    for (int i = 0; i < t->shape->data_size * 2; i++)
    {
        t->data[i] = ((float *)map)[i + begin_pos];
    }
    return 0;
}

int new_empty_tensor(int *shape, int shape_size, tensor_f32 *t)
{
    t->shape = (tensor_shape *)malloc(sizeof(tensor_shape));
    CHECK_MALLOC(t->shape, "tensor shape");

    t->shape->shape_size = shape_size;

    t->shape->shape = (int *)malloc(shape_size * sizeof(int));
    CHECK_MALLOC(t->shape->shape, "tensor shape size");
    int data_size = 1;
    for (int i = 0; i < shape_size; i++)
    {
        t->shape->shape[i] = shape[i];
        data_size *= shape[i];
    }

    t->data = (float *)malloc(data_size * sizeof(float));
    CHECK_MALLOC(t->data, "tensor data");

    return 0;
}

int tensor_mult(tensor_f32 *a, tensor_f32 *b, tensor_f32 *c)
{
    if (a->shape->shape_size != 2 || b->shape->shape_size != 2 || c->shape->shape_size != 2)
    {
        printerr("Error: invalid shape size for tensor multiplication: only 2x2 tensors supported\n");
        return 1;
    }
    if (a->shape->shape[1] != b->shape->shape[0] || a->shape->shape[0] != c->shape->shape[0] ||
        b->shape->shape[1] != c->shape->shape[1])
    {
        printerr("Error: invalid shape for tensor multiplication: %dx%d * %dx%d = %dx%d\n", a->shape->shape[0],
                 a->shape->shape[1], b->shape->shape[0], b->shape->shape[1], c->shape->shape[0], c->shape->shape[1]);
        return 1;
    }

    for (int i = 0; i < c->shape->shape[0]; i++)
    {
        for (int j = 0; j < c->shape->shape[1]; j++)
        {
            c->data[i * c->shape->shape[1] + j] = 0;
            for (int k = 0; k < a->shape->shape[1]; k++)
            {
                c->data[i * c->shape->shape[1] + j] +=
                    a->data[i * a->shape->shape[1] + k] * b->data[k * b->shape->shape[1] + j];
            }
        }
    }
    return 0;
}

#endif
