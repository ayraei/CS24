#=============================================================================#
# makefile for building the fsum program.
#
# Caltech CS24 - Introduction to Computing Systems
# Spring 2011 - Donnie Pinkston (donnie@cs.caltech.edu)
#=============================================================================#

CFLAGS=-Wall

all: fsum

fsum: fsum.o ffunc.o
	gcc -o fsum fsum.o ffunc.o

clean:
	rm -f fsum *.o *~

.PHONY: all clean

