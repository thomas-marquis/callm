add_executable(test_arena_basic test_arena_basic.c)
target_link_libraries(test_arena_basic PRIVATE callm_arena unity m)
add_test(NAME test_arena_basic COMMAND test_arena_basic)

add_executable(test_arena_config test_arena_config.c)
target_link_libraries(test_arena_config PRIVATE callm_arena unity m)
add_test(NAME test_arena_config COMMAND test_arena_config)

add_executable(test_arena_allocation test_arena_allocation.c)
target_link_libraries(test_arena_allocation PRIVATE callm_arena unity m)
add_test(NAME test_arena_allocation COMMAND test_arena_allocation)

add_executable(test_arena_access test_arena_access.c)
target_link_libraries(test_arena_access PRIVATE callm_arena unity m)
add_test(NAME test_arena_access COMMAND test_arena_access)

add_executable(test_arena_layout test_arena_layout.c)
target_link_libraries(test_arena_layout PRIVATE callm_arena unity m)
add_test(NAME test_arena_layout COMMAND test_arena_layout)

add_executable(test_arena_hints test_arena_hints.c)
target_link_libraries(test_arena_hints PRIVATE callm_arena unity m)
add_test(NAME test_arena_hints COMMAND test_arena_hints)

add_executable(test_arena_errors test_arena_errors.c)
target_link_libraries(test_arena_errors PRIVATE callm_arena unity m)
add_test(NAME test_arena_errors COMMAND test_arena_errors)
