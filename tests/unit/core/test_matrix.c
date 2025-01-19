#include "unity.h"
#include <math.h>

#include "../../src/core/matrix.h"

void
setUp(void)
{
}

void
tearDown(void)
{
}

void
test_matrix_equals()
{
    // Given
    Matrix *a = Matrix_new(3, 3);
    Matrix *b = Matrix_new(3, 3);
    Matrix *c = Matrix_new(3, 3);

    float *data1 = (float[]){ 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    float *data2 = (float[]){ 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    float *data3 = (float[]){ 1, 2, 3, 4, 10000, 6, 7, 8, 9 };

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
test_should_slice_by_columns()
{
    // Given
    Matrix *m = Matrix_new(3, 4);
    float *data = (float[]){ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
    Matrix_fill(m, data);

    Matrix *expected = Matrix_new(3, 2);
    Matrix_fill(expected, (float[]){ 2, 3, 6, 7, 10, 11 });

    // When & Then
    Matrix *res = Matrix_slice_column(m, 1, 2);
    TEST_ASSERT_EQUAL(1, Matrix_equals(res, expected));

    Matrix_free(m);
    Matrix_free(res);
    Matrix_free(expected);
}

void
test_seelct_matrix_columns(void)
{
    // Given
    Matrix *m = Matrix_new(3, 4);
    float *data = (float[]){ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
    Matrix_fill(m, data);

    Matrix *expected = Matrix_new(3, 2);
    Matrix_fill(expected, (float[]){ 2, 3, 6, 7, 10, 11 });

    // When
    Matrix *res = Matrix_select_columns(m, (int[]){ 1, 2 }, 2);

    // Then
    TEST_ASSERT_EQUAL(1, Matrix_equals(res, expected));

    Matrix_free(m);
    Matrix_free(res);
    Matrix_free(expected);
}
void
test_matrix_transpose(void)
{
    // Given
    Matrix *m = Matrix_new(2, 3);
    float *data = (float[]){ 1, 2, 3, 4, 5, 6 };
    Matrix_fill(m, data);

    Matrix *expected = Matrix_new(3, 2);
    Matrix_fill(expected, (float[]){ 1, 4, 2, 5, 3, 6 });

    // When
    Matrix *result = Matrix_transpose(m);

    // Then
    TEST_ASSERT_EQUAL(1, Matrix_equals(result, expected));

    Matrix_free(m);
    Matrix_free(result);
    Matrix_free(expected);
}

void
test_matrix_apply_softmax(void)
{
    // Given
    Matrix *m = Matrix_new(1, 3);
    float *data = (float[]){ 1.0, 2.0, 3.0 };
    Matrix_fill(m, data);

    Matrix *expected = Matrix_new(1, 3);
    float sum_exp = exp(1.0) + exp(2.0) + exp(3.0);
    Matrix_fill(expected, (float[]){ exp(1.0) / sum_exp, exp(2.0) / sum_exp, exp(3.0) / sum_exp });

    // When
    Matrix_apply_softmax(m);

    // Then
    TEST_ASSERT_EQUAL(1, Matrix_equals(m, expected));

    Matrix_free(m);
    Matrix_free(expected);
}

int
main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_matrix_equals);
    RUN_TEST(test_should_slice_by_columns);
    RUN_TEST(test_seelct_matrix_columns);
    RUN_TEST(test_matrix_transpose);
    RUN_TEST(test_matrix_apply_softmax);
    return UNITY_END();
}
