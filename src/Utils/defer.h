#ifndef CPPTOOLKITS_DEFER_H
#define CPPTOOLKITS_DEFER_H

#include <functional>
#include <vector>

#include "disableCopyAndAssign.h"

class Defer {
public:
    Defer() = delete;

    explicit Defer(const std::function<void()> &func)
        : defer_funcs_()
    {
        defer_funcs_.push_back(func);
    }

    ~Defer()
    {
        for (auto &func : defer_funcs_)
            func();
    }

    DISABLE_COPY_AND_ASSIGN(Defer);

    void push(const std::function<void()> &func)
    {
        defer_funcs_.push_back(func);
    }

private:
    // 强制对象只能初始化在栈上, 不能被new到堆上
    void * operator  new ( size_t  t) { return nullptr; }
    void  operator  delete ( void * ptr) {}

private:
    std::vector<std::function<void()>> defer_funcs_;
};

#endif //CPPTOOLKITS_DEFER_H
