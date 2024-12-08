set(CALLM_MATHS_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/bf16.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/safetensors.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/matrix.c")
set(CALLM_MATHS_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/bf16.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/safetensors.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/matrix.h")

add_library(callm_maths STATIC ${CALLM_MATHS_SOURCES} ${CALLM_MATHS_HEADERS})
target_include_directories(callm_maths PUBLIC "./")
