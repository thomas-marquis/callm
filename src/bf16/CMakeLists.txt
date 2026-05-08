option(ENABLE_AVX2 "Enable AVX2 SIMD optimizations" ON)
if (NOT CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
    set(ENABLE_AVX2 OFF)
endif ()

set(CALLM_BF16_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/bf16_scalar.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/bf16.c")

if (ENABLE_AVX2)
    list(APPEND CALLM_BF16_SOURCES
            "${CMAKE_CURRENT_SOURCE_DIR}/bf16_simd.c")
endif ()

add_library(callm_bf16 STATIC ${CALLM_BF16_SOURCES})
target_include_directories(callm_bf16 PUBLIC include)

if (ENABLE_AVX2)
    target_compile_definitions(callm_bf16 PRIVATE CALLM_ENABLE_AVX2)
    if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(callm_bf16 PRIVATE -mavx2)
    endif ()
endif ()
