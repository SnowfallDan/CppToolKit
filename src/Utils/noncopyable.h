#ifndef MYUTILS_NONCOPYABLE_H
#define MYUTILS_NONCOPYABLE_H

class noncopyable
{
public:
    noncopyable(const noncopyable&) = delete;
    void operator=(const noncopyable&) = delete;

protected:
    noncopyable() = default;
    ~noncopyable() = default;
};

#endif //MYUTILS_NONCOPYABLE_H
