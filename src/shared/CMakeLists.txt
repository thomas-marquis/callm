set(CALLM_SHARED_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/errors.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/base64.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/linked_list.c")

add_library(callm_shared STATIC ${CALLM_SHARED_SOURCES})
target_link_libraries(callm_shared PUBLIC m jansson)
target_include_directories(callm_shared PUBLIC include)
