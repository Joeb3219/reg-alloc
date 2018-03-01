CC := gcc
EXE_NAME := test

all: clean build

build: main.c
	$(CC) -g -o $(EXE_NAME) main.c

clean:
	rm -rf *.o $(EXE_NAME)
