#ifndef CPPTOOLKITS_WRAP_MALLOC_H
#define CPPTOOLKITS_WRAP_MALLOC_H

#include <stdio.h>

#ifdef ENABLE_JEMALLOC
#include <jemalloc/jemalloc.h>
#else
#include <stdlib.h>
#endif

extern "C"
{
#ifndef ENABLE_JEMALLOC
    void *__real_malloc(size_t size);
    void *__real_free(size_t size);
#endif
    void *__wrap_malloc(size_t size)
    {
        printf("In wrap malloc");
#ifdef ENABLE_JEMALLOC
        return je_malloc(size);
#else
        return __real_malloc(size);
#endif
    }

    void __wrap_free(void *ptr)
    {
        printf("In wrap free");
#ifdef ENABLE_JEMALLOC
        je_free(ptr);
#else
        __real_free(ptr);
#endif
    }
} // extern "C"

#endif //CPPTOOLKITS_WRAP_MALLOC_H
