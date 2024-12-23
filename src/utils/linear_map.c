#include "linear_map.h"
#include "errors.h"
#include "logging.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define LINEAR_MAP_DEFAULT_CAPACITY 16

static LinearMapItem *
LineraMapItem_new(char *key, void *value, int value_size)
{
    LinearMapItem *item = (LinearMapItem *) malloc(sizeof(LinearMapItem));
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
static CallmStatusCode
LinearMap_resize(LinearMap *map)
{
    if (map->size < map->capacity)
    {
        return OK;
    }

    int new_capacity = map->capacity * 2;
    LinearMapItem *new_items = (LinearMapItem *) malloc(sizeof(LinearMapItem) * new_capacity);
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

LinearMap *
LinearMap_new()
{
    LinearMap *map = (LinearMap *) malloc(sizeof(LinearMap));
    (*map).size = 0;
    (*map).capacity = LINEAR_MAP_DEFAULT_CAPACITY;
    (*map).items = (LinearMapItem *) malloc(sizeof(LinearMapItem) * LINEAR_MAP_DEFAULT_CAPACITY);
    return map;
}

void
LinearMap_free(LinearMap *map)
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

CallmStatusCode
LinearMap_insert(LinearMap *map, char *key, void *value, int value_size)
{
    assert(map != NULL && "map must not be NULL to insert item into");

    if (LinearMap_resize(map) != OK)
    {
        LOG_ERROR("Failed to insert item into linear map: resize failed");
        return ERROR;
    }

    LinearMapItem *item = LineraMapItem_new(key, value, value_size);
    if (item == NULL)
    {
        LOG_ERROR("Failed to insert item into linear map: failed to create new item");
        return ERROR;
    }

    map->items[map->size] = *item;
    map->size++;

    return OK;
}

void *
LinearMap_get(LinearMap *map, char *key)
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
