#ifndef _UTF8_H
#define _UTF8_H

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

int utf8_to_gb(const char *src, char *dst, int len)
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

    char *outbuf = dst;
    iconv_t cd;
    cd = iconv_open("GBK", "UTF-8");
    if (cd != (iconv_t) -1)
    {
        ret = iconv(cd, &inbuf, &inlen, &outbuf, &outlen);
        if (ret != 0)
        {
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

unsigned char utf8_look_for_table[] =
        {
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
                2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
                3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
                4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 1, 1
        };
#define UTFLEN(x) utf8_look_for_table[(x)]
//根据首字节,获取utf8字符所占字节数
inline int get_utf8_char_byte_num(unsigned char ch)
{
    int byteNum = 0;
    if (ch >= 0xFC && ch < 0xFE)
        byteNum = 6;
    else if (ch >= 0xF8)
        byteNum = 5;
    else if (ch >= 0xF0)
        byteNum = 4;
    else if (ch >= 0xE0)
        byteNum = 3;
    else if (ch >= 0xC0)
        byteNum = 2;
    else if (0 == (ch & 0x80))
        byteNum = 1;
    return byteNum;
}

//判断字符串是否是utf8格式
int is_str_utf8(const char *str)
{
    int byteNum = 0;
    unsigned char ch;
    const char *ptr = str;
    if (NULL == str)
        return 0;

    while (*ptr != '\0')
    {
        ch = (unsigned char) *ptr;
        if (byteNum == 0) //根据首字节特性判断该字符的字节数
        {
            if (0 == (byteNum = get_utf8_char_byte_num(ch)))
                return 0;
        }
        else //多字节字符,非首字节格式:10xxxxxx
        {
            if ((ch & 0xC0) != 0x80)
                return 0;
        }

        byteNum--;
        ptr++;
    }
    if (byteNum > 0)
        return 0;
    return 1;
}

bool is_str_gbk(const string &strIn)
{
    if(is_str_utf8(strIn.c_str()))
        return false;
    unsigned char ch1;
    unsigned char ch2;

    if (strIn.size() >= 2)
    {
        ch1 = (unsigned char) strIn.at(0);
        ch2 = (unsigned char) strIn.at(1);
        return ch1 >= 129 && ch1 <= 254 && ch2 >= 64 && ch2 <= 254;
    }
    else return false;
}

string gbk2utf8(const string &in)
{
    if(is_str_utf8(in.c_str()))
        return in;
    string out;
    size_t inlen = in.size();
    size_t outlen = inlen * 10;

    auto outbuf = (char *) malloc(outlen);
    memset(outbuf, 0, outlen);
    if (gb_to_utf8(in.c_str(), outbuf, outlen) == 0)
        out = std::string(outbuf);

    free(outbuf);
    return std::move(out);
}

string utf82gbk(const string &in)
{
    string out;
    size_t inlen = in.size();
    size_t outlen = inlen * 10;

    auto outbuf = (char *) malloc(outlen);
    memset(outbuf, 0, outlen);
    if(utf8_to_gb(in.c_str(), outbuf, outlen) == 0)
        out = std::string(outbuf);

    free(outbuf);
    return std::move(out);
}

#endif
