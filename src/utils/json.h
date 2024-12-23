#ifndef LIB_JSON_H
#define LIB_JSON_H

#include <jansson.h>

#define GET_JSON_OBJECT(root, key, obj)                                                                                \
    json_object_get(root, key);                                                                                        \
    if (obj == NULL)                                                                                                   \
    {                                                                                                                  \
        printerr("Error getting %s key from JSON\n", key);                                                             \
        return ERROR;                                                                                                  \
    }
#define GET_JSON_OBJECT_PANIC(root, key, obj)                                                                          \
    json_object_get(root, key);                                                                                        \
    if (obj == NULL)                                                                                                   \
    {                                                                                                                  \
        printerr("Error getting %s key from JSON\n", key);                                                             \
        exit(1);                                                                                                       \
    }

#endif
