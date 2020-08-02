#ifndef CPPTOOLKITS_STRINGUTILS_H
#define CPPTOOLKITS_STRINGUTILS_H

#include <string>

using std::string;

template<typename ...Args>
static inline string query_string_(const char *fmt, Args &&...arg)
{
    char *ptr_out = nullptr;
    asprintf(&ptr_out, fmt, arg...);
    if (ptr_out)
    {
        string ret(ptr_out);
        free(ptr_out);
        return ret;
    }
    return "";
}

template<typename ...Args>
static inline string query_string_(const string &fmt, Args &&...args)
{
    return queryString(fmt.data(), std::forward<Args>(args)...);
}

static inline const char *query_string_(const char *fmt)
{
    return fmt;
}

static inline const string &query_string_(const string &fmt)
{
    return fmt;
}

#endif //CPPTOOLKITS_STRINGUTILS_H
