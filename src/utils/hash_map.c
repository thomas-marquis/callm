#include "hash_map.h"
#include "errors.h"
#include "logging.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASH_MAP_INITIAL_CAPACITY 16
#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL

typedef struct
{
    const char *key;
    void *value;
} HashMapItem;

struct HashMap
{
    HashMapItem *items;
    size_t size;
    size_t capacity;
};

// Return 64-bit FNV-1a hash for key (NUL-terminated). See description:
// https://en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function
static uint64_t
hash_key(const char *key)
{
    uint64_t hash = FNV_OFFSET;
    for (const char *p = key; *p; p++)
    {
        hash ^= (uint64_t) (unsigned char) (*p);
        hash *= FNV_PRIME;
    }
    return hash;
}

static CallmStatusCode
insert_item(HashMapItem *items, size_t capacity, size_t *map_size, const char *key, void *value)
{
    uint64_t hash = hash_key(key);
    size_t index = (size_t) (hash & (uint64_t) (capacity - 1));

    LOGF_INFO("Inserting item in hash map at index %zu", index);

    // Loop till we find an empty entry.
    while (items[index].key != NULL)
    {
        LOGF_INFO("TOTO %s:", items[index].key);
        if (strcmp(key, items[index].key) == 0)
        {
            LOGF_INFO("Key already exists in hash map: %s", key);

            // Found key (it already exists), update value.
            items[index].value = value;

            LOG_INFO("COUCOU");

            return OK;
        }
        // Key wasn't in this slot, move to next (linear probing).
        LOG_INFO("DUDU1");
        index++;
        LOG_INFO("DUDU2");
        if (index >= capacity)
        {
            // At end of entries array, wrap around.
            index = 0;
        }
    }

    // Didn't find key, allocate+copy if needed, then insert it.
    if (map_size != NULL)
    {
        LOGF_INFO("Allocating memory for key %s", key);
        key = strdup(key);

        LOG_INFO("DUDU3");

        LOGF_INFO("Duplicated key: %s", (char *) key);
        CHECK_MALLOC(key, "error duplicating key");
        (*map_size)++;
    }
    items[index].key = key;
    LOG_INFO("Setting value in hash map");

    items[index].value = value;

    return OK;
}

static CallmStatusCode
expand_map(HashMap *map)
{
    size_t new_capacity = map->capacity * 2;
    if (new_capacity < map->capacity)
    {
        LOG_ERROR("error expanding map: new capacity would overflow");
        return ERROR;
    }

    HashMapItem *new_items = calloc(new_capacity, sizeof(HashMapItem));
    CHECK_MALLOC(new_items, "error allocating memory for new items");

    // Iterate entries, move all non-empty ones to new table's entries.
    for (size_t i = 0; i < map->capacity; i++)
    {
        HashMapItem item = map->items[i];
        if (item.key != NULL)
        {
            if (insert_item(new_items, new_capacity, NULL, item.key, item.value) == ERROR)
            {
                LOG_ERROR("error inserting item in new map");
                return ERROR;
            }
        }
    }

    // Free old entries array and update this amp's details.
    free(map->items);
    map->items = new_items;
    map->capacity = new_capacity;

    return OK;
}

HashMap *
HashMap_new()
{
    HashMap *map = malloc(sizeof(HashMap));
    CHECK_MALLOC_PANIC(map, "error allocating memory for HashMapStrStr");

    map->capacity = HASH_MAP_INITIAL_CAPACITY;
    map->size = 0;

    map->items = calloc(map->capacity, sizeof(HashMapItem));
    CHECK_MALLOC_PANIC(map->items, "error allocating memory for initial HashMapStrStr items");

    return map;
}

CallmStatusCode
HashMap_free(HashMap *map)
{
    if (map == NULL)
    {
        return OK;
    }
    if (map->items != NULL)
    {
        for (int i = 0; i < map->capacity; i++)
        {
            free((void *) map->items[i].key);
        }
        free(map->items);
    }
    free(map);

    return OK;
}

CallmStatusCode
HashMap_insert(HashMap *map, const char *key, void *value)
{
    if (value == NULL)
    {

        LOG_ERROR("impossible to insert null pointer value in hash map");
        return ERROR;
    }

    if (map->size >= map->capacity / 2)
    {
        CallmStatusCode err = expand_map(map);
        if (err != OK)
        {
            LOGF_ERROR("error expanding map: %s", CallmStatusCode_string(err));
            return ERROR;
        }
    }

    return insert_item(map->items, map->capacity, &map->size, key, value);
}

void *
HashMap_get(HashMap *map, const char *key)
{
    // AND hash with capacity-1 to ensure it's within entries array.
    uint64_t hash = hash_key(key);
    size_t index = (size_t) (hash & (uint64_t) (map->capacity - 1));

    // Loop till we find an empty entry.
    while (map->items[index].key != NULL)
    {
        if (strcmp(key, map->items[index].key) == 0)
        {
            // Found key, return value.
            return map->items[index].value;
        }
        // Key wasn't in this slot, move to next (linear probing).
        index++;
        if (index >= map->capacity)
        {
            // At end of entries array, wrap around.
            index = 0;
        }
    }
    return NULL;
}

size_t
HashMap_size(HashMap *map)
{
    return map->size;
}
