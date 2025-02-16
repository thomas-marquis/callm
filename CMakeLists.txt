cmake_minimum_required(VERSION 3.28)

project(
  CaLLM
  VERSION 0.1.0
  LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ENABLE_TESTING "Enable Test Builds" OFF)

include(AddGitSubmodule)
include(Docs)
include(CPM)

if(RELEASE_TYPE STREQUAL "PYTHON")
    message(STATUS "Building the project in python release mode")
    cmake_policy(SET CMP0148 OLD)
    if(SKBUILD)
        message(STATUS "The project is built using scikit-build")
        list(APPEND CMAKE_MODULE_PATH "${BASE_SKBUILD_LIB_PATH}/resources/cmake")
        find_package(PythonExtensions REQUIRED)
        # find_package(NumPy REQUIRED)
    endif()
endif()

add_git_submodule(external/jansson)
cpmaddpackage("gh:PCRE2Project/pcre2#pcre2-10.44")

include_directories(${JANSSON_INCLUDE_DIRS})

add_subdirectory(src)

if(RELEASE_TYPE STREQUAL "DEV")
  message(STATUS "Building the project in dev mode")
  cpmaddpackage("gh:ThrowTheSwitch/Unity#v2.6.0")
  include(CTest)
  enable_testing()

  add_subdirectory(tests)
endif()

