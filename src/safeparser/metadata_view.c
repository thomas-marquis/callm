#include "metadata_view.h"

#include <stdlib.h>
#include <string.h>

typedef struct
{
    MetadataView *view;
} MetadataBuildContext;

static QuerySortBy g_sort_by = QUERY_SORT_BY_NAME;

static int
MetadataView_compare_shape(const TensorMetadata *left, const TensorMetadata *right)
{
    if (left->shape_size != right->shape_size)
    {
        return left->shape_size < right->shape_size ? -1 : 1;
    }

    for (size_t i = 0; i < left->shape_size; i++)
    {
        if (left->shape[i] != right->shape[i])
        {
            return left->shape[i] < right->shape[i] ? -1 : 1;
        }
    }

    return 0;
}

static int
MetadataView_compare_dtype(const TensorMetadata *left, const TensorMetadata *right)
{
    const char *left_dtype = MetadataView_dtype_to_string(left->dtype);
    const char *right_dtype = MetadataView_dtype_to_string(right->dtype);
    return strcmp(left_dtype, right_dtype);
}

static int
MetadataView_compare_name(const TensorMetadata *left, const TensorMetadata *right)
{
    return strcmp(left->name, right->name);
}

static int
MetadataView_qsort_compare(const void *left_raw, const void *right_raw)
{
    const TensorMetadata *left = (const TensorMetadata *) left_raw;
    const TensorMetadata *right = (const TensorMetadata *) right_raw;

    int cmp;
    if (g_sort_by == QUERY_SORT_BY_SHAPE)
    {
        cmp = MetadataView_compare_shape(left, right);
        if (cmp != 0)
        {
            return cmp;
        }

        cmp = MetadataView_compare_name(left, right);
        if (cmp != 0)
        {
            return cmp;
        }

        return MetadataView_compare_dtype(left, right);
    }

    if (g_sort_by == QUERY_SORT_BY_DTYPE)
    {
        cmp = MetadataView_compare_dtype(left, right);
        if (cmp != 0)
        {
            return cmp;
        }

        cmp = MetadataView_compare_name(left, right);
        if (cmp != 0)
        {
            return cmp;
        }

        return MetadataView_compare_shape(left, right);
    }

    cmp = MetadataView_compare_name(left, right);
    if (cmp != 0)
    {
        return cmp;
    }

    cmp = MetadataView_compare_shape(left, right);
    if (cmp != 0)
    {
        return cmp;
    }

    return MetadataView_compare_dtype(left, right);
}

static void
MetadataView_copy_item(const TensorMetadata *src, TensorMetadata *dst)
{
    dst->dtype = src->dtype;
    dst->shape_size = src->shape_size;

    dst->name = (char *) malloc(strlen(src->name) + 1);
    CHECK_MALLOC_PANIC(dst->name, "metadata name");
    strcpy(dst->name, src->name);

    if (src->shape_size == 0)
    {
        dst->shape = NULL;
        return;
    }

    dst->shape = (int *) malloc(src->shape_size * sizeof(int));
    CHECK_MALLOC_PANIC(dst->shape, "metadata shape");
    memcpy(dst->shape, src->shape, src->shape_size * sizeof(int));
}

static CallmStatusCode
MetadataView_append_from_core(const SafetensorsTensorMetadata *metadata, void *context)
{
    MetadataBuildContext *build_context = (MetadataBuildContext *) context;
    MetadataView *view = build_context->view;

    TensorMetadata *new_items = (TensorMetadata *) realloc(view->items, (view->count + 1) * sizeof(TensorMetadata));
    if (new_items == NULL)
    {
        printerr("Error allocating metadata list\n");
        return ERROR;
    }

    view->items = new_items;

    TensorMetadata *item = &view->items[view->count];
    item->name = (char *) malloc(strlen(metadata->name) + 1);
    if (item->name == NULL)
    {
        printerr("Error allocating metadata name\n");
        return ERROR;
    }
    strcpy(item->name, metadata->name);

    item->shape_size = (size_t) metadata->shape_size;
    item->dtype = metadata->dtype;

    if (item->shape_size == 0)
    {
        item->shape = NULL;
        view->count += 1;
        return OK;
    }

    item->shape = (int *) malloc(item->shape_size * sizeof(int));
    if (item->shape == NULL)
    {
        free(item->name);
        printerr("Error allocating metadata shape\n");
        return ERROR;
    }

    memcpy(item->shape, metadata->shape, item->shape_size * sizeof(int));
    view->count += 1;
    return OK;
}

CallmStatusCode
MetadataView_build(const Safetensors *safetensors, MetadataView *out_view)
{
    out_view->items = NULL;
    out_view->count = 0;

    MetadataBuildContext context = { .view = out_view };
    return Safetensors_iterate_tensor_metadata(safetensors, MetadataView_append_from_core, &context);
}

static int
MetadataView_match_shape(const TensorMetadata *item, const QueryOptions *options)
{
    if (!options->has_filter_shape)
    {
        return 1;
    }

    if (item->shape_size != options->filter_shape_size)
    {
        return 0;
    }

    for (size_t i = 0; i < item->shape_size; i++)
    {
        if (item->shape[i] != options->filter_shape[i])
        {
            return 0;
        }
    }

    return 1;
}

static int
MetadataView_match_name(const TensorMetadata *item, const QueryOptions *options)
{
    if (!options->has_filter_name)
    {
        return 1;
    }

    return strcmp(item->name, options->filter_name) == 0;
}

static int
MetadataView_match_dtype(const TensorMetadata *item, const QueryOptions *options)
{
    if (!options->has_filter_dtype)
    {
        return 1;
    }

    return item->dtype == options->filter_dtype;
}

CallmStatusCode
MetadataView_filter(const MetadataView *input, const QueryOptions *options, MetadataView *out_view)
{
    out_view->items = NULL;
    out_view->count = 0;

    if (input->count == 0)
    {
        return OK;
    }

    out_view->items = (TensorMetadata *) malloc(input->count * sizeof(TensorMetadata));
    if (out_view->items == NULL)
    {
        printerr("Error allocating metadata filtered list\n");
        return ERROR;
    }

    for (size_t i = 0; i < input->count; i++)
    {
        const TensorMetadata *item = &input->items[i];
        if (!MetadataView_match_name(item, options) || !MetadataView_match_dtype(item, options)
            || !MetadataView_match_shape(item, options))
        {
            continue;
        }

        TensorMetadata *target = &out_view->items[out_view->count];
        MetadataView_copy_item(item, target);
        out_view->count += 1;
    }

    if (out_view->count == 0)
    {
        free(out_view->items);
        out_view->items = NULL;
    }

    return OK;
}

void
MetadataView_sort(MetadataView *view, QuerySortBy sort_by)
{
    if (view->count < 2)
    {
        return;
    }

    g_sort_by = sort_by;
    qsort(view->items, view->count, sizeof(TensorMetadata), MetadataView_qsort_compare);
}

void
MetadataView_free(MetadataView *view)
{
    for (size_t i = 0; i < view->count; i++)
    {
        free(view->items[i].name);
        free(view->items[i].shape);
    }

    free(view->items);
    view->items = NULL;
    view->count = 0;
}

const char *
MetadataView_dtype_to_string(enum Dtype dtype)
{
    return Safetensors_dtype_to_string(dtype);
}
