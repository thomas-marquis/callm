set(CALLM_TOKENIZER_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/tokenizer.c")
set(CALLM_TOKENIZER_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/tokenizer.h")

add_library(callm_tokenizer STATIC ${CALLM_TOKENIZER_SOURCES}
                                   ${CALLM_TOKENIZER_HEADERS})
target_include_directories(callm_tokenizer PUBLIC "./")
