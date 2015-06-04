CFLAGS = -Wall -g -O0

VMEM_OBJS = virtualmem.o vmalloc.o matrix.o test_matrix.o

# So that the binary programs can be listed in fewer places
#BINARIES = test_matrix test_matrix_fifo test_matrix_clru
BINARIES = test_matrix test_matrix_fifo


all: $(BINARIES)


test_matrix: $(VMEM_OBJS) vmpolicy_random.o
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@ $(LDFLAGS)

test_matrix_fifo: $(VMEM_OBJS) vmpolicy_fifo.o
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@ $(LDFLAGS)

test_matrix_clru: $(VMEM_OBJS) vmpolicy_clru.o
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -f *.o *~ $(BINARIES)


.PHONY: all clean

