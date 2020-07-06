#include "../../GBK2UTF/GBKUTF.h"

int main()
{
    string inbuf = "通道名称通道名称通道名称"; //gbk
    printf("inbuf = %s\n", inbuf.c_str());

    string outbuf;
    if(is_str_gbk(inbuf))
    {
        outbuf = gbk2utf8(inbuf);  //utf8
        printf("utbuf = %s\n", outbuf.c_str());
    }
    else
        outbuf = inbuf;

    if(is_str_gbk(outbuf))
    {
        printf("--------\n");
        outbuf = gbk2utf8(outbuf);  //utf8
        printf("--- outbuf = %s\n", outbuf.c_str());
    }
    else
    {
        printf("+++++++++\n");
        outbuf = utf82gbk(outbuf);  //gbk
        printf("--- outbuf = %s\n", outbuf.c_str());
    }

    return 0;
}