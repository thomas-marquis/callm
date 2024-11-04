#ifndef TENSOR_H
#define TENSOR_H

struct tensor_shape
{
    int shape_size;
    int data_size;
    int *shape;
};
typedef struct tensor_shape tensor_shape;

struct tensor_f32
{
    tensor_shape *shape;
    float *data;
};
typedef struct tensor_f32 tensor_f32;

int new_tensor_from_map(void *map, tensor_f32 *t, int begin_pos, int end_pos);

int new_empty_tensor(int *shape, int shape_size, tensor_f32 *t);

int tensor_mult(tensor_f32 *a, tensor_f32 *b, tensor_f32 *c);

#endif
