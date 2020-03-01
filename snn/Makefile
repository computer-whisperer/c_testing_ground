CC = gcc

CFLAGS = -Wall -c -g

## A list of options to pass to the linker. 
LDFLAGS = -Wall -g -lm

## Name the executable program, list source files
PROG = basicssn
SRCS = basicssn.c 

## Build the program from the object files (-o option)
$(PROG): basicssn.o
	$(CC) basicssn.o -o $(PROG) $(LDFLAGS)

basicssn.o: basicssn.c
	$(CC) $(CFLAGS) basicssn.c

## Remove all the compilation and debugging files
clean: 
	rm -f core $(PROG) basicssn.o *~
