#ifndef HASH_MAP_H
#define HASH_MAP_H

#include "errors.h"

typedef struct HashMap HashMap;

HashMap *HashMap_new ();

CallmStatusCode HashMap_free (HashMap *map);

CallmStatusCode HashMap_insert (HashMap *map, const char *key, void *value);

void *HashMap_get (HashMap *map, const char *key);

#endif // HASH_MAP_H
