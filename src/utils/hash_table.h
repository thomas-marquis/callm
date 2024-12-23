#ifndef LIB_HASH_TABLE_H
#define LIB_HASH_TABLE_H

#include "errors.h"
#include <stddef.h>

#define LOAD_FACTOR 0.75

struct HashTableNode
{
    char *key;
    char *value;
    struct HashTableNode *next;
};
typedef struct HashTableNode HashTableNode;

typedef struct
{
    HashTableNode **table;
    int size;
    int count;
} HashTable;

/**
 * Computes the hash value for a given key.
 *
 * @param key The key to hash.
 * @param table_size The size of the hash table.
 * @return The hash value for the key.
 */
unsigned int HashTable_hash(const char *key, int table_size);

/**
 * Creates a new hash table node.
 *
 * @param key The key for the node.
 * @param value The value for the node.
 * @param value_size The size of the value.
 * @return A pointer to the newly created node.
 */
HashTableNode *HashTableNode_new(const char *key, const void *value, size_t value_size);

/**
 * Creates a new hash table.
 *
 * @param size The initial size of the hash table.
 * @return A pointer to the newly created hash table.
 */
HashTable *HashTable_new(int size);

/**
 * Resizes the hash table to accommodate more elements.
 *
 * @param hash_table The hash table to resize.
 * @return CallmStatusCode OK if the resize was successful, ERROR otherwise.
 */
CallmStatusCode HashTable_resize(HashTable *hash_table);

/**
 * Inserts a key-value pair into the hash table.
 *
 * @param hash_table The hash table to insert into.
 * @param key The key to insert.
 * @param value The value to insert.
 * @param value_size The size of the value.
 * @return status_t OK if the resize was successful, ERROR otherwise.
 */
CallmStatusCode HashTable_insert(HashTable *hash_table, const char *key, const void *value, size_t value_size);

/**
 * Retrieves a value from the hash table by key.
 *
 * @param hash_table The hash table to search.
 * @param key The key to search for.
 * @return The value associated with the key, or NULL if the key is not found.
 */
void *HashTable_get(HashTable *hash_table, const char *key);

/**
 * Deletes a key-value pair from the hash table.
 *
 * @param hash_table The hash table to delete from.
 * @param key The key to delete.
 * @return status_t OK if the resize was successful, ERROR otherwise.
 */
CallmStatusCode HashTable_delete(HashTable *hash_table, const char *key);

/**
 * Frees the memory allocated for the hash table.
 *
 * @param hash_table The hash table to free.
 * @return CallmStatusCode OK if the resize was successful, ERROR otherwise.
 */
CallmStatusCode HashTable_free(HashTable *hash_table);

char **HashTable_keys(HashTable *hash_table);

#endif  // !LIB_HASH_TABLE_H
