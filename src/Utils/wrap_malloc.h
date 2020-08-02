#ifndef CPPTOOLKITS_WRAP_MALLOC_H
#define CPPTOOLKITS_WRAP_MALLOC_H

#ifdef ENABLE_TCMALLOC
#include <google/tcmalloc.h>
#else
#include <stdlib.h>
#endif

extern "C"
{
#ifndef ENABLE_TCMALLOC
    void *__real_malloc(size_t size);
    void *__real_free(size_t size);
#endif
    void *__wrap_malloc(size_t size)
    {
        printf("In wrap malloc");
#ifdef ENABLE_TCMALLOC
        return tc_malloc(size);
#else
        return __real_malloc(size);
#endif
    }

    void __wrap_free(void *ptr)
    {
        printf("In wrap free");
#ifdef ENABLE_TCMALLOC
        tc_free(ptr);
#else
        __real_free(ptr);
#endif
    }
} // extern "C"

#endif //CPPTOOLKITS_WRAP_MALLOC_H
