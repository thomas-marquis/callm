#include "unity.h"

#include "../../../src/safeparser/output.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

static char *
capture_output(const MetadataView *view, QueryOutputMode output_mode)
{
    fflush(stdout);

    int pipe_fds[2] = { -1, -1 };
    TEST_ASSERT_TRUE(pipe(pipe_fds) == 0);

    int saved_stdout_fd = dup(STDOUT_FILENO);
    TEST_ASSERT_TRUE(saved_stdout_fd >= 0);

    TEST_ASSERT_TRUE(dup2(pipe_fds[1], STDOUT_FILENO) >= 0);
    close(pipe_fds[1]);

    TEST_ASSERT_EQUAL(OK, Output_print(view, output_mode));

    fflush(stdout);
    TEST_ASSERT_TRUE(dup2(saved_stdout_fd, STDOUT_FILENO) >= 0);
    close(saved_stdout_fd);
    pipe_fds[1] = -1;

    char temp[256];
    size_t total_size = 0;
    char *buffer = (char *) malloc(1);
    TEST_ASSERT_NOT_NULL(buffer);
    buffer[0] = '\0';

    ssize_t read_count = 0;
    while ((read_count = read(pipe_fds[0], temp, sizeof(temp))) > 0)
    {
        char *new_buffer = (char *) realloc(buffer, total_size + (size_t) read_count + 1);
        TEST_ASSERT_NOT_NULL(new_buffer);
        buffer = new_buffer;

        memcpy(buffer + total_size, temp, (size_t) read_count);
        total_size += (size_t) read_count;
        buffer[total_size] = '\0';
    }
    TEST_ASSERT_TRUE(read_count == 0);
    close(pipe_fds[0]);

    return buffer;
}

void
test_should_preserve_existing_text_mode_format()
{
    MetadataView view;
    view.count = 1;
    view.items = (TensorMetadata *) malloc(sizeof(TensorMetadata));

    int shape[] = { 2, 4 };
    fill_item(&view.items[0], "layer.weight", BF16, shape, 2);

    char *output = capture_output(&view, QUERY_OUTPUT_MODE_TEXT);
    TEST_ASSERT_EQUAL_STRING("name=layer.weight shape=[2,4] dtype=BF16\n", output);

    free(output);
    free_fixture(&view);
}

void
test_should_print_csv_header_and_rows()
{
    MetadataView view;
    view.count = 2;
    view.items = (TensorMetadata *) malloc(view.count * sizeof(TensorMetadata));

    int shape0[] = { 2, 4 };
    int shape1[] = { 8 };
    fill_item(&view.items[0], "layer.weight", BF16, shape0, 2);
    fill_item(&view.items[1], "layer.bias", F32, shape1, 1);

    char *output = capture_output(&view, QUERY_OUTPUT_MODE_CSV);
    TEST_ASSERT_EQUAL_STRING("name,shape,dtype\nlayer.weight,\"2,4\",BF16\nlayer.bias,\"8\",F32\n", output);

    free(output);
    free_fixture(&view);
}

void
test_should_escape_csv_fields_with_commas_quotes_and_newlines()
{
    MetadataView view;
    view.count = 1;
    view.items = (TensorMetadata *) malloc(sizeof(TensorMetadata));

    int shape[] = { 1, 2 };
    fill_item(&view.items[0], "name,with\"quote\nandline", F32, shape, 2);

    char *output = capture_output(&view, QUERY_OUTPUT_MODE_CSV);
    TEST_ASSERT_EQUAL_STRING("name,shape,dtype\n\"name,with\"\"quote\nandline\",\"1,2\",F32\n", output);

    free(output);
    free_fixture(&view);
}

void
test_should_serialize_shape_as_single_csv_field()
{
    MetadataView view;
    view.count = 1;
    view.items = (TensorMetadata *) malloc(sizeof(TensorMetadata));

    int shape[] = { 3, 5, 7 };
    fill_item(&view.items[0], "tensor", BF16, shape, 3);

    char *output = capture_output(&view, QUERY_OUTPUT_MODE_CSV);
    TEST_ASSERT_EQUAL_STRING("name,shape,dtype\ntensor,\"3,5,7\",BF16\n", output);

    free(output);
    free_fixture(&view);
}

int
main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_should_preserve_existing_text_mode_format);
    RUN_TEST(test_should_print_csv_header_and_rows);
    RUN_TEST(test_should_escape_csv_fields_with_commas_quotes_and_newlines);
    RUN_TEST(test_should_serialize_shape_as_single_csv_field);
    return UNITY_END();
}
