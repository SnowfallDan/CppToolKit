#include "wrap_malloc.h"

void
do_something(size_t i)
{
    // Leak some memory.
    malloc(i * 100);
}

int main(int argc, char **argv)
{
    for (size_t i = 0; i < 10; i++)
        do_something(i);

    dump_malloc_info();

    return 0;
}