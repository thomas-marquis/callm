PROGRAM = callm
SOURCES = main.c safetensors.c tensor.c types.c matrix.c
OBJECTS = $(SOURCES:.c=.o)
ASM = $(SOURCES:.c=.s)
I_FILES = $(SOURCES:.c=.i)
CC = gcc
CFLAGS = -Wall
LDFLAGS = -ljansson

all: _run | $(PROGRAM)

run: _run | clean

_run: $(PROGRAM)
	./$(PROGRAM)

$(PROGRAM): $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	@rm -f $(OBJECTS) $(PROGRAM) $(ASM) $(I_FILES)
