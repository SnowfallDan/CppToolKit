#include "../../GBK2UTF/GBKUTF.h"

int main()
{
    string inbuf = "²âÊÔÎÄµÄ"; //gbk
    printf("inbuf = %s\n", inbuf.c_str());
    string outbuf;
    auto flag = is_str_utf8(outbuf.c_str());
    printf("utf8 is_str_utf8 Flag = %d\n", flag);
    flag = is_str_gbk(outbuf.c_str());
    printf("gbk is_str_gbk Flag = %d\n", flag);
    outbuf = gbk2utf8(inbuf);  //utf8
    printf("utbuf = %s\n", outbuf.c_str());

    flag = is_str_gbk(outbuf.c_str());
    printf("gbk is_str_gbk Flag = %d\n", flag);
    if(is_str_gbk(outbuf.c_str()))
    {
        printf("--------\n");
        outbuf = gbk2utf8(outbuf);  //utf8
        printf("--- outbuf = %s\n", outbuf.c_str());
    }
    else
    {
        printf("++++++++\n");
        outbuf = utf82gbk(outbuf);  //gbk
        printf("--- outbuf = %s\n", outbuf.c_str());
    }

    return 0;
}