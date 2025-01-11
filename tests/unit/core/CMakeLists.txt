add_executable(callm_test_core "${CMAKE_CURRENT_SOURCE_DIR}/test_base64.c")
target_link_libraries(callm_test_core PUBLIC callm_core jansson pcre2-8
                                             pcre2-16 pcre2-32 pcre2-posix)
target_link_libraries(callm_test_core PRIVATE unity)
add_test(NAME callm_test_core COMMAND callm_test_core)
