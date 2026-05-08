add_executable(test_bf16_basic "test_bf16_basic.c")
target_link_libraries(test_bf16_basic PRIVATE callm_bf16 unity m)
add_test(NAME bf16_basic COMMAND test_bf16_basic)

add_executable(test_bf16_arithmetic "test_bf16_arithmetic.c")
target_link_libraries(test_bf16_arithmetic PRIVATE callm_bf16 unity m)
add_test(NAME bf16_arithmetic COMMAND test_bf16_arithmetic)

add_executable(test_bf16_comparison "test_bf16_comparison.c")
target_link_libraries(test_bf16_comparison PRIVATE callm_bf16 unity m)
add_test(NAME bf16_comparison COMMAND test_bf16_comparison)

add_executable(test_bf16_cpu "test_bf16_cpu.c")
target_link_libraries(test_bf16_cpu PRIVATE callm_bf16 unity m)
add_test(NAME bf16_cpu COMMAND test_bf16_cpu)

add_executable(test_bf16_simd "test_bf16_simd.c")
target_link_libraries(test_bf16_simd PRIVATE callm_bf16 unity m)
add_test(NAME bf16_simd COMMAND test_bf16_simd)

add_executable(test_bf16_special "test_bf16_special.c")
target_link_libraries(test_bf16_special PRIVATE callm_bf16 unity m)
add_test(NAME bf16_special COMMAND test_bf16_special)

add_executable(test_bf16_rounding "test_bf16_rounding.c")
target_link_libraries(test_bf16_rounding PRIVATE callm_bf16 unity m)
add_test(NAME bf16_rounding COMMAND test_bf16_rounding)
