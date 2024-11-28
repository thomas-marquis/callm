cmake_minimum_required(VERSION 3.28)

project(CaLLM VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/")
include(AddGitSubmodule)

add_git_submodule(external/jansson)

include_directories(${JANSSON_INCLUDE_DIRS})

set(CALLM_ROOT_SOURCES
    types.c
    safetensors.c
    matrix.c
    )
set(CALLM_ROOT_EXE
    main.c
    )

add_library(root STATIC ${CALLM_ROOT_SOURCES})

add_subdirectory(lib)
add_subdirectory(tokenizer)

add_executable(callm ${CALLM_ROOT_EXE})

target_link_libraries(callm PUBLIC root vocab lib jansson)
