#include "membase.h"

void usage(const char *progname);
membase_t * make_cached_memory(int argc, const char **argv,
                               unsigned int mem_size);
