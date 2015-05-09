all: testmem heaptest apsptest qsorttest

CFLAGS=-O2
#CFLAGS=-g -O0


membase.o:	membase.c membase.h
memory.o:	memory.c memory.h membase.h
cache.o:	cache.c cache.h membase.h
cmdline.o:	cmdline.c cmdline.h membase.h memory.h cache.h

testmem.o:	testmem.c membase.h memory.h cache.h

heap.o:		heap.h membase.h
heaptest.o:	heap.h membase.h memory.h cache.h

apsptest.o:	membase.h memory.h cache.h

qsorttest.o:	membase.h memory.h cache.h

testmem: membase.o memory.o cache.o testmem.o
	gcc -o $@ $^

heaptest: membase.o memory.o cache.o cmdline.o heap.o heaptest.o
	gcc -o $@ $^

apsptest: membase.o memory.o cache.o cmdline.o apsptest.o
	gcc -o $@ $^

qsorttest: membase.o memory.o cache.o cmdline.o qsorttest.o
	gcc -o $@ $^

clean:
	-rm -f *.o testmem heaptest apsptest qsorttest

