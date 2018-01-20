CC = g++
DEBUG = -g -O0 -pedantic-errors
CFLAGS = -Wall -std=c++14 -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)

all: shell

shell: shell.o
	$(CC) $(LFLAGS) -o shell shell.o

shell.o: shell.cpp shell.h
	$(CC) $(CFLAGS) shell.cpp

.PHONY: clean

clean:
	rm -rf *.o
	rm -rf shell

