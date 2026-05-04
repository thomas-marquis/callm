#include "unity.h"

#include "../../../src/safeparser/query_options.h"
#include <string.h>

void
setUp(void)
{
}

void
tearDown(void)
{
}

void
test_should_parse_help_option()
{
    char *argv[] = { "safeparser", "--help" };
    QueryOptions options;

    TEST_ASSERT_EQUAL(OK, QueryOptions_parse(2, argv, &options));
    TEST_ASSERT_EQUAL_INT(1, options.show_help);

    QueryOptions_free(&options);
}

void
test_should_fail_when_missing_file_path()
{
    char *argv[] = { "safeparser" };
    QueryOptions options;

    TEST_ASSERT_EQUAL(ERROR, QueryOptions_parse(1, argv, &options));

    QueryOptions_free(&options);
}

void
test_should_fail_on_invalid_sort_key()
{
    char *argv[] = { "safeparser", "--sort-by", "weight", "model.safetensors" };
    QueryOptions options;

    TEST_ASSERT_EQUAL(ERROR, QueryOptions_parse(4, argv, &options));

    QueryOptions_free(&options);
}

void
test_should_fail_on_invalid_shape_filter()
{
    char *argv[] = { "safeparser", "--filter-shape", "4,,8", "model.safetensors" };
    QueryOptions options;

    TEST_ASSERT_EQUAL(ERROR, QueryOptions_parse(4, argv, &options));

    QueryOptions_free(&options);
}

void
test_should_parse_combined_filters_and_sort()
{
    char *argv[] = { "safeparser",     "--filter-name", "layer.weight", "--filter-shape", "4096,11008",
                     "--filter-dtype", "BF16",          "--sort-by",    "dtype",          "model.safetensors" };
    QueryOptions options;

    TEST_ASSERT_EQUAL(OK, QueryOptions_parse(10, argv, &options));
    TEST_ASSERT_EQUAL_STRING("model.safetensors", options.file_path);

    TEST_ASSERT_EQUAL_INT(1, options.has_filter_name);
    TEST_ASSERT_EQUAL_STRING("layer.weight", options.filter_name);

    TEST_ASSERT_EQUAL_INT(1, options.has_filter_shape);
    TEST_ASSERT_EQUAL_UINT(2, options.filter_shape_size);
    TEST_ASSERT_EQUAL_INT(4096, options.filter_shape[0]);
    TEST_ASSERT_EQUAL_INT(11008, options.filter_shape[1]);

    TEST_ASSERT_EQUAL_INT(1, options.has_filter_dtype);
    TEST_ASSERT_EQUAL_INT(BF16, options.filter_dtype);

    TEST_ASSERT_EQUAL_INT(QUERY_SORT_BY_DTYPE, options.sort_by);

    QueryOptions_free(&options);
}

int
main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_should_parse_help_option);
    RUN_TEST(test_should_fail_when_missing_file_path);
    RUN_TEST(test_should_fail_on_invalid_sort_key);
    RUN_TEST(test_should_fail_on_invalid_shape_filter);
    RUN_TEST(test_should_parse_combined_filters_and_sort);
    return UNITY_END();
}
