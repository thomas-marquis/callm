#ifndef DYNAMIC_LIST_H
#define DYNAMIC_LIST_H

#include "./errors.h"
#include <stddef.h>

typedef struct
{
  void *data;
  size_t item_size;
  size_t size;
  size_t capacity;
} DynamicList;

DynamicList *DynamicList_new (size_t item_size);

CallmStatusCode DynamicList_append (DynamicList *list, void *item);

CallmStatusCode DynamicList_free (DynamicList *list);

CallmStatusCode DynamicList_print (DynamicList *list);

#endif // !#ifndef DYNAMIC_LIST_H
