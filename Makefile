PROGRAM = callm
SOURCES = main.c safetensors.c types.c matrix.c tokenizer/tokenizer.c lib/hash_table.c lib/linear_map.c
OBJECTS = $(SOURCES:.c=.o)
ASM = $(SOURCES:.c=.s)
I_FILES = $(SOURCES:.c=.i)
CC = gcc
CFLAGS = -Wall
LDFLAGS = -ljansson -lpcre
CMAKE_OPT= -DENABLE_TESTING=OFF

all: run

clean:
	@rm -f $(OBJECTS) $(PROGRAM) $(ASM) $(I_FILES)
	@rm -f graph.dot*

prepare:
	@rm -rf build
	@mkdir build

dependency:
	@rm -f graph.dot*
	@cmake -B ./build -S . --graphviz=graph.dot && dot -Tpng graph.dot -o graph.png

docs:
	@cd docs && doxygen
.PHONY: docs

build:
	@cmake -B ./build -S . $(CMAKE_OPT)
	@cd build && make
.PHONY: build

run: build

	@./build/callm
.PHONY: run


test: prepare
	@cmake -B ./build -S . -DENABLE_TESTING=ON
	@cd build && make
	@cd build/tests && make test
