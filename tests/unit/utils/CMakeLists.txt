add_executable(callm_test_hash_map "${CMAKE_CURRENT_SOURCE_DIR}/test_hash_map.c")
target_link_libraries(
  callm_test_hash_map
  PUBLIC callm_utils
         callm_tokenizer
         callm_maths
         jansson
         pcre2-8
         pcre2-16
         pcre2-32
         pcre2-posix)
target_link_libraries(callm_test_hash_map PRIVATE unity)
add_test(NAME callm_test_hash_map COMMAND callm_test_hash_map)
