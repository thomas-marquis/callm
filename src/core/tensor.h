#ifndef CALLM_TENSOR_H
#define CALLM_TENSOR_H

#include "../shared/errors.h"

typedef struct tensor Tensor;

/**
 * Initialize a new empy tensor.
 *
 * @param ndim number of dimensions
 * @param shape array containing the size of each dimension (its length MUST be equal to ndim).
 *              The Tensor object doesn't have ownership over this pointer.
 *              Can be NULL for a scalar tensor.
 * @return new tensor pointer
 */
Tensor *
Tensor_new(size_t ndim, size_t *shape);

/**
 * Freed the tensor.
 *
 * @param t Tensor instance
 */
void
Tensor_free(Tensor *t);

/**
 * Return the shape of the tensor. NULL if it is a scalar
 *
 * @param t
 * @return
 */
size_t*
Tensor_shape(Tensor *t);

/**
 * Returns the total number of data contained into the tensor.
 *
 * @param t
 * @return
 */
size_t
Tensor_size(Tensor *t);

/**
 * Returns the tensor's number of dimensions.
 *
 * @param t
 * @return
 */
size_t
Tensor_ndim(Tensor *t);

/**
 * Perform a dot product between 2 tensors, allocate, create and returns the resulting tensor.
 * The 2 input tensors must have compatible shapes. Given 2 tensors A, B with n dimensions:
 * - shape(A) = (d1, d2, .., m, p)
 * - shape(B) = (d1, d2, .., q, m)
 *
 * The dot product is only performes between the 2 last dimensions. The resulting tensor C have the shape:
 * - shape(C) = (d1, d2, .., q, p)
 *
 * Note it also works for ndim=1 and ndim=0 tensors.
 *
 * @param t current Tensor instance
 * @param other Tensor to multiply with. Must have compatible dimensions
 * @return the resulting tensor or null when an error occurred.
 */
Tensor *
Tensor_dot(Tensor *t, Tensor *other);

#endif  // CALLM_TENSOR_H
