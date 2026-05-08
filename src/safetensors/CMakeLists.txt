set(CALLM_SAFETENSORS_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/safetensors.c")

add_library(callm_safetensors STATIC ${CALLM_SAFETENSORS_SOURCES})
target_link_libraries(callm_safetensors PUBLIC m callm_bf16 callm_shared)
target_include_directories(callm_safetensors PUBLIC include)