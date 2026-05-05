#include "output.h"

#include <stdio.h>

static void
Output_write_csv_field(const char *field)
{
    int requires_quotes = 0;
    for (const char *c = field; *c != '\0'; c++)
    {
        if (*c == ',' || *c == '"' || *c == '\n')
        {
            requires_quotes = 1;
            break;
        }
    }

    if (!requires_quotes)
    {
        fputs(field, stdout);
        return;
    }

    putchar('"');
    for (const char *c = field; *c != '\0'; c++)
    {
        if (*c == '"')
        {
            fputs("\"\"", stdout);
            continue;
        }
        putchar(*c);
    }
    putchar('"');
}

static CallmStatusCode
Output_print_text(const MetadataView *view)
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

static CallmStatusCode
Output_print_csv(const MetadataView *view)
{
    printf("name,shape,dtype\n");

    for (size_t i = 0; i < view->count; i++)
    {
        const TensorMetadata *item = &view->items[i];

        Output_write_csv_field(item->name);
        putchar(',');

        putchar('"');
        for (size_t d = 0; d < item->shape_size; d++)
        {
            if (d > 0)
            {
                putchar(',');
            }
            printf("%d", item->shape[d]);
        }
        putchar('"');

        putchar(',');
        Output_write_csv_field(MetadataView_dtype_to_string(item->dtype));
        putchar('\n');
    }

    return OK;
}

CallmStatusCode
Output_print(const MetadataView *view, QueryOutputMode output_mode)
{
    if (output_mode == QUERY_OUTPUT_MODE_TEXT)
    {
        return Output_print_text(view);
    }

    if (output_mode == QUERY_OUTPUT_MODE_CSV)
    {
        return Output_print_csv(view);
    }

    printerr("Unknown output mode: %d\n", output_mode);
    return ERROR;
}
