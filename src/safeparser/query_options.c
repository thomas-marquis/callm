#include "query_options.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void
QueryOptions_init(QueryOptions *options)
{
    options->show_help = 0;
    options->file_path = NULL;
    options->has_filter_name = 0;
    options->filter_name = NULL;
    options->has_filter_shape = 0;
    options->filter_shape = NULL;
    options->filter_shape_size = 0;
    options->has_filter_dtype = 0;
    options->filter_dtype = F32;
    options->sort_by = QUERY_SORT_BY_NAME;
    options->output_mode = QUERY_OUTPUT_MODE_TEXT;
}

void
QueryOptions_free(QueryOptions *options)
{
    free(options->filter_shape);
    options->filter_shape = NULL;
    options->filter_shape_size = 0;
}

void
QueryOptions_print_help(void)
{
    printf("Usage: safeparser [OPTIONS] <FILE>\n\n");
    printf("Parse a Safetensors file and print tensor metadata only (name, shape, dtype).\n");
    printf("Tensor payload values are never loaded into matrices for display.\n\n");
    printf("Arguments:\n");
    printf("  <FILE>                       Path to the .safetensors file\n\n");
    printf("Options:\n");
    printf("  --filter-name <NAME>         Exact tensor name match\n");
    printf("  --filter-shape <D1,D2,...>   Exact shape match, comma-separated dimensions\n");
    printf("  --filter-dtype <DTYPE>       Dtype filter (supported: F32, BF16)\n");
    printf("  --sort-by <name|shape|dtype> Sort output by a single key (ascending)\n");
    printf("  --output <text|csv>          Output format (default: text)\n");
    printf("  -h, --help                   Show this help message and exit\n");
}

static CallmStatusCode
QueryOptions_parse_output_mode(const char *output_mode_string, QueryOutputMode *output_mode)
{
    if (strcmp(output_mode_string, "text") == 0)
    {
        *output_mode = QUERY_OUTPUT_MODE_TEXT;
        return OK;
    }

    if (strcmp(output_mode_string, "csv") == 0)
    {
        *output_mode = QUERY_OUTPUT_MODE_CSV;
        return OK;
    }

    printerr("Invalid output mode: '%s' (supported: text, csv)\n", output_mode_string);
    return ERROR;
}

static CallmStatusCode
QueryOptions_parse_shape(const char *shape_string, int **shape, size_t *shape_size)
{
    if (shape_string == NULL || shape_string[0] == '\0')
    {
        printerr("Invalid shape filter: value cannot be empty\n");
        return ERROR;
    }

    size_t count = 0;
    int *dims = NULL;

    const char *cursor = shape_string;
    while (*cursor != '\0')
    {
        errno = 0;
        char *next = NULL;
        long value = strtol(cursor, &next, 10);

        if (next == cursor || errno == ERANGE || value < 0 || value > INT_MAX)
        {
            free(dims);
            printerr("Invalid shape filter: '%s'\n", shape_string);
            return ERROR;
        }

        int *new_dims = (int *) realloc(dims, (count + 1) * sizeof(int));
        if (new_dims == NULL)
        {
            free(dims);
            printerr("Error allocating memory for shape filter\n");
            return ERROR;
        }

        dims = new_dims;
        dims[count] = (int) value;
        count += 1;

        if (*next == '\0')
        {
            break;
        }

        if (*next != ',')
        {
            free(dims);
            printerr("Invalid shape filter: '%s'\n", shape_string);
            return ERROR;
        }

        cursor = next + 1;
        if (*cursor == '\0')
        {
            free(dims);
            printerr("Invalid shape filter: '%s'\n", shape_string);
            return ERROR;
        }
    }

    if (count == 0)
    {
        free(dims);
        printerr("Invalid shape filter: '%s'\n", shape_string);
        return ERROR;
    }

    *shape = dims;
    *shape_size = count;
    return OK;
}

static CallmStatusCode
QueryOptions_parse_dtype(const char *dtype_string, enum Dtype *dtype)
{
    if (strcmp(dtype_string, "F32") == 0)
    {
        *dtype = F32;
        return OK;
    }

    if (strcmp(dtype_string, "BF16") == 0)
    {
        *dtype = BF16;
        return OK;
    }

    printerr("Invalid dtype filter: '%s' (supported: F32, BF16)\n", dtype_string);
    return ERROR;
}

static CallmStatusCode
QueryOptions_parse_sort(const char *sort_string, QuerySortBy *sort_by)
{
    if (strcmp(sort_string, "name") == 0)
    {
        *sort_by = QUERY_SORT_BY_NAME;
        return OK;
    }

    if (strcmp(sort_string, "shape") == 0)
    {
        *sort_by = QUERY_SORT_BY_SHAPE;
        return OK;
    }

    if (strcmp(sort_string, "dtype") == 0)
    {
        *sort_by = QUERY_SORT_BY_DTYPE;
        return OK;
    }

    printerr("Invalid sort key: '%s' (supported: name, shape, dtype)\n", sort_string);
    return ERROR;
}

CallmStatusCode
QueryOptions_parse(int argc, char **argv, QueryOptions *options)
{
    QueryOptions_init(options);

    for (int i = 1; i < argc; i++)
    {
        const char *arg = argv[i];

        if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0)
        {
            options->show_help = 1;
            return OK;
        }

        if (strcmp(arg, "--filter-name") == 0)
        {
            if (i + 1 >= argc)
            {
                printerr("Missing value for --filter-name\n");
                return ERROR;
            }

            options->has_filter_name = 1;
            options->filter_name = argv[++i];
            continue;
        }

        if (strcmp(arg, "--filter-shape") == 0)
        {
            if (i + 1 >= argc)
            {
                printerr("Missing value for --filter-shape\n");
                return ERROR;
            }

            int *shape = NULL;
            size_t shape_size = 0;
            if (QueryOptions_parse_shape(argv[++i], &shape, &shape_size) != OK)
            {
                return ERROR;
            }

            free(options->filter_shape);
            options->filter_shape = shape;
            options->filter_shape_size = shape_size;
            options->has_filter_shape = 1;
            continue;
        }

        if (strcmp(arg, "--filter-dtype") == 0)
        {
            if (i + 1 >= argc)
            {
                printerr("Missing value for --filter-dtype\n");
                return ERROR;
            }

            enum Dtype dtype;
            if (QueryOptions_parse_dtype(argv[++i], &dtype) != OK)
            {
                return ERROR;
            }

            options->has_filter_dtype = 1;
            options->filter_dtype = dtype;
            continue;
        }

        if (strcmp(arg, "--sort-by") == 0)
        {
            if (i + 1 >= argc)
            {
                printerr("Missing value for --sort-by\n");
                return ERROR;
            }

            if (QueryOptions_parse_sort(argv[++i], &options->sort_by) != OK)
            {
                return ERROR;
            }
            continue;
        }

        if (strcmp(arg, "--output") == 0)
        {
            if (i + 1 >= argc)
            {
                printerr("Missing value for --output\n");
                return ERROR;
            }

            if (QueryOptions_parse_output_mode(argv[++i], &options->output_mode) != OK)
            {
                return ERROR;
            }
            continue;
        }

        if (arg[0] == '-')
        {
            printerr("Unknown option: %s\n", arg);
            return ERROR;
        }

        if (options->file_path != NULL)
        {
            printerr("Unexpected argument: %s\n", arg);
            return ERROR;
        }

        options->file_path = arg;
    }

    if (options->file_path == NULL)
    {
        printerr("Missing Safetensors file path\n");
        return ERROR;
    }

    return OK;
}
