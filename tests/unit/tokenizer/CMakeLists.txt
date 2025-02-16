add_executable(callm_test_tokenizer_encode "${CMAKE_CURRENT_SOURCE_DIR}/test_tokenizer_encode.c")
target_link_libraries(callm_test_tokenizer_encode PRIVATE callm_tokenizer callm_core unity m jansson pcre2-8 pcre2-16 pcre2-32 pcre2-posix)
add_test(NAME test_tokenizer_encode COMMAND callm_test_tokenizer_encode)

