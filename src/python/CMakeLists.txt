add_library(pycallm MODULE "${CMAKE_CURRENT_SOURCE_DIR}/_pycallm.c")
set_target_properties(pycallm PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(callm_core PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(callm_tokenizer PROPERTIES POSITION_INDEPENDENT_CODE ON)

python_extension_module(pycallm)
target_link_libraries(pycallm callm_core callm_tokenizer
                      jansson pcre2-8 pcre2-16 pcre2-32 pcre2-posix)

install(TARGETS pycallm LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
