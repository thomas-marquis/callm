#ifndef CALLM_SAFEPARSER_METADATA_VIEW_H
#define CALLM_SAFEPARSER_METADATA_VIEW_H

#include "../core/safetensors.h"
#include "../shared/errors.h"
#include "query_options.h"
#include <stddef.h>

typedef struct
{
    char *name;
    int *shape;
    size_t shape_size;
    enum Dtype dtype;
} TensorMetadata;

typedef struct
{
    TensorMetadata *items;
    size_t count;
} MetadataView;

CallmStatusCode MetadataView_build(const Safetensors *safetensors, MetadataView *out_view);

CallmStatusCode MetadataView_filter(const MetadataView *input, const QueryOptions *options, MetadataView *out_view);

void MetadataView_sort(MetadataView *view, QuerySortBy sort_by);

void MetadataView_free(MetadataView *view);

const char *MetadataView_dtype_to_string(enum Dtype dtype);

#endif
