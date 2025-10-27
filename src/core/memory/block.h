#ifndef CALLM_BLOCK_H
#define CALLM_BLOCK_H

#include <stddef.h>
#include "../../shared/errors.h"
#include "../../shared/utils.h"

#define NEW_STATIC_COORDS(varname, ...) \
    size_t varname[] = {__VA_ARGS__}; \

#define NEW_COORDS(varname, ...) \
    do \
    { \
        size_t tmp[VA_NARGS(__VA_ARGS__)] = {__VA_ARGS__}; \
        size_t *varname = malloc(VA_NARGS(__VA_ARGS__) * sizeof(size_t));    \
        for (size_t j = 0; j < VA_NARGS(__VA_ARGS__); j++) \
            varname[j] = tmp[j]; \
    } while (0);

typedef struct memblock MemBlock;

/**
 * Init a new memory block manager for floats.
 *
 * @param ndim
 * @param shape Not owned by the block
 * @param err
 * @return
 */
MemBlock *
MemBlock_new(size_t ndim, size_t *shape, Error **err);

/**
 * Free the memory block.
 *
 * @param b
 */
void
MemBlock_free(MemBlock *b);

size_t
MemBlock_size(MemBlock *b);

size_t*
MemBlock_shape(MemBlock *b);

size_t
MemBlock_ndim(MemBlock *b);

/**
 * Set or update a value at a given memory block location.
 *
 * @param b
 * @param val value to set
 * @param coords array of coordinates (must exist)
 * @param err
 */
void
MemBlock_set_val(MemBlock *b, float val, size_t* coords, Error **err);

/**
 * Get the value corresponding to the specified memory location.
 *
 * @param b
 * @param coords array of coordinates (must exist)
 * @param err
 * @return
 */
float
MemBlock_get_val(MemBlock *b, size_t* coords, Error **err);

#endif  // CALLM_BLOCK_H
