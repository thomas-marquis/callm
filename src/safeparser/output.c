#include "output.h"

#include <stdio.h>

CallmStatusCode
Output_print(const MetadataView *view)
{
    for (size_t i = 0; i < view->count; i++)
    {
        const TensorMetadata *item = &view->items[i];
        printf("name=%s shape=[", item->name);

        for (size_t d = 0; d < item->shape_size; d++)
        {
            if (d > 0)
            {
                printf(",");
            }
            printf("%d", item->shape[d]);
        }

        printf("] dtype=%s\n", MetadataView_dtype_to_string(item->dtype));
    }

    return OK;
}
