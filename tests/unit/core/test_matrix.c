#include "unity.h"
#include <math.h>

#include "../../src/core/matrix.h"

float
mock_times_tow_func(float x)
{
    return x * 2;
}

void
mock_add_max_func(float *x, int n)
{
    float max = 0;
    for (int i = 0; i < n; i++)
    {
        if (x[i] > max)
        {
            max = x[i];
        }
    }
    for (int i = 0; i < n; i++)
    {
        x[i] += max;
    }
}

float
mock_sum_func(float *x, int n)
{
    float sum = 0;
    for (int i = 0; i < n; i++)
    {
        sum += x[i];
    }
    return sum;
}

void
setUp()
{
}

void
tearDown()
{
}

void
test_mock_add_max_func()
{
    // Given
    float data[] = { 10, 2, 30, -1 };
    float expected[] = { 40, 32, 60, 29 };

    // When
    mock_add_max_func(data, 4);

    // Then
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(expected, data, 4);
}

void
test_matrix_equals()
{
    // Given
    Matrix *a = Matrix_new(3, 3);
    Matrix *b = Matrix_new(3, 3);
    Matrix *c = Matrix_new(3, 3);

    float data1[] = { 1, 2, 3,  //
                      4, 5, 6,  //
                      7, 8, 9 };
    float data2[] = { 1, 2, 3,  //
                      4, 5, 6,  //
                      7, 8, 9 };
    float data3[] = { 1, 2,     3,  //
                      4, 10000, 6,  //
                      7, 8,     9 };

    Matrix_fill(a, data1);
    Matrix_fill(b, data2);
    Matrix_fill(c, data3);

    // When & Then
    TEST_ASSERT_EQUAL(1, Matrix_equals(a, b));
    TEST_ASSERT_EQUAL(0, Matrix_equals(a, c));

    Matrix_free(a);
    Matrix_free(b);
    Matrix_free(c);
}

void
test_matrix_apply_each()
{
    // Given
    Matrix *m = Matrix_new(3, 3);
    float data[] = { 1, 2, 3,  //
                     4, 5, 6,  //
                     7, 8, 9 };
    Matrix_fill(m, data);

    Matrix *expected = Matrix_new(3, 3);
    float expected_data[] = { 2,  4,  6,   //
                              8,  10, 12,  //
                              14, 16, 18 };
    Matrix_fill(expected, expected_data);

    // When
    Matrix_apply_each(m, mock_times_tow_func);

    // Then
    TEST_ASSERT_EQUAL(1, Matrix_equals(m, expected));

    Matrix_free(m);
    Matrix_free(expected);
}

void
test_matrix_apply_along_row()
{
    // Given
    Matrix *m = Matrix_new(2, 3);
    float data[] = { 1, 2, 3,  //
                     4, 5, 6 };
    Matrix_fill(m, data);

    Matrix *expected = Matrix_new(2, 3);
    float expected_data[] = { 1 + 3, 2 + 3, 3 + 3,  //
                              4 + 6, 5 + 6, 6 + 6 };
    Matrix_fill(expected, expected_data);

    // When
    Matrix_apply_along(m, MAT_APPLY_ROW, mock_add_max_func);

    // Then
    TEST_ASSERT_EQUAL(1, Matrix_equals(m, expected));

    Matrix_free(m);
    Matrix_free(expected);
}

void
test_matrix_apply_along_col()
{
    // Given
    Matrix *m = Matrix_new(2, 3);
    float data[] = { 1, 2, 3,  //
                     4, 5, 6 };
    Matrix_fill(m, data);

    Matrix *expected = Matrix_new(2, 3);
    float expected_data[] = { 1 + 4, 2 + 5, 3 + 6,  //
                              4 + 4, 5 + 5, 6 + 6 };
    Matrix_fill(expected, expected_data);

    // When
    Matrix_apply_along(m, MAT_APPLY_COL, mock_add_max_func);

    // Then
    TEST_ASSERT_EQUAL(1, Matrix_equals(m, expected));

    Matrix_free(m);
    Matrix_free(expected);
}

void
test_matrix_should_reduce_along_rows()
{
    // Given
    Matrix *m = Matrix_new(3, 3);
    float data[] = { 1, 2, 3,  //
                     4, 5, 6,  //
                     7, 8, 9 };
    Matrix_fill(m, data);
    Matrix *expected = Matrix_new(3, 1);
    float expected_data[] = { 6,   //
                              15,  //
                              24 };
    Matrix_fill(expected, expected_data);

    // When
    Matrix *res = Matrix_reduce_along(m, MAT_APPLY_ROW, mock_sum_func);

    // Then
    TEST_ASSERT_EQUAL(1, Matrix_equals(res, expected));
    Matrix_free(m);
    Matrix_free(res);
    Matrix_free(expected);
}

void
test_should_slice_by_columns()
{
    // Given
    Matrix *m = Matrix_new(3, 4);
    float data[] = { 1, 2,  3,  4,  //
                     5, 6,  7,  8,  //
                     9, 10, 11, 12 };
    Matrix_fill(m, data);

    Matrix *expected = Matrix_new(3, 2);
    float expected_data[] = { 2,  3,  //
                              6,  7,  //
                              10, 11 };
    Matrix_fill(expected, expected_data);

    // When & Then
    Matrix *res = Matrix_slice_column(m, 1, 2);
    TEST_ASSERT_EQUAL(1, Matrix_equals(res, expected));

    Matrix_free(m);
    Matrix_free(res);
    Matrix_free(expected);
}

void
test_seelct_matrix_columns()
{
    // Given
    Matrix *m = Matrix_new(3, 4);
    float data[] = { 1, 2,  3,  4,  //
                     5, 6,  7,  8,  //
                     9, 10, 11, 12 };
    Matrix_fill(m, data);

    Matrix *expected = Matrix_new(3, 2);
    float expected_data[] = { 2,  3,  //
                              6,  7,  //
                              10, 11 };
    Matrix_fill(expected, expected_data);

    // When
    Matrix *res = Matrix_select_columns(m, (int[]){ 1, 2 }, 2);

    // Then
    TEST_ASSERT_EQUAL(1, Matrix_equals(res, expected));

    Matrix_free(m);
    Matrix_free(res);
    Matrix_free(expected);
}
void
test_matrix_transpose()
{
    // Given
    Matrix *m = Matrix_new(2, 3);
    float data[] = { 1, 2, 3,  //
                     4, 5, 6 };
    Matrix_fill(m, data);

    Matrix *expected = Matrix_new(3, 2);
    float expected_data[] = { 1, 4,  //
                              2, 5,  //
                              3, 6 };
    Matrix_fill(expected, expected_data);

    // When
    Matrix *result = Matrix_transpose(m);

    // Then
    TEST_ASSERT_EQUAL(1, Matrix_equals(result, expected));

    Matrix_free(m);
    Matrix_free(result);
    Matrix_free(expected);
}

void
test_matrix_multiply_each_element()
{
    // Given
    Matrix *A = Matrix_new(2, 3);
    float data[] = { 1, 2, 3,  //
                     4, 5, 6 };
    Matrix_fill(A, data);

    Matrix *B = Matrix_new(2, 3);
    float data2[] = { 2, 4, 2,  //
                      4, 2, 4 };
    Matrix_fill(B, data2);

    Matrix *expected = Matrix_new(2, 3);
    float expected_data[] = { 2,  8,  6,  //
                              16, 10, 24 };
    Matrix_fill(expected, expected_data);

    // When
    Matrix *result = Matrix_multiply(A, B);

    // Then
    TEST_ASSERT_EQUAL(1, Matrix_equals(result, expected));
    Matrix_free(A);
    Matrix_free(B);
    Matrix_free(result);
    Matrix_free(expected);
}

void
test_matrix_add_scalar()
{
    // Given
    Matrix *A = Matrix_new(2, 3);
    float data[] = { 1, 2, 3,  //
                     4, 5, 6 };
    Matrix_fill(A, data);

    Matrix *expected = Matrix_new(2, 3);
    float expected_data[] = { 11, 12, 13,  //
                              14, 15, 16 };
    Matrix_fill(expected, expected_data);

    // When
    Matrix *result = Matrix_add_scalar(A, 10);

    // Then
    TEST_ASSERT_EQUAL(1, Matrix_equals(result, expected));
    Matrix_free(A);
    Matrix_free(result);
    Matrix_free(expected);
}

void
test_matrix_multiply_scalar()
{
    // Given
    Matrix *A = Matrix_new(2, 3);
    float data[] = { 1, 2, 3,  //
                     4, 5, 6 };
    Matrix_fill(A, data);

    Matrix *expected = Matrix_new(2, 3);
    float expected_data[] = { 2, 4,  6,  //
                              8, 10, 12 };
    Matrix_fill(expected, expected_data);

    // When
    Matrix *result = Matrix_multiply_scalar(A, 2);

    // Then
    TEST_ASSERT_EQUAL(1, Matrix_equals(result, expected));
    Matrix_free(A);
    Matrix_free(result);
    Matrix_free(expected);
}

void
test_matrix_multiply_broadcast_row()
{
    // Given
    Matrix *A_small = Matrix_new(3, 1);
    float data[] = { 2,  //
                     4,  //
                     6 };
    Matrix_fill(A_small, data);

    Matrix *B_large = Matrix_new(3, 4);
    float data2[] = { 1, 1, 1, 1,  //
                      2, 2, 2, 2,  //
                      3, 3, 3, 3 };
    Matrix_fill(B_large, data2);

    Matrix *expected = Matrix_new(3, 4);
    float expected_data[] = { 2,  2,  2,  2,  //
                              8,  8,  8,  8,  //
                              18, 18, 18, 18 };
    Matrix_fill(expected, expected_data);

    // When
    Matrix *result = Matrix_multiply_broadcast(A_small, B_large, MAT_APPLY_ROW);

    // Then
    TEST_ASSERT_EQUAL(1, Matrix_equals(result, expected));
    Matrix_free(A_small);
    Matrix_free(B_large);
    Matrix_free(result);
    Matrix_free(expected);
}

void
test_matrix_multiply_broadcast_col()
{
    // Given
    Matrix *A_small = Matrix_new(1, 4);
    float data[] = { 2, 4, 6, 8 };
    Matrix_fill(A_small, data);

    Matrix *B_large = Matrix_new(3, 4);
    float data2[] = { 1, 1, 1, 1,  //
                      2, 2, 2, 2,  //
                      3, 3, 3, 3 };
    Matrix_fill(B_large, data2);

    Matrix *expected = Matrix_new(3, 4);
    float expected_data[] = { 2, 4,  6,  8,   //
                              4, 8,  12, 16,  //
                              6, 12, 18, 24 };

    Matrix_fill(expected, expected_data);

    // When
    Matrix *result = Matrix_multiply_broadcast(A_small, B_large, MAT_APPLY_COL);

    // Then
    TEST_ASSERT_EQUAL(1, Matrix_equals(result, expected));
    Matrix_free(A_small);
    Matrix_free(B_large);
    Matrix_free(result);
    Matrix_free(expected);
}

void
test_matrix_to_json(void)
{
    // Given
    Matrix *M = Matrix_new(2, 3);
    float data[] = { 1, 2, 3,  //
                     4, 5, 6 };
    Matrix_fill(M, data);

    const char *expected = "[[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]]";

    // When
    const char *json_str = Matrix_to_json(M);

    // Then
    TEST_ASSERT_EQUAL_STRING(expected, json_str);
}

int
main()
{
    UNITY_BEGIN();
    RUN_TEST(test_matrix_equals);
    RUN_TEST(test_should_slice_by_columns);
    RUN_TEST(test_seelct_matrix_columns);
    RUN_TEST(test_matrix_transpose);
    RUN_TEST(test_matrix_apply_each);
    RUN_TEST(test_mock_add_max_func);
    RUN_TEST(test_matrix_apply_along_row);
    RUN_TEST(test_matrix_apply_along_col);
    RUN_TEST(test_matrix_should_reduce_along_rows);
    RUN_TEST(test_matrix_multiply_each_element);
    RUN_TEST(test_matrix_multiply_scalar);
    RUN_TEST(test_matrix_add_scalar);
    RUN_TEST(test_matrix_multiply_broadcast_row);
    RUN_TEST(test_matrix_multiply_broadcast_col);
    RUN_TEST(test_matrix_to_json);
    return UNITY_END();
}
