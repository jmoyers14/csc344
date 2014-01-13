# Makefile for CPE464 program 1

CC= gcc
CFLAGS= -g

# libraries are already in the link path.
LIBS = -lsox

all:   wreckingball

wreckingball: wreckingball.c
	$(CC) $(CFLAGS) -o wreckingball wreckingball.c -c -g $(LIBS)

clean:
	rm wreckingball



