#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>
#include <stdint.h>

typedef uint16_t bf16;

/**
 * Converts a bfloat16 (bf16) number to a float32 (float) number.
 *
 * @param b The bfloat16 number to be converted.
 * @return The corresponding float32 number.
 *
 * This function performs the following steps:
 * 1. Extracts the sign, exponent, and mantissa from the bfloat16 number.
 * 2. Handles special cases:
 *    - If the exponent is zero and the mantissa is zero, the result is zero (positive or negative).
 *    - If the exponent is zero and the mantissa is non-zero, the result is treated as zero (denormalized numbers are
 * not handled).
 *    - If the exponent is maximal (0x1F), the result is either infinity (if mantissa is zero) or NaN (if mantissa is
 * non-zero).
 * 3. Adjusts the exponent and mantissa to fit the float32 format.
 * 4. Constructs the float32 number from the adjusted sign, exponent, and mantissa.
 * 5. Returns the resulting float32 number.
 */
float bf16_to_float(bf16 b);

typedef struct HashTableNode
{
    char *key;
    char *value;
    struct HashTableNode *next;
} HashTableNode;

typedef struct HashTable
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
unsigned int hash_table_hash(const char *key, int table_size);

/**
 * Creates a new hash table node.
 *
 * @param key The key for the node.
 * @param value The value for the node.
 * @param value_size The size of the value.
 * @return A pointer to the newly created node.
 */
HashTableNode *new_hash_table_node(const char *key, const void *value, size_t value_size);

/**
 * Creates a new hash table.
 *
 * @param size The initial size of the hash table.
 * @return A pointer to the newly created hash table.
 */
HashTable *new_hash_table(int size);

/**
 * Resizes the hash table to accommodate more elements.
 *
 * @param hash_table The hash table to resize.
 * @return 0 if the resize was successful, 1 otherwise.
 */
int hash_table_resize(HashTable *hash_table);

/**
 * Inserts a key-value pair into the hash table.
 *
 * @param hash_table The hash table to insert into.
 * @param key The key to insert.
 * @param value The value to insert.
 * @param value_size The size of the value.
 * @return 0 if the insertion was successful, 1 otherwise.
 */
int hash_table_insert(HashTable *hash_table, const char *key, const void *value, size_t value_size);

/**
 * Retrieves a value from the hash table by key.
 *
 * @param hash_table The hash table to search.
 * @param key The key to search for.
 * @return The value associated with the key, or NULL if the key is not found.
 */
void *hash_table_get(HashTable *hash_table, const char *key);

/**
 * Deletes a key-value pair from the hash table.
 *
 * @param hash_table The hash table to delete from.
 * @param key The key to delete.
 * @return 0 if the deletion was successful, 1 otherwise.
 */
int hash_table_delete(HashTable *hash_table, const char *key);

/**
 * Frees the memory allocated for the hash table.
 *
 * @param hash_table The hash table to free.
 * @return 0 if the hash table was successfully freed, 1 otherwise.
 */
int hash_table_free(HashTable *hash_table);

#endif
