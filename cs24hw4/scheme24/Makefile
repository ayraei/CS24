OBJS=ptr_vector.o values.o alloc.o parse.o special_forms.o \
	native_lambdas.o evaluator.o repl.o

CFLAGS=-Wall -g -O0
LDFLAGS=-lm

all:  scheme24


scheme24: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o scheme24


docs:
	doxygen


clean:
	rm -f *.gch *.o *~ scheme24
	rm -rf docs/html

.PHONY: all clean docs

