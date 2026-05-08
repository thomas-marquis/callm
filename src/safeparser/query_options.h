#ifndef CALLM_SAFEPARSER_QUERY_OPTIONS_H
#define CALLM_SAFEPARSER_QUERY_OPTIONS_H

#include "../safetensors/include/callm/safetensors.h"
#include "../shared/include/callm/errors.h"
#include <stddef.h>

typedef enum
{
    QUERY_SORT_BY_NAME,
    QUERY_SORT_BY_SHAPE,
    QUERY_SORT_BY_DTYPE,
} QuerySortBy;

typedef enum
{
    QUERY_OUTPUT_MODE_TEXT,
    QUERY_OUTPUT_MODE_CSV,
} QueryOutputMode;

typedef struct
{
    int show_help;
    const char *file_path;

    int has_filter_name;
    const char *filter_name;

    int has_filter_shape;
    int *filter_shape;
    size_t filter_shape_size;

    int has_filter_dtype;
    enum Dtype filter_dtype;

    QuerySortBy sort_by;
    QueryOutputMode output_mode;
} QueryOptions;

void QueryOptions_init(QueryOptions *options);

void QueryOptions_free(QueryOptions *options);

CallmStatusCode QueryOptions_parse(int argc, char **argv, QueryOptions *options);

void QueryOptions_print_help(void);

#endif
