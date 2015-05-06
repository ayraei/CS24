ASFLAGS = -g
CFLAGS = -g -O0 -Wall

all: smain

smain: smain.o screen.o drawing.o pixel.o
	$(CC) smain.o screen.o drawing.o pixel.o -o smain

clean:
	rm -f *.o *~ smain smain.exe

