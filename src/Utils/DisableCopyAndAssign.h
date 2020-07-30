#ifndef MYUTILS_DISABLECOPYANDASSIGN_H
#define MYUTILS_DISABLECOPYANDASSIGN_H

// Disable copy construction and equal sign assignment operators for classes
// Because delete in C + + 11 is used, the macro definition does not need to be placed in private
#define DISABLE_COPY_AND_ASSIGN(CLASS_TYPE) \
    CLASS_TYPE(const CLASS_TYPE&) = delete; \
    CLASS_TYPE& operator=(const CLASS_TYPE&) = delete

#endif //MYUTILS_DISABLECOPYANDASSIGN_H
