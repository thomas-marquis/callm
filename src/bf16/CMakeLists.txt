option(ENABLE_AVX2 "Enable AVX2 SIMD optimizations" ON)

set(BF16_SOURCES
    bf16_scalar.c
    bf16.c
)

if(ENABLE_AVX2)
    list(APPEND BF16_SOURCES bf16_simd.c)
endif()

add_library(bf16 STATIC ${BF16_SOURCES})
target_include_directories(bf16 PUBLIC include)

if(ENABLE_AVX2)
    target_compile_definitions(bf16 PRIVATE CALLM_ENABLE_AVX2)
    # Use -mavx2 for GCC/Clang
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(bf16 PRIVATE -mavx2)
    endif()
endif()
