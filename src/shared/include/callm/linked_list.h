#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include "errors.h"
#include <stddef.h>

#define LINKED_LIST_ITER(list, item) for (LinkedList *item = list; item != NULL; item = LinkedList_get_next(item))

typedef struct LinkedList LinkedList;

LinkedList *LinkedList_new();

CallmStatusCode LinkedList_free(LinkedList *list);

void LinkedList_add(LinkedList *list, void *value);

void *LinkedList_get_head_value(LinkedList *list);

LinkedList *LinkedList_get_next(LinkedList *list);

size_t LinkedList_size(LinkedList *list);

#endif  // LINKED_LIST_H
