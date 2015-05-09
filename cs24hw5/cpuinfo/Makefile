# MacOS X flags for building for 32-bit Pentium Pro
ASFLAGS=-arch i686
CFLAGS=-arch i686

OBJS=cpuid.o cpuid_ext.o cpuinfo.o

all: cpuinfo

cpuinfo: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o cpuinfo $(LDFLAGS)

clean:
	rm -f $(OBJS) *~ cpuinfo

.PHONY: clean

