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

static void
free_fixture(MetadataView *view)
{
    MetadataView_free(view);
}

void
test_should_sort_by_name_with_shape_and_dtype_tiebreakers()
{
    MetadataView view;
    view.count = 4;
    view.items = (TensorMetadata *) malloc(view.count * sizeof(TensorMetadata));

    int shape0[] = { 2 };
    int shape1[] = { 1, 3 };
    int shape2[] = { 1, 3 };
    int shape3[] = { 2, 1 };

    fill_item(&view.items[0], "b", F32, shape0, 1);
    fill_item(&view.items[1], "a", F32, shape1, 2);
    fill_item(&view.items[2], "a", BF16, shape2, 2);
    fill_item(&view.items[3], "a", BF16, shape3, 2);

    MetadataView_sort(&view, QUERY_SORT_BY_NAME);

    TEST_ASSERT_EQUAL_STRING("a", view.items[0].name);
    TEST_ASSERT_EQUAL_INT(BF16, view.items[0].dtype);
    TEST_ASSERT_EQUAL_INT(1, view.items[0].shape[0]);

    TEST_ASSERT_EQUAL_STRING("a", view.items[1].name);
    TEST_ASSERT_EQUAL_INT(F32, view.items[1].dtype);
    TEST_ASSERT_EQUAL_INT(1, view.items[1].shape[0]);

    TEST_ASSERT_EQUAL_STRING("a", view.items[2].name);
    TEST_ASSERT_EQUAL_INT(2, view.items[2].shape[0]);

    TEST_ASSERT_EQUAL_STRING("b", view.items[3].name);

    free_fixture(&view);
}

void
test_should_sort_by_dtype_with_name_tiebreaker()
{
    MetadataView view;
    view.count = 4;
    view.items = (TensorMetadata *) malloc(view.count * sizeof(TensorMetadata));

    int shape0[] = { 2 };
    int shape1[] = { 3 };
    int shape2[] = { 4 };
    int shape3[] = { 1 };

    fill_item(&view.items[0], "z", F32, shape0, 1);
    fill_item(&view.items[1], "b", BF16, shape1, 1);
    fill_item(&view.items[2], "a", BF16, shape2, 1);
    fill_item(&view.items[3], "c", F32, shape3, 1);

    MetadataView_sort(&view, QUERY_SORT_BY_DTYPE);

    TEST_ASSERT_EQUAL_INT(BF16, view.items[0].dtype);
    TEST_ASSERT_EQUAL_STRING("a", view.items[0].name);

    TEST_ASSERT_EQUAL_INT(BF16, view.items[1].dtype);
    TEST_ASSERT_EQUAL_STRING("b", view.items[1].name);

    TEST_ASSERT_EQUAL_INT(F32, view.items[2].dtype);
    TEST_ASSERT_EQUAL_STRING("c", view.items[2].name);

    TEST_ASSERT_EQUAL_INT(F32, view.items[3].dtype);
    TEST_ASSERT_EQUAL_STRING("z", view.items[3].name);

    free_fixture(&view);
}

void
test_should_sort_by_shape_with_name_tiebreaker()
{
    MetadataView view;
    view.count = 4;
    view.items = (TensorMetadata *) malloc(view.count * sizeof(TensorMetadata));

    int shape0[] = { 2 };
    int shape1[] = { 1, 4 };
    int shape2[] = { 1, 3 };
    int shape3[] = { 1, 3 };

    fill_item(&view.items[0], "d", BF16, shape0, 1);
    fill_item(&view.items[1], "b", F32, shape1, 2);
    fill_item(&view.items[2], "c", BF16, shape2, 2);
    fill_item(&view.items[3], "a", F32, shape3, 2);

    MetadataView_sort(&view, QUERY_SORT_BY_SHAPE);

    TEST_ASSERT_EQUAL_STRING("d", view.items[0].name);
    TEST_ASSERT_EQUAL_UINT(1, view.items[0].shape_size);

    TEST_ASSERT_EQUAL_STRING("a", view.items[1].name);
    TEST_ASSERT_EQUAL_INT(1, view.items[1].shape[0]);
    TEST_ASSERT_EQUAL_INT(3, view.items[1].shape[1]);

    TEST_ASSERT_EQUAL_STRING("c", view.items[2].name);
    TEST_ASSERT_EQUAL_INT(1, view.items[2].shape[0]);
    TEST_ASSERT_EQUAL_INT(3, view.items[2].shape[1]);

    TEST_ASSERT_EQUAL_STRING("b", view.items[3].name);

    free_fixture(&view);
}

int
main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_should_sort_by_name_with_shape_and_dtype_tiebreakers);
    RUN_TEST(test_should_sort_by_dtype_with_name_tiebreaker);
    RUN_TEST(test_should_sort_by_shape_with_name_tiebreaker);
    return UNITY_END();
}
