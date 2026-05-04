set(CALLM_SAFEPARSER_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/query_options.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/metadata_view.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/output.c")

set(CALLM_SAFEPARSER_HEADERS
        "${CMAKE_CURRENT_SOURCE_DIR}/query_options.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/metadata_view.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/output.h")

add_library(callm_safeparser STATIC ${CALLM_SAFEPARSER_SOURCES} ${CALLM_SAFEPARSER_HEADERS})
target_link_libraries(callm_safeparser PUBLIC callm_core callm_shared jansson)
target_include_directories(callm_safeparser PUBLIC "./")

add_executable(safeparser "${CMAKE_CURRENT_SOURCE_DIR}/main.c")
target_link_libraries(safeparser PRIVATE callm_safeparser)
