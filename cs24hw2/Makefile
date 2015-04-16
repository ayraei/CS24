all: factmain 

# perhaps use this as you develop further parts 
# of the assignment
all: factmain gcdmain 

factmain.o:	fact.h factmain.c
fact.o:	fact.s
gcdmain.o:	gcd.h gcdmain.c
gcd.o:	gcd.s


factmain:	fact.o factmain.o
	gcc -o factmain fact.o factmain.o

gcdmain:	gcd.o gcdmain.o
	gcc -o gcdmain gcd.o gcdmain.o


clean:
	-rm -f *.o
	-rm -f factmain factmain.exe gcdmain gcdmain.exe


.PHONY: all clean

