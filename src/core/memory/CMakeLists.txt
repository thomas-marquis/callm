set(CALLM_MEMORY_SOURCES
        block.c)
set(CALLM_MEMORY_HEADERS
        block.h)

add_library(callm_memory STATIC ${CALLM_MEMORY_SOURCES} ${CALLM_MEMORY_HEADERS})
target_link_libraries(callm_memory PUBLIC)
target_include_directories(callm_memory PUBLIC ".")
