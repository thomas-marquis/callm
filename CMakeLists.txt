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

add_subdirectory(src)
