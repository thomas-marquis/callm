set(CALLM_MONITOR_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/probe.c")
set(CALLM_MONITOR_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/probe.h")

add_library(callm_monitor STATIC ${CALLM_MONITOR_SOURCES}
                                 ${CALLM_MONITOR_HEADERS})
target_link_libraries(callm_monitor PUBLIC callm_core callm_shared)

target_include_directories(callm_monitor PUBLIC "./")
