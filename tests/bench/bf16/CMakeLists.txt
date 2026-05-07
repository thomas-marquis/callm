add_executable(bench_bf16 "bench_bf16.c")
target_link_libraries(bench_bf16 PRIVATE bf16 m)
