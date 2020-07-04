#ifndef _UTF8_H
#define _UTF8_H

// example:
// ==================================
//    string inbuf = "adf ！@#￥%……&×（）——+～中文测试！ 1234567890\nabcdefghigklmnopqrstuvwxyz\rABCDEFGHIGKLMNOPQRSTUVWXYZ"; //gbk
//    string outbuf;
//    printf("--- inbuf = %s\n", inbuf.c_str());
//    outbuf = gbk2utf8(inbuf);  //utf8
//    printf("--- outbuf = %s\n", outbuf.c_str());
//
//    if(outbuf.empty())
//    {
//        printf("error!");
//        return -1;
//    }

#include <iconv.h>
#include <string>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <memory.h>

using std::string;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * GBK to UTF-8
 *
 * @param src [in]
 * @param dst [out]
 * @param len [in] The most bytes which starting at dst, will be written.
 */
int gb_to_utf8(const char *src, char *dst, int len)
{
    int ret = 0;
    size_t inlen = strlen(src) + 1;
    size_t outlen = len;

    // duanqn: The iconv function in Linux requires non-const char *
    // So we need to copy the source string
    char *inbuf = (char *) malloc(len);
    char *inbuf_hold = inbuf;   // iconv may change the address of inbuf
    // so we use another pointer to keep the address
    memcpy(inbuf, src, len);

    char *outbuf2 = NULL;
    char *outbuf = dst;
    iconv_t cd;

    // starkwong: if src==dst, the string will become invalid during conversion since UTF-8 is 3 chars in Chinese but GBK is mostly 2 chars
    if (src == dst)
    {
        outbuf2 = (char *) malloc(len);
        memset(outbuf2, 0, len);
        outbuf = outbuf2;
    }

    cd = iconv_open("UTF-8", "GBK");
    if (cd != (iconv_t) -1)
    {
        ret = iconv(cd, &inbuf, &inlen, &outbuf, &outlen);
        if (ret != 0)
        {
            printf("iconv failed err: %s\n", strerror(errno));
            return -1;
        }

        if (outbuf2 != NULL)
        {
            strcpy(dst, outbuf2);
            free(outbuf2);
        }

        iconv_close(cd);
    }
    else
        return -1;

    free(inbuf_hold);   // Don't pass in inbuf as it may have been modified

    return 0;
}

int utf8_to_gb(const char* src, char* dst, int len)
{
    int ret = 0;
    size_t inlen = strlen(src) + 1;
    size_t outlen = len;

    // duanqn: The iconv function in Linux requires non-const char *
    // So we need to copy the source string
    char* inbuf = (char *)malloc(len);
    char* inbuf_hold = inbuf;   // iconv may change the address of inbuf
    // so we use another pointer to keep the address
    memcpy(inbuf, src, len);

    char* outbuf = dst;
    iconv_t cd;
    cd = iconv_open("GBK", "UTF-8");
    if (cd != (iconv_t)-1) {
        ret = iconv(cd, &inbuf, &inlen, &outbuf, &outlen);
        if (ret != 0) {
            printf("iconv failed err: %s\n", strerror(errno));
            return -1;
        }

        iconv_close(cd);
    }
    free(inbuf_hold);   // Don't pass in inbuf as it may have been modified
}

#ifdef __cplusplus
}
#endif

int is_str_utf8(const char* str)
{
    int nBytes = 0;////UTF8可用1-6个字节编码,ASCII用一个字节
    unsigned char ch = 0;
    bool bAllAscii = true;//如果全部都是ASCII,说明不是UTF-8
    for (unsigned int i = 0; str[i] != '\0'; ++i)
    {
        ch = *(str + i);
        if ((ch & 0x80) != 0)
            bAllAscii = false;
        if(nBytes == 0)
        {
            if((ch & 0x80) != 0)
            {
                while((ch & 0x80) != 0)
                {
                    ch <<= 1;
                    nBytes ++;
                }
                if((nBytes < 2) || (nBytes > 6))
                {
                    return 0;
                }
                nBytes --;
            }
        }
        else
        {
            if((ch & 0xc0) != 0x80)
            {
                return 0;
            }
            nBytes --;
        }
        i ++;
    }
    if(bAllAscii)
        return false;
    return (nBytes == 0);
}

bool is_str_gbk(const char* str)
{
    if(is_str_utf8(str))
        return false;
    unsigned int nBytes = 0;//GBK可用1-2个字节编码,中文两个 ,英文一个
    unsigned char chr = *str;
    bool bAllAscii = true; //如果全部都是ASCII,
    for (unsigned int i = 0; str[i] != '\0'; ++i){
        chr = *(str + i);
        if ((chr & 0x80) != 0 && nBytes == 0){// 判断是否ASCII编码,如果不是,说明有可能是GBK
            bAllAscii = false;
        }
        if (nBytes == 0) {
            if (chr >= 0x80) {
                if (chr >= 0x81 && chr <= 0xFE){
                    nBytes = +2;
                }
                else{
                    return false;
                }
                nBytes--;
            }
        }
        else{
            if (chr < 0x40 || chr>0xFE){
                return false;
            }
            nBytes--;
        }//else end
    }
    if (nBytes != 0) {   //违返规则
        return false;
    }
    if (bAllAscii){ //如果全部都是ASCII, 也是GBK
        return true;
    }
    return true;
}

string gbk2utf8(const string &in)
{
    string out;
    // 不是gbk编码格式
    if(!is_str_gbk(in.c_str()))
        return out;

    size_t inlen = in.size();
    size_t outlen = inlen * 10;

    auto outbuf = (char *)malloc(outlen);
    memset(outbuf, 0, outlen);
    if(gb_to_utf8(in.c_str(), outbuf, outlen) == 0)
        out = std::string(outbuf);

    free(outbuf);
    return std::move(out);
}

string utf82gbk(const string &in)
{
    string out;
    size_t inlen = in.size();
    size_t outlen = inlen * 10;

    auto outbuf = (char *)malloc(outlen);
    memset(outbuf, 0, outlen);
    if(utf8_to_gb(in.c_str(), outbuf, outlen) == 0 && is_str_gbk(outbuf))
        out = std::string(outbuf);

    free(outbuf);
    return std::move(out);
}

#endif
