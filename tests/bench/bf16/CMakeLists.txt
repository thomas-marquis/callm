add_executable(bench_bf16 "bench_bf16.c")
target_link_libraries(bench_bf16 PRIVATE bf16 m)

# Pass AVX2 flag to benchmark if enabled in bf16 library
if (ENABLE_AVX2)
    target_compile_definitions(bench_bf16 PRIVATE CALLM_ENABLE_AVX2)
endif ()
