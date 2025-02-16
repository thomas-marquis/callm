set(CALLM_LLM_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/embeddings.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/attention.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/decoder.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/model.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/rms_norm.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/rotary_embedding.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/mlp.c")
set(CALLM_LLM_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/embeddings.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/attention.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/decoder.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/model.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/rms_norm.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/rotary_embedding.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/mlp.h")

add_library(callm_llm STATIC ${CALLM_LLM_SOURCES} ${CALLM_LLM_HEADERS})
target_link_libraries(callm_llm PUBLIC callm_core)

target_include_directories(callm_llm PUBLIC "./")
