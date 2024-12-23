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

find_package(Python REQUIRED)

# include_directories("${PYTHON_INCLUDE_DIRS}")

add_git_submodule(external/jansson)
cpmaddpackage("gh:PCRE2Project/pcre2#pcre2-10.44")
if(ENABLE_TESTING)
  message(STATUS "Testing is enabled")
  cpmaddpackage("gh:ThrowTheSwitch/Unity#v2.6.0")
  include(CTest)
  enable_testing()
  add_subdirectory(tests)
endif()

include_directories(${JANSSON_INCLUDE_DIRS})

set(CALLM_ROOT_EXE "${CMAKE_CURRENT_SOURCE_DIR}/main.c")

add_subdirectory(src)

add_executable(callm ${CALLM_ROOT_EXE})

target_link_libraries(
  callm
  PUBLIC callm_utils
         callm_tokenizer
         callm_maths
         jansson
         pcre2-8
         pcre2-16
         pcre2-32
         pcre2-posix)

# set(CMAKE_BUILD_TYPE Debug) target_compile_options(callm PRIVATE -g)
if(ENABLE_TESTING)
  target_compile_options(callm PRIVATE -g)
endif()
