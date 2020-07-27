#include "../../GBK2UTF/GBKUTF.h"

int main()
{
    string inbuf = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                   "<Response>\n"
                   "<CmdType>Catalog</CmdType>\n"
                   "<SN>2</SN>\n"
                   "<DeviceID>34020000001320000101</DeviceID>\n"
                   "<SumNum>1</SumNum>\n"
                   "<DeviceList Num=\"1\">\n"
                   "<Item>\n"
                   "<DeviceID>34020000001320000101</DeviceID>\n"
                   "<Name>a通道aa名称通道a名称asd通道名称a</Name>\n"
                   "<Manufacturer>Hikvision</Manufacturer>\n"
                   "<Model>IP Camera</Model>\n"
                   "<Owner>Owner</Owner>\n"
                   "<CivilCode>CivilCode</CivilCode>\n"
                   "<Address>Address</Address>\n"
                   "<Parental>0</Parental>\n"
                   "<SafetyWay>0</SafetyWay>\n"
                   "<RegisterWay>1</RegisterWay>\n"
                   "<Secrecy>0</Secrecy>\n"
                   "<Status>ON</Status>\n"
                   "</Item>\n"
                   "</DeviceList>\n"
                   "</Response>";
    //gbk
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
//        printf("--- outbuf = %s\n", outbuf.c_str());
    }
    else
    {
        printf("+++++++++\n");
        outbuf = utf82gbk(outbuf);  //gbk
//        printf("--- outbuf = %s\n", outbuf.c_str());
    }

    return 0;
}