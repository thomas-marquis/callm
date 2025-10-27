set(CALLM_CORE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/base64.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/bf16.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/linked_list.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/config.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/matrix.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/maths.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/matrix_view.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/safetensors.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/tensor.c"
        tensor.c)
set(CALLM_CORE_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/base64.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/bf16.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/json.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/linked_list.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/matrix.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/maths.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/matrix_view.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/safetensors.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/config.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/uthash.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/tensor.h")

add_subdirectory(memory)

add_library(callm_core STATIC ${CALLM_CORE_SOURCES} ${CALLM_CORE_HEADERS})
target_link_libraries(callm_core PUBLIC m jansson callm_memory callm_shared)
target_include_directories(callm_core PUBLIC "./")
