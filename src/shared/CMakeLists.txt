set(CALLM_SHARED_SOURCES
    errors.c)
set(CALLM_SHARED_HEADERS
    errors.h
    logging.h
    utils.h)

add_library(callm_shared STATIC ${CALLM_SHARED_SOURCES} ${CALLM_SHARED_HEADERS})
target_link_libraries(callm_shared PUBLIC)
target_include_directories(callm_shared PUBLIC "./")
