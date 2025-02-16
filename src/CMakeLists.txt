add_subdirectory(core)
add_subdirectory(tokenizer)
add_subdirectory(llm)
add_subdirectory(main)

if(RELEASE_TYPE STREQUAL "PYTHON")
  add_subdirectory(python)
endif()
