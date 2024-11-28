function(add_git_submodule dir)
    find_package(Git REQUIRED)

    if (NOT EXISTS ${CMAKE_SOURCE_DIR}/${dir}/CMakeLists.txt)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive -- ${CMAKE_SOURCE_DIR}/${dir}
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        )
    endif()

    if (EXISTS ${CMAKE_SOURCE_DIR}/${dir}/CMakeLists.txt)
        add_subdirectory(${CMAKE_SOURCE_DIR}/${dir})
        message(STATUS "Added submodule ${dir}")
    else()
        message(FATAL_ERROR "Submodule ${dir} not found")
    endif()
endfunction(add_git_submodule)
