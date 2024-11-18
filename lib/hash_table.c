#ifndef LIB_HASH_TABLE_C
#define LIB_HASH_TABLE_C

#include "hash_table.h"
#include <stdlib.h>
#include <string.h>

unsigned int hash_table_hash(const char *key, int table_size)
{
    unsigned int hash = 0;
    while (*key)
    {
        hash = (hash << 5) + *key++;
    }
    return hash % table_size;
}

hash_table_node_t *new_hash_table_node(const char *key, const void *value, size_t value_size)
{
    hash_table_node_t *new_node = (hash_table_node_t *)malloc(sizeof(hash_table_node_t));
    new_node->key = strdup(key);
    new_node->value = malloc(value_size);
    memcpy(new_node->value, value, value_size);
    new_node->next = NULL;
    return new_node;
}

hash_table_t *new_hash_table(int size)
{
    hash_table_t *hash_table = (hash_table_t *)malloc(sizeof(hash_table_t));
    CHECK_MALLOC_PANIC(hash_table, "hash table");
    hash_table->table = (hash_table_node_t **)malloc(sizeof(hash_table_node_t *) * size);
    CHECK_MALLOC_PANIC(hash_table->table, "hash table inner table");
    for (int i = 0; i < size; i++)
    {
        hash_table->table[i] = NULL;
    }
    hash_table->size = size;
    hash_table->count = 0;
    return hash_table;
}

status_t hash_table_resize(hash_table_t *hash_table)
{
    int new_size = hash_table->size * 2;
    hash_table_node_t **new_table = (hash_table_node_t **)malloc(sizeof(hash_table_node_t *) * new_size);
    CHECK_MALLOC(new_table, "new hash table inner table");
    for (int i = 0; i < new_size; i++)
    {
        new_table[i] = NULL;
    }

    for (int i = 0; i < hash_table->size; i++)
    {
        hash_table_node_t *temp = hash_table->table[i];
        while (temp)
        {
            hash_table_node_t *next = temp->next;
            unsigned int index = hash_table_hash(temp->key, new_size);
            temp->next = new_table[index];
            new_table[index] = temp;
            temp = next;
        }
    }

    free(hash_table->table);
    hash_table->table = new_table;
    hash_table->size = new_size;
    return OK;
}

status_t hash_table_insert(hash_table_t *hash_table, const char *key, const void *value, size_t value_size)
{
    if ((float)hash_table->count / hash_table->size > LOAD_FACTOR)
    {
        if (hash_table_resize(hash_table))
        {
            return ERROR;
        }
    }

    unsigned int index = hash_table_hash(key, hash_table->size);
    hash_table_node_t *new_node = new_hash_table_node(key, value, value_size);
    if (hash_table->table[index] == NULL)
    {
        hash_table->table[index] = new_node;
    }
    else
    {
        hash_table_node_t *temp = hash_table->table[index];
        while (temp->next)
        {
            temp = temp->next;
        }
        temp->next = new_node;
    }
    hash_table->count++;
    return OK;
}

void *hash_table_get(hash_table_t *hash_table, const char *key)
{
    unsigned int index = hash_table_hash(key, hash_table->size);
    hash_table_node_t *temp = hash_table->table[index];
    while (temp)
    {
        if (strcmp(temp->key, key) == 0)
        {
            return temp->value;
        }
        temp = temp->next;
    }
    return NULL;
}

status_t hash_table_delete(hash_table_t *hash_table, const char *key)
{
    unsigned int index = hash_table_hash(key, hash_table->size);
    hash_table_node_t *temp = hash_table->table[index];
    hash_table_node_t *prev = NULL;
    while (temp)
    {
        if (strcmp(temp->key, key) == 0)
        {
            if (prev == NULL)
            {
                hash_table->table[index] = temp->next;
            }
            else
            {
                prev->next = temp->next;
            }
            free(temp->key);
            free(temp->value);
            free(temp);
            hash_table->count--;
            return 0;
        }
        prev = temp;
        temp = temp->next;
    }
    return OK;
}

status_t hash_table_free(hash_table_t *hash_table)
{
    for (int i = 0; i < hash_table->size; i++)
    {
        hash_table_node_t *temp = hash_table->table[i];
        while (temp)
        {
            hash_table_node_t *to_free = temp;
            temp = temp->next;
            free(to_free->key);
            free(to_free->value);
            free(to_free);
        }
    }
    free(hash_table->table);
    free(hash_table);
    return OK;
}

#endif // !LIB_HASH_TABLE_C
