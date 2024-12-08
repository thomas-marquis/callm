#ifndef LINEAR_MAP_H
#define LINEAR_MAP_H

#include "./errors.h"

typedef struct
{
    char *key;
    void *value;
    int value_size;
} LinearMapItem;

typedef struct
{
    LinearMapItem *items;
    int size;
    int capacity;
} LinearMap;

LinearMap *LinearMap_new();

void LinearMap_free(LinearMap *map);

/**
 * Insert a value into a linear map.
 *
 * @param map The linear map to insert the value into.
 * @param key The key to associate with the value.
 * @param value The value pointer to insert.
 * @param value_size The size (bytes) of the value.
 * @return status_t OK if the insertion was successful, ERROR otherwise.
 */
CallmStatusCode LinearMap_insert(LinearMap *map, char *key, void *value, int value_size);

/**
 * Get a value from a linear map.
 *
 * @param map The linear map to get the value from.
 * @param key The key of the value to get.
 * @return The value associated with the key, or NULL if the key does not exist.
 */
void *LinearMap_get(LinearMap *map, char *key);

#endif // !#ifndef LINEAR_MAP_H
