set(CALLM_ARENA_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/callm_arena.c")

add_library(callm_arena STATIC ${CALLM_ARENA_SOURCES})
target_link_libraries(callm_arena PUBLIC callm_shared callm_core)
target_include_directories(callm_arena PUBLIC include)