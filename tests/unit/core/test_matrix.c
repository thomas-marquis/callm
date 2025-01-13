#include "unity.h"

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
    Matrix *a = Matrix_new(3, 3);
    Matrix *b = Matrix_new(3, 3);
    Matrix *c = Matrix_new(3, 3);

    float *data1 = (float[]){ 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    float *data2 = (float[]){ 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    float *data3 = (float[]){ 1, 2, 3, 4, 10000, 6, 7, 8, 9 };

    Matrix_fill(a, data1);
    Matrix_fill(b, data2);
    Matrix_fill(c, data3);

    TEST_ASSERT_EQUAL(1, Matrix_equals(a, b));
    TEST_ASSERT_EQUAL(0, Matrix_equals(a, c));

    Matrix_free(a);
    Matrix_free(b);
    Matrix_free(c);
}

void
test_should_slice_by_columns()
{
    Matrix *m = Matrix_new(3, 3);
    float *data = (float[]){ 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    Matrix_fill(m, data);

    // TODO

    Matrix_free(m);
}

int
main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_matrix_equals);
    // RUN_TEST(test_should_slice_by_columns);
    return UNITY_END();
}
