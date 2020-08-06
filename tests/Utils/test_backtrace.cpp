#include "backtrace.h"

void func()
{
    string stacktrace;
    Backtrace::DumpStackTraceToString(&stacktrace);
    printf("stack trace: \n%s\n", stacktrace.c_str());
}

int main()
{
    func();
    return 0;
}