#include "dynamic_list.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

DynamicList *
DynamicList_new (size_t item_size)
{
  DynamicList *list = (DynamicList *)malloc (sizeof (DynamicList));
  list->data = NULL;
  list->size = 0;
  list->capacity = 10; // TODO magic  number
  list->item_size = item_size;
  list->data = (void *)malloc (list->capacity * sizeof (item_size));
  return list;
}

CallmStatusCode
DynamicList_append (DynamicList *list, void *item)
{
  if (list->size == list->capacity)
    {
      list->capacity *= 2;
      list->data = realloc (list->data, list->capacity * list->item_size);
      CHECK_MALLOC (list->data, "DynamicList_append");
    }
  memcpy (list->data + list->size * list->item_size, item, list->item_size);
  list->size++;
  return OK;
}

CallmStatusCode
DynamicList_free (DynamicList *list)
{
  if (list->data != NULL)
    {
      free (list->data);
    }
  free (list);
  return OK;
}

CallmStatusCode
DynamicList_print (DynamicList *list)
{
  printf ("DynamicList(");
  printf ("size=%zu, ", list->size);
  printf ("capacity=%zu)", list->capacity);
  printf ("\n");
  printf ("First 10 elements:\n");
  for (int i = 0; i < 10; i++)
    {
      printf ("%d: %d\n", i,
              *((int *)(list->data
                        + i * sizeof (int)))); // TODO use generic type here
    }
  return OK;
}
