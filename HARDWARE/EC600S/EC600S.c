#include "ec600s.h"
#include "usart.h"
#include "stdlib.h"
#include "string.h"
#include "wdg.h"
#include "delay.h"


//char Timestr[100];//时间字符串
extern char RxBuffer[100];
extern char RxCounter;


void  EC600S_Init(void)
{
    char *strx = NULL, *extstrx; 	//返回值指针判断

    Uart1_SendStr("Init EC600S ... \r\n");

    printf("AT\r\n");
    delay_ms(500);

    printf("AT\r\n");
    delay_ms(500);


    strx=strstr((const char*)RxBuffer,(const char*)"OK");//返回OK
    while(strx==NULL)
    {
        Clear_Buffer();
        printf("AT\r\n");
        delay_ms(500);
        strx=strstr((const char*)RxBuffer,(const char*)"OK");//返回OK
    }

    printf("ATE0\r\n"); //关闭回显
    delay_ms(500);
    Clear_Buffer();
    printf("AT+CSQ\r\n"); //检查CSQ
    delay_ms(500);
    /////////////////////////////////
    printf("AT+CPIN?\r\n");//检查SIM卡是否在位
    delay_ms(500);
    strx=strstr((const char*)RxBuffer,(const char*)"+CPIN: READY");//查看是否返回ready
    while(strx==NULL)
    {
        Clear_Buffer();
        printf("AT+CPIN?\r\n");
        delay_ms(500);
        strx=strstr((const char*)RxBuffer,(const char*)"+CPIN: READY");//检查SIM卡是否在位，等待卡在位，如果卡识别不到，剩余的工作就没法做了
    }
    Clear_Buffer();
    ///////////////////////////////////
    printf("AT+QMTCLOSE=0\r\n");//断开MQTT连接
    delay_ms(500);
    ///////////////////////////////////
    printf("AT+CREG?\r\n");//查看是否注册GSM网络
    delay_ms(500);
    strx=strstr((const char*)RxBuffer,(const char*)"+CREG: 0,1");//返回正常
    extstrx=strstr((const char*)RxBuffer,(const char*)"+CREG: 0,5");//返回正常，漫游
    while(strx==NULL&&extstrx==NULL)
    {
        Clear_Buffer();
        printf("AT+CREG?\r\n");//查看是否注册GSM网络
        delay_ms(500);
        strx=strstr((const char*)RxBuffer,(const char*)"+CREG: 0,1");//返回正常
        extstrx=strstr((const char*)RxBuffer,(const char*)"+CREG: 0,5");//返回正常，漫游
    }
    Clear_Buffer();
    /////////////////////////////////////
    printf("AT+CGREG?\r\n");//查看是否注册GPRS网络
    delay_ms(500);
    strx=strstr((const char*)RxBuffer,(const char*)"+CGREG: 0,1");//，这里重要，只有注册成功，才可以进行GPRS数据传输。
    extstrx=strstr((const char*)RxBuffer,(const char*)"+CGREG: 0,5");//返回正常，漫游
    while(strx==NULL&&extstrx==NULL)
    {
        Clear_Buffer();
        printf("AT+CGREG?\r\n");//查看是否注册GPRS网络
        delay_ms(500);
        strx=strstr((const char*)RxBuffer,(const char*)"+CGREG: 0,1");//，这里重要，只有注册成功，才可以进行GPRS数据传输。
        extstrx=strstr((const char*)RxBuffer,(const char*)"+CGREG: 0,5");//返回正常，漫游
    }
    Clear_Buffer();
    printf("AT+QIACT=1\r\n");//激活
    delay_ms(500);
    Clear_Buffer();
    printf("AT+QIACT?\r\n");//获取当前卡的IP地址
    delay_ms(500);
    Clear_Buffer();
}


void  MQTT_Init(void)
{
    char *strx = NULL; 	//返回值指针判断

    printf("AT+QMTOPEN=0,\"47.92.146.210\",1883\r\n");//通过TCP方式去连接MQTT服务器
    delay_ms(500);
    strx=strstr((const char*)RxBuffer,(const char*)"+QMTOPEN: 0,0");//看下返回状态
    while(strx==NULL)
    {
        strx=strstr((const char*)RxBuffer,(const char*)"+QMTOPEN: 0,0");//确认返回值正确
    }
    Clear_Buffer();
    printf("AT+QMTCONN=0,\"clientExample\",\"username\",\"password\"\r\n");//去登录MQTT服务器，设备ID，账号和密码.用户根据实际需要进行更改
    delay_ms(500);
    strx=strstr((const char*)RxBuffer,(const char*)"+QMTCONN: 0,0,0");//看下返回状态
    while(strx==NULL)
    {
        strx=strstr((const char*)RxBuffer,(const char*)"+QMTCONN: 0,0,0");//看下返回状态
    }
    Clear_Buffer();
    printf("AT+QMTSUB=0,1,\"MZH_EC600S\",0\r\n");//订阅个主题
    delay_ms(500);
    strx=strstr((const char*)RxBuffer,(const char*)"+QMTSUB: 0,1,0,0");//订阅成功
    while(strx==NULL)
    {
        strx=strstr((const char*)RxBuffer,(const char*)"+QMTSUB: 0,1,0,0");//订阅成功
    }
    Clear_Buffer();
}

void Send_SMS(char *phone,char *data)//发送短信
{
    char *strx=0; 	//返回值指针判断
    printf("AT+CMGF=1\r\n");//设置文本模式
    delay_ms(500);
    printf("AT+CSCS=\042GSM\042\r\n");   //设置TE字符集
    delay_ms(500);
    printf("AT+CMGS=\"");
    printf(phone);//输入手机号码
    printf("\"\r\n");
    delay_ms(500);
    printf (data);//文本内容
    Clear_Buffer();	//清除上面的内容
    while((USART2->SR&0X40)==0);//
    USART2->DR = 0X1A;   //发送结束符
    delay_ms(500);
    while(1)
    {
        strx=strstr((const char*)RxBuffer,(const char*)"+CMGS");//返回CMGS表明短信发送成功
        if(strx)
            break;//跳出
    }
    Clear_Buffer();	//清除上面的内容
}

