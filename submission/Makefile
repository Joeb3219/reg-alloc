CC := gcc
EXE_NAME := alloc

all: clean build

build: main.c
	$(CC) -g -o $(EXE_NAME) main.c instr.c util.c

clean:
	rm -rf *.o $(EXE_NAME)
