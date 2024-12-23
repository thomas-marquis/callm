#include "linked_list.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct LinkedList
{
    void *value;
    LinkedList *prev;
    LinkedList *next;
};

LinkedList *
LinkedList_new()
{
    LinkedList *list = (LinkedList *) malloc(sizeof(LinkedList));
    list->value = NULL;
    list->prev = NULL;
    list->next = NULL;
    return list;
}

CallmStatusCode
LinkedList_free(LinkedList *list)
{
    LinkedList *current = list;
    while (current != NULL)
    {
        LinkedList *next = current->next;
        free(current);
        current = next;
    }

    return OK;
}

void
LinkedList_add(LinkedList *list, void *value)
{
    assert(list != NULL && "LinkedList_add: list is NULL");

    if ((*list).value == NULL) /* First item inserted */
    {
        (*list).value = value;
    }
    else
    {
        LinkedList *new_node = LinkedList_new();
        LinkedList *current = list;
        while (current->next != NULL)
        {
            current = current->next;
        }
        current->next = new_node;
        new_node->prev = current;
    }
}

void *
LinkedList_get_head_value(LinkedList *list)
{
    return list->value;
}

LinkedList *
LinkedList_get_next(LinkedList *list)
{
    return list->next;
}

size_t
LinkedList_size(LinkedList *list)
{
    assert(list != NULL && "LinkedList_size: list is NULL");
    if (list->value == NULL)
    {
        return 0;
    }

    LinkedList *current = list;
    size_t size = 1;
    for (; current->next != NULL; current = current->next)
    {
        size++;
    }

    return size;
}
