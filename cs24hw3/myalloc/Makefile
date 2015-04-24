all: testunacceptable testmyalloc

CFLAGS=-g


clean: 
	rm -rf *.o *~ testunacceptable testmyalloc \
		testunacceptable.exe testmyalloc.exe

unacceptable_myalloc.o:	unacceptable_myalloc.c myalloc.h
sequence.o:	sequence.h sequence.c
myalloc.o:	myalloc.c myalloc.h
testalloc.o:	testalloc.c myalloc.h sequence.h

testunacceptable:	testalloc.o    unacceptable_myalloc.o sequence.o
	gcc -o testunacceptable testalloc.o unacceptable_myalloc.o sequence.o


testmyalloc:	testalloc.o    myalloc.o sequence.o
	gcc -o testmyalloc testalloc.o myalloc.o sequence.o
			



