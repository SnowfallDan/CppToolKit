#include "../../GBK2UTF/GBKUTF.h"

int main()
{
    string inbuf = "adf £¡@#£¤%¡­¡­&¡Á£¨£©??+¡«ÖÐÎÄ²âÊÔ£¡ 1234567890\nabcdefghigklmnopqrstuvwxyz\rABCDEFGHIGKLMNOPQRSTUVWXYZ"; //gbk
    printf("--- inbuf = %s\n", inbuf.c_str());
    string outbuf;
    outbuf = gbk2utf8(inbuf);  //utf8
    printf("--- outbuf = %s\n", outbuf.c_str());

    if(outbuf.empty())
    {
        printf("error!");
        return -1;
    }

    auto outbuf2 = utf82gbk(outbuf); //gbk
    printf("--- outbuf2 = %s\n", outbuf2.c_str());

    if(outbuf2 == inbuf)
        printf("--- SUCCESS ---");

    return 0;
}