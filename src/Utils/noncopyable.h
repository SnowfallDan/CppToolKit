#ifndef CPPTOOLKITS_NONCOPYABLE_H
#define CPPTOOLKITS_NONCOPYABLE_H

class noncopyable
{
public:
    noncopyable(const noncopyable&) = delete;
    void operator=(const noncopyable&) = delete;

protected:
    noncopyable() = default;
    ~noncopyable() = default;
};

#endif //CPPTOOLKITS_NONCOPYABLE_H
