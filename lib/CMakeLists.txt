set(CALLM_LIB_SOURCES
    hash_table.c
    linear_map.c
    )

add_library(lib STATIC ${CALLM_LIB_SOURCES})

target_include_directories(lib PUBLIC "./")
