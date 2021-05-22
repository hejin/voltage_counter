#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "math.h"
#include "stdio.h"
#include "stm32f10x_flash.h"
#include "stdlib.h"
#include "string.h"
#include "wdg.h"
#include "EC600S.h"


char *strx=0,*Readystrx,*Errstrx; 	//返回值指针判断

extern unsigned char RxBuffer[500];
extern unsigned int  RxCounter;
extern unsigned char uart1_getok;
extern unsigned char uart2_getok;
extern unsigned char RxCounter1;
extern unsigned char RxBuffer1[100];
extern unsigned char Timeout;
extern unsigned char restflag;

extern volatile unsigned char uart3_getok;
extern unsigned char RxCounter2;
extern unsigned char RxBuffer2[100];

unsigned int chann = 1; //the channel to read voltage

//向串口1打印数据
void Uart1_SendStr(char *SendBuf)
{
    while (*SendBuf != '\0') {
        while((USART1->SR&0x40) == 0)
            ;//等待发送完成

        USART1->DR = (u8) *SendBuf;
        SendBuf++;
    }
}


//清空缓存
void Clear_Buffer(void)
{
    u8 i;

    Uart1_SendStr((char*)RxBuffer);

    for(i = 0; i < 100; i++)
        RxBuffer[i]=0;//缓存

    RxCounter=0;
    IWDG_Feed();//喂狗
}


//发送AT指令给到模块，从串口1接收指令，串口2控制
void Send_ATcmd(void)
{
    char i;
    for(i = 0; i < RxCounter1; i++) {
        while((USART2->SR&0x40)==0) {
            ;
        } //等待发送完成

        USART2->DR = RxBuffer1[i];
    }
}


//对EC600S进行开机
void OPEN_EC600S(void)
{
    printf("AT\r\n");
    delay_ms(300);
    strx=strstr((const char*)RxBuffer,(const char*)"OK");//返回OK
    printf("AT\r\n");
    delay_ms(300);
    strx=strstr((const char*)RxBuffer,(const char*)"OK");//返回OK
    IWDG_Feed();//喂狗
    if(strx==NULL)
    {
        PWRKEY=1;//拉低
        delay_ms(300);
        delay_ms(300);
        delay_ms(300);
        delay_ms(300);
        PWRKEY=0;//拉高正常开机
        IWDG_Feed();//喂狗
    }
    printf("AT\r\n");
    delay_ms(300);
    IWDG_Feed();//喂狗
    strx=strstr((const char*)RxBuffer,(const char*)"OK");//返回OK
    while(strx==NULL)
    {
        Clear_Buffer();
        printf("AT\r\n");
        delay_ms(300);
        LED1=!LED1;
        strx=strstr((const char*)RxBuffer,(const char*)"OK");//返回OK
    }
    LED1=0;
    IWDG_Feed();//喂狗
}

//发送读取电压值的命令
void Send_VoltCounterReadCmd(void)
{
    const char *cmdstr_for_chann_1;
    char i;

    if (chann == 0)
        cmdstr_for_chann_1 = "YSR1\n";
    else
        cmdstr_for_chann_1 = "YSR2\n";

    for(i = 0; i < strlen(cmdstr_for_chann_1); i++)
    {
        while ((USART3->SR&0X40) == 0) {
            ;
        } //等待发送完成

        USART3->DR = cmdstr_for_chann_1[i];
    }
}

//此代码实现的功能是将STM32作为一个搬运工，信使，串口1发数据给单片机
//单片机把数据发给串口2，串口2发给BC20,BC20反馈给串口2，串口1再通过
//STM32发给串口1进行打印。使用起来仿佛是直接串口1控制模块一个感受！
int main22(void)
{
    u16 wait_count = 0;
    u16 ready_to_sendcmd = 1;
    char msg[128] = { 0 };

    delay_init();             //延时函数初始化
    NVIC_Configuration();     //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
    LED_Init();               //初始化与LED连接的硬件接口
    EC600SCTR_Init();         //初始化EC200U的PWR引脚

    uart_init(115200);        //串口1初始化，可连接PC进行打印模块返回数据
    Uart1_SendStr("hello, world!\r\n");

    uart3_init(115200);
    IWDG_Init(7, 625);        //8S一次
    //OPEN_EC600S();          //对EC600S开机

    while(1)
    {
#if 0
        if(uart1_getok)
        {
            Send_ATcmd();
            uart1_getok=0;
            RxCounter1=0;

        }
        if(uart2_getok)
        {
            for(i=0; i<RxCounter; i++)
                UART1_send_byte(RxBuffer[i]);
            RxCounter=0;
            uart2_getok=0;

        }
#endif
        ++wait_count;
        if (wait_count % 5 == 0)
             ready_to_sendcmd = 1;

        if (ready_to_sendcmd && (wait_count % 5 == 0)) {
            Send_VoltCounterReadCmd();
            Uart1_SendStr("ReadCMD sent. waiting for reply from channel");
            if (chann == 0)
                Uart1_SendStr(" ##1 ...\r\n");
            else
                Uart1_SendStr(" ##2 ...\r\n");

            ready_to_sendcmd = 0;
        }

        delay_ms(100);//wait and check
        if (uart3_getok) {
            int i;

            Uart1_SendStr("successfully got voltage counter from channel");
            if (chann == 0)
                Uart1_SendStr(" ##1 ...\r\n");
            else
                Uart1_SendStr(" ##2 ...\r\n");

            for(i = 0; i < RxCounter2; i++)
                UART1_send_byte(RxBuffer2[i]);
            RxCounter2       = 0;
            uart3_getok      = 0;
            ready_to_sendcmd = 1;
            wait_count       = 0;
            chann            = 1 - chann;
        }


        delay_ms(1000);

        sprintf(msg, "wakeup ...ready_to_sendcmd = %d wait_count=%d\r\n",
                ready_to_sendcmd, wait_count);
        Uart1_SendStr(msg);

        IWDG_Feed();//喂狗
    }
}





int main(void)
{
    char sendstr[]="How are you, Hua? Greetings!";//输入自己需要发送的英文短信内容即可
    char phone[12]="13601149321";//输入自己的手机号码即可
    char sms[64];
    int i = 0;


    delay_init();            //延时函数初始化
    NVIC_Configuration();    //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
    LED_Init();              //初始化与LED连接的硬件接口
    uart_init(115200);       //串口1初始化，接电脑端打印调试信息
    uart2_init(115200);      //串口2初始化，对接EC600S模块
    delay_ms(500);
    PWRKEY=1;//模块开机
    EC600S_Init(); //EC600S初始化
    Uart1_SendStr("hello, world! from MCU\r\n");

    //Send_Text(phone, sendstr);//短信发送
    sprintf(sms, "SMS from MCU: %d\n\n", ++i);
    Uart1_SendStr(sms);
//  printf("AT+QMTCFG=?\r\n");delay_ms(1000);

    printf("AT+QMTCFG=\"recv/mode\",0,0\r\n");delay_ms(1000);

    printf("AT+QMTOPEN=0,\"mqtt.filesystems.io\",1883\r\n"); delay_ms(1000);

    printf("AT+QMTCONN=0,\"someclientID\",\"base_000\",\"iBATERRY668$\"\r\n"); delay_ms(10000);

    printf("AT+QMTPUBEX=0,0,0,0,\"testtopic0\",30\r\n");delay_ms(1000);
    printf("This is test data, hello MQTT.\r\n");delay_ms(1000);

    printf("AT+QMTCLOSE=0\r\n"); delay_ms(1000);

    printf("AT+QMTDISC=0\r\n"); delay_ms(1000);


    Uart1_SendStr("MQTT Done\r\n");
    while(1)
    {
        delay_ms(1000);
        Clear_Buffer();
    }
}
