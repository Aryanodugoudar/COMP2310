# Set the C compiler to `gcc` (GNU Compiler Collection)
CC=gcc

# Settings to use for the compilation
CFLAGS=-Wall -Wconversion -g
EX2FLAGS=-Og -no-pie -fno-pic -fno-pie

all: ex1 ex2

ex1: src/ex1/song-lib.c
	"$(CC)" $(CFLAGS) -o $@ $^

ex2: ex2-student-exec ex2-solution-exec

ex2-student-exec: src/ex2
	"$(CC)" $(EX2FLAGS) -o driver-student $^/driver.c $^/hack.c

ex2-solution-exec: src/ex2
	"$(CC)" -c $(EX2FLAGS) $^/driver.c
	"$(CC)" -c $(EX2FLAGS) $^/hack-compiled.s
	"$(CC)" -o driver-solution $(EX2FLAGS) driver.o hack-compiled.o

.PHONY: clean
clean:
	rm ex1 *.o driveri driver-student driver-solution > /dev/null || true