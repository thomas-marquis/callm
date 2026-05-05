set(CALLM_ARENA_SOURCES)
set(CALLM_ARENA_HEADERS
        "${CMAKE_CURRENT_SOURCE_DIR}/arena.h")

add_library(callm_arena STATIC ${CALLM_ARENA_SOURCES} ${CALLM_ARENA_HEADERS})
target_link_libraries(callm_arena PUBLIC callm_shared)