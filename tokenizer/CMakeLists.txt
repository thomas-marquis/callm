set(CALLM_VOCAB_SOURCES
    tokenizer.c
    )

add_library(vocab STATIC ${CALLM_VOCAB_SOURCES})
target_include_directories(vocab PUBLIC "./")
