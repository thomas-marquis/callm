set(CALLM_UTILS_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/errors.c"
                        "${CMAKE_CURRENT_SOURCE_DIR}/linked_list.c")
set(CALLM_UTILS_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/linked_list.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/errors.h" "${CMAKE_CURRENT_SOURCE_DIR}/json.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/logging.h")

add_library(callm_utils STATIC ${CALLM_UTILS_SOURCES} ${CALLM_UTILS_HEADERS})

target_include_directories(callm_utils PUBLIC "./")
