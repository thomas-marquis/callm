set(CALLM_ROOT_EXE "${CMAKE_CURRENT_SOURCE_DIR}/main.c")

add_executable(callm ${CALLM_ROOT_EXE})

target_link_libraries(
  callm
  PUBLIC callm_core
         callm_tokenizer
         callm_llm
         jansson
         pcre2-8
         pcre2-16
         pcre2-32
         pcre2-posix)

# set(CMAKE_BUILD_TYPE Debug) target_compile_options(callm PRIVATE -g)
if(ENABLE_TESTING)
  target_compile_options(callm PRIVATE -g)
endif()
