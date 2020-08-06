#ifndef CPPTOOLKITS_WRAP_MALLOC_H
#define CPPTOOLKITS_WRAP_MALLOC_H

#include <stdlib.h>
#include <string>
#include <map>
#include <vector>
#include <cstdint>

#include "backtrace.h"
#include "CurrentThread.h"

using std::string;

struct original_malloc_ptr
{
    uint64_t size;
    void *ptr;
};

struct malloc_ptr_info
{
    original_malloc_ptr *ori_ptr;
    int tid;
    string backtrace;
};

#define MALLOC_OFFSET (sizeof(uint64_t))

std::map<const void *, malloc_ptr_info> malloc_info_map;

extern "C"
{
    void *__real_malloc(size_t size);
    void *__real_free(void *ptr);

    static inline void *save_malloc_info(const size_t &size)
    {
        original_malloc_ptr *ori_ptr = nullptr;
        ori_ptr =  (original_malloc_ptr *)__real_malloc(size + MALLOC_OFFSET);
        ori_ptr->size = size;
        ori_ptr->ptr = (uint64_t *)ori_ptr + MALLOC_OFFSET;

        malloc_ptr_info info{};
        info.ori_ptr = ori_ptr;
        info.tid = CurrentThread::gettid();
        Backtrace::DumpStackTraceToString(&(info.backtrace), 2);
        malloc_info_map[info.ori_ptr->ptr] = info;

        return info.ori_ptr->ptr;
    }

    static inline void free_and_remove_malloc_info(const void *ptr)
    {
        if (nullptr == ptr)
        {
            printf("[%s: %d] | ptr is null\n", __FUNCTION__, __LINE__);
            return;
        }

        auto it = malloc_info_map.find(ptr);
        if(it != malloc_info_map.end())
        {
             auto info = it->second;
            malloc_info_map.erase(it);
            __real_free(info.ori_ptr);
        }
    }

    void *__wrap_malloc(size_t size)
    {
        return save_malloc_info(size);
    }

    void __wrap_free(void *ptr)
    {
        free_and_remove_malloc_info(ptr);
    }

    static inline void dump_malloc_info()
    {
        if(malloc_info_map.empty())
        {
            printf("------ All Malloced Mem Free -------\n");
            return;
        }

        printf("------ Dump Not Free Malloced Info -------\n");
        for (auto &it : malloc_info_map)
        {
             printf("Address: %p\n", it.first);
             printf("Size: %lu\n", it.second.ori_ptr->size);
             printf("Thread: %u\n", it.second.tid);
             printf("BackTrace: \n%s\n", it.second.backtrace.c_str());
             printf("----------------------------------\n");
        }
    }

} // extern "C"

#endif //CPPTOOLKITS_WRAP_MALLOC_H
