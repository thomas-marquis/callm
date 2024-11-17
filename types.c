#ifndef TYPES_C
#define TYPES_C

#include "types.h"
#include "utils.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOAD_FACTOR 0.75

float bf16_to_float(bf16 b)
{
    // Extraction of bfloat16 components
    bf16 sign = (b >> 15) & 0x1;     // 1 bit for the sign      0000 0001
    bf16 exponent = (b >> 7) & 0xFF; // 8 bits for the exponent 1111 1111
    bf16 mantissa = b & 0x7F;        // 7 bits for the mantissa 0111 1111

    // If the exponent is zero, we have a zero or a denormalized number
    if (exponent == 0)
    {
        if (mantissa == 0)
        {
            // Zero number (positive or negative)
            return sign ? -0.0f : 0.0f;
        }
        else
        {
            // Denormalized number (not handled here for simplicity)
            return 0.0f;
        }
    }
    else if (exponent == 0x1F)
    {
        // Maximal exponent case: infinity or NaN
        if (mantissa == 0)
        {
            return sign ? -INFINITY : INFINITY;
        }
        else
        {
            return NAN; // NaN
        }
    }

    // Calculate the new exponent for float32
    uint32_t new_sign = sign << 31;         // Sign on bit 31
    uint32_t new_exponent = exponent << 23; // Shifted exponent
    uint32_t new_mantissa = mantissa << 16; // Extended mantissa

    // Construct the integer representing the float32
    uint32_t float_bits = new_sign | new_exponent | new_mantissa;

    // Interpret the number as a float
    float result;
    memcpy(&result, &float_bits, sizeof(result));

    return result;
}

unsigned int hash_table_hash(const char *key, int table_size)
{
    unsigned int hash = 0;
    while (*key)
    {
        hash = (hash << 5) + *key++;
    }
    return hash % table_size;
}

HashTableNode *new_hash_table_node(const char *key, const void *value, size_t value_size)
{
    HashTableNode *new_node = (HashTableNode *)malloc(sizeof(HashTableNode));
    new_node->key = strdup(key);
    new_node->value = malloc(value_size);
    memcpy(new_node->value, value, value_size);
    new_node->next = NULL;
    return new_node;
}

HashTable *new_hash_table(int size)
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

int hash_table_resize(HashTable *hash_table)
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
            unsigned int index = hash_table_hash(temp->key, new_size);
            temp->next = new_table[index];
            new_table[index] = temp;
            temp = next;
        }
    }

    free(hash_table->table);
    hash_table->table = new_table;
    hash_table->size = new_size;
    return 0;
}

int hash_table_insert(HashTable *hash_table, const char *key, const void *value, size_t value_size)
{
    if ((float)hash_table->count / hash_table->size > LOAD_FACTOR)
    {
        if (hash_table_resize(hash_table))
        {
            return 1;
        }
    }

    unsigned int index = hash_table_hash(key, hash_table->size);
    HashTableNode *new_node = new_hash_table_node(key, value, value_size);
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
    return 0;
}

void *hash_table_get(HashTable *hash_table, const char *key)
{
    unsigned int index = hash_table_hash(key, hash_table->size);
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

int hash_table_delete(HashTable *hash_table, const char *key)
{
    unsigned int index = hash_table_hash(key, hash_table->size);
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
    return 0;
}

int hash_table_free(HashTable *hash_table)
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
    return 0;
}

#endif
