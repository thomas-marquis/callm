#ifndef LINEAR_MAP_C
#define LINEAR_MAP_C

#include "linear_map.h"
#include "logging.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define LINEAR_MAP_DEFAULT_CAPACITY 16

static linear_map_item_t *LineraMapItem_new(char *key, void *value, int value_size)
{
    linear_map_item_t *item = (linear_map_item_t *)malloc(sizeof(linear_map_item_t));
    (*item).key = key;
    (*item).value = value;
    (*item).value_size = value_size;
    return item;
}

/**
 * Resize the linear map to accommodate more items if necessary.
 *
 * @param map The linear map to resize.
 * @return status_t OK if the resize was successful, ERROR otherwise.
 */
static status_t LinearMap_resize(linear_map_t *map)
{
    if (map->size < map->capacity)
    {
        return OK;
    }

    int new_capacity = map->capacity * 2;
    linear_map_item_t *new_items = (linear_map_item_t *)malloc(sizeof(linear_map_item_t) * new_capacity);
    CHECK_MALLOC(new_items, "new linear map items");

    for (int i = 0; i < map->size; i++)
    {
        new_items[i] = map->items[i];
    }
    free(map->items);
    map->items = new_items;
    map->capacity = new_capacity;
    return OK;
}

linear_map_t *LinearMap_new()
{
    linear_map_t *map = (linear_map_t *)malloc(sizeof(linear_map_t));
    (*map).size = 0;
    (*map).capacity = LINEAR_MAP_DEFAULT_CAPACITY;
    (*map).items = (linear_map_item_t *)malloc(sizeof(linear_map_item_t) * LINEAR_MAP_DEFAULT_CAPACITY);
    return map;
}

void LinearMap_free(linear_map_t *map)
{
    if (map == NULL)
    {
        return;
    }
    if ((*map).items != NULL)
    {
        for (int i = 0; i < (*map).size; i++)
        {
            free((*map).items[i].key);
            free((*map).items[i].value);
        }
        free((*map).items);
    }
    free(map);
}

status_t LinearMap_insert(linear_map_t *map, char *key, void *value, int value_size)
{
    assert(map != NULL && "map must not be NULL to insert item into");

    if (LinearMap_resize(map) != OK)
    {
        log_error("Failed to insert item into linear map: resize failed");
        return ERROR;
    }

    linear_map_item_t *item = LineraMapItem_new(key, value, value_size);
    if (item == NULL)
    {
        log_error("Failed to insert item into linear map: failed to create new item");
        return ERROR;
    }

    map->items[map->size] = *item;
    map->size++;

    return OK;
}

void *LinearMap_get(linear_map_t *map, char *key)
{
    assert(map != NULL && "map must not be NULL to get item from");
    for (int i = 0; i < map->size; i++)
    {
        if (strcmp(map->items[i].key, key) == 0)
        {
            return map->items[i].value;
        }
    }
    return NULL;
}

#endif // !#ifndef LINEAR_MAP_C
