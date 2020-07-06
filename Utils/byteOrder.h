#ifndef MYUTILS_BYTEORDER_H
#define MYUTILS_BYTEORDER_H

bool byteOrder()
{
    union
    {
        short value;
        char union_char[sizeof(short)];
    } test;

    test.value = 0x0102;
    if(test.union_char[0] == 1 && test.union_char[1] == 2)
        return true;
    else if(test.union_char[1] == 1 && test.union_char[0] == 2)
        return false;
    else
        return false;
}


#endif //MYUTILS_BYTEORDER_H
