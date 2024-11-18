cmake_minimum_required(VERSION 3.28)

project(CaLLM VERSION 0.1.0 LANGUAGES C)

find_package(jansson REQUIRED)
include_directories(${JANSSON_INCLUDE_DIRS})

add_library(types STATIC types.c)
add_library(safetensors STATIC safetensors.c)
add_library(matrix STATIC matrix.c)
add_library(vocab STATIC tokenizer/vocab.c)
add_library(hash_table STATIC lib/hash_table.c)

add_executable(callm main.c)

target_link_libraries(callm PUBLIC types safetensors matrix vocab hash_table)
target_link_libraries(MyProject ${JANSSON_LIBRARIES})
