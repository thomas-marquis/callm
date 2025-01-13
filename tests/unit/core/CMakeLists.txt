add_executable(callm_test_base64 "${CMAKE_CURRENT_SOURCE_DIR}/test_base64.c")
target_link_libraries(callm_test_base64 PRIVATE callm_core unity)
add_test(NAME test_base64 COMMAND callm_test_base64)

add_executable(callm_test_matrix "${CMAKE_CURRENT_SOURCE_DIR}/test_matrix.c")
target_link_libraries(callm_test_matrix PRIVATE callm_core unity)
add_test(NAME test_matrix COMMAND callm_test_matrix)
