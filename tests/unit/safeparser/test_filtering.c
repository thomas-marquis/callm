#include "unity.h"

#include "../../../src/safeparser/metadata_view.h"

#include <stdlib.h>
#include <string.h>

void
setUp(void)
{
}

void
tearDown(void)
{
}

static void
fill_item(TensorMetadata *item, const char *name, enum Dtype dtype, const int *shape, size_t shape_size)
{
    item->name = (char *) malloc(strlen(name) + 1);
    strcpy(item->name, name);
    item->dtype = dtype;
    item->shape_size = shape_size;
    item->shape = (int *) malloc(shape_size * sizeof(int));
    memcpy(item->shape, shape, shape_size * sizeof(int));
}

static MetadataView
make_fixture_view(void)
{
    MetadataView view;
    view.count = 4;
    view.items = (TensorMetadata *) malloc(view.count * sizeof(TensorMetadata));

    int shape0[] = { 4096, 11008 };
    int shape1[] = { 4096, 11008 };
    int shape2[] = { 4096, 4096 };
    int shape3[] = { 11008 };

    fill_item(&view.items[0], "layer.0.weight", BF16, shape0, 2);
    fill_item(&view.items[1], "layer.1.weight", BF16, shape1, 2);
    fill_item(&view.items[2], "layer.1.bias", F32, shape2, 2);
    fill_item(&view.items[3], "token_embedding", F32, shape3, 1);

    return view;
}

void
test_should_filter_by_exact_name()
{
    MetadataView input = make_fixture_view();
    MetadataView output;
    QueryOptions options;
    QueryOptions_init(&options);

    options.has_filter_name = 1;
    options.filter_name = "layer.1.weight";

    TEST_ASSERT_EQUAL(OK, MetadataView_filter(&input, &options, &output));
    TEST_ASSERT_EQUAL_UINT(1, output.count);
    TEST_ASSERT_EQUAL_STRING("layer.1.weight", output.items[0].name);

    MetadataView_free(&output);
    MetadataView_free(&input);
    QueryOptions_free(&options);
}

void
test_should_filter_by_dtype_and_shape()
{
    MetadataView input = make_fixture_view();
    MetadataView output;
    QueryOptions options;
    QueryOptions_init(&options);

    int filter_shape[] = { 4096, 11008 };
    options.has_filter_dtype = 1;
    options.filter_dtype = BF16;
    options.has_filter_shape = 1;
    options.filter_shape = (int *) malloc(2 * sizeof(int));
    memcpy(options.filter_shape, filter_shape, 2 * sizeof(int));
    options.filter_shape_size = 2;

    TEST_ASSERT_EQUAL(OK, MetadataView_filter(&input, &options, &output));
    TEST_ASSERT_EQUAL_UINT(2, output.count);
    TEST_ASSERT_EQUAL_STRING("layer.0.weight", output.items[0].name);
    TEST_ASSERT_EQUAL_STRING("layer.1.weight", output.items[1].name);

    MetadataView_free(&output);
    MetadataView_free(&input);
    QueryOptions_free(&options);
}

void
test_should_apply_all_filters_with_and_semantics()
{
    MetadataView input = make_fixture_view();
    MetadataView output;
    QueryOptions options;
    QueryOptions_init(&options);

    int filter_shape[] = { 4096, 11008 };
    options.has_filter_name = 1;
    options.filter_name = "layer.1.weight";
    options.has_filter_dtype = 1;
    options.filter_dtype = BF16;
    options.has_filter_shape = 1;
    options.filter_shape = (int *) malloc(2 * sizeof(int));
    memcpy(options.filter_shape, filter_shape, 2 * sizeof(int));
    options.filter_shape_size = 2;

    TEST_ASSERT_EQUAL(OK, MetadataView_filter(&input, &options, &output));
    TEST_ASSERT_EQUAL_UINT(1, output.count);
    TEST_ASSERT_EQUAL_STRING("layer.1.weight", output.items[0].name);

    MetadataView_free(&output);
    MetadataView_free(&input);
    QueryOptions_free(&options);
}

int
main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_should_filter_by_exact_name);
    RUN_TEST(test_should_filter_by_dtype_and_shape);
    RUN_TEST(test_should_apply_all_filters_with_and_semantics);
    return UNITY_END();
}
