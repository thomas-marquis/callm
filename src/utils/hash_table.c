#include "hash_table.h"
#include "errors.h"
#include <stdlib.h>
#include <string.h>

unsigned int HashTable_hash(const char *key, int table_size)
{
    unsigned int hash = 0;
    while (*key)
    {
        hash = (hash << 5) + *key++;
    }
    return hash % table_size;
}

HashTableNode *HashTableNode_new(const char *key, const void *value, size_t value_size)
{
    HashTableNode *new_node = (HashTableNode *)malloc(sizeof(HashTableNode));
    new_node->key = strdup(key);
    new_node->value = malloc(value_size);
    memcpy(new_node->value, value, value_size);
    new_node->next = NULL;
    return new_node;
}

HashTable *HashTable_new(int size)
{
    HashTable *hash_table = (HashTable *)malloc(sizeof(HashTable));
    CHECK_MALLOC_PANIC(hash_table, "hash table");
    hash_table->table = (HashTableNode **)malloc(sizeof(HashTableNode *) * size);
    CHECK_MALLOC_PANIC(hash_table->table, "hash table inner table");
    for (int i = 0; i < size; i++)
    {
        hash_table->table[i] = NULL;
    }
    hash_table->size = size;
    hash_table->count = 0;
    return hash_table;
}

CallmStatusCode HashTable_resize(HashTable *hash_table)
{
    int new_size = hash_table->size * 2;
    HashTableNode **new_table = (HashTableNode **)malloc(sizeof(HashTableNode *) * new_size);
    CHECK_MALLOC(new_table, "new hash table inner table");
    for (int i = 0; i < new_size; i++)
    {
        new_table[i] = NULL;
    }

    for (int i = 0; i < hash_table->size; i++)
    {
        HashTableNode *temp = hash_table->table[i];
        while (temp)
        {
            HashTableNode *next = temp->next;
            unsigned int index = HashTable_hash(temp->key, new_size);
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

CallmStatusCode HashTable_insert(HashTable *hash_table, const char *key, const void *value, size_t value_size)
{
    if ((float)hash_table->count / hash_table->size > LOAD_FACTOR)
    {
        if (HashTable_resize(hash_table))
        {
            return ERROR;
        }
    }

    unsigned int index = HashTable_hash(key, hash_table->size);
    HashTableNode *new_node = HashTableNode_new(key, value, value_size);
    if (hash_table->table[index] == NULL)
    {
        hash_table->table[index] = new_node;
    }
    else
    {
        HashTableNode *temp = hash_table->table[index];
        while (temp->next)
        {
            temp = temp->next;
        }
        temp->next = new_node;
    }
    hash_table->count++;
    return OK;
}

void *HashTable_get(HashTable *hash_table, const char *key)
{
    unsigned int index = HashTable_hash(key, hash_table->size);
    HashTableNode *temp = hash_table->table[index];
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

CallmStatusCode HashTable_delete(HashTable *hash_table, const char *key)
{
    unsigned int index = HashTable_hash(key, hash_table->size);
    HashTableNode *temp = hash_table->table[index];
    HashTableNode *prev = NULL;
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

CallmStatusCode HashTable_free(HashTable *hash_table)
{
    for (int i = 0; i < hash_table->size; i++)
    {
        HashTableNode *temp = hash_table->table[i];
        while (temp)
        {
            HashTableNode *to_free = temp;
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

char **HashTable_keys(HashTable *hash_table)
{
    char **keys = (char **)malloc(sizeof(char *) * hash_table->count);
    for (int i = 0; i < hash_table->count; i++)
    {
        HashTableNode *temp = hash_table->table[i];
        printf("COUCOU : key=%p\n", temp);
        keys[i] = temp->key;
    }
    return keys;
}
