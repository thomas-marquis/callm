#include "unity.h"

#include "../../src/tokenizer/tokenizer.h"
#include <string.h>

Tokenizer *tokenizer;

void
setUp(void)
{
    tokenizer = Tokenizer_new("../../../../resources/tokenizer.model");
}

void
tearDown(void)
{
    Tokenizer_free(tokenizer);
}

void
test_should_encode_text_correctly(void)
{
    // Given
    char *input_str = "Je ne pense pas qu'il y ait de bonnes ou de mauvaises situations...";
    int *expected_tok_ids = (int[]){ 30854, 841,  73953, 6502, 934,   35329, 379,   107902, 409,
                                     7970,  4978, 6033,  409,  60014, 85,    22626, 15082,  1131 };

    // When
    int *actual_tok_ids;
    int token_count;
    Tokenizer_encode(tokenizer, input_str, &actual_tok_ids, &token_count);

    // Then
    char *msg = malloc(1024 * sizeof(char));
    if (msg == NULL)
    {
        perror("Failed to allocate memory for message");
        exit(EXIT_FAILURE);
    }

    sprintf(msg, "Token ids should match\nExpected: ");
    for (int i = 0; i < token_count; i++)
    {
        char buffer[16];
        sprintf(buffer, "%d ", expected_tok_ids[i]);
        strcat(msg, buffer);
    }
    strcat(msg, "\nActual: ");
    for (int i = 0; i < token_count; i++)
    {
        char buffer[16];
        sprintf(buffer, "%d ", actual_tok_ids[i]);
        strcat(msg, buffer);
    }

    TEST_ASSERT_EQUAL_INT_ARRAY_MESSAGE(expected_tok_ids, actual_tok_ids, token_count, msg);
}

int
main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_should_encode_text_correctly);
    return UNITY_END();
}
