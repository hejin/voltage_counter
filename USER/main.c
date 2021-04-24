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
char *strx=0,*Readystrx,*Errstrx ; 	//返回值指针判断
extern unsigned char  RxBuffer[500];
extern unsigned int RxCounter;
extern unsigned char uart1_getok;
extern unsigned char uart2_getok;
extern char RxCounter1,RxBuffer1[100];
extern unsigned char Timeout,restflag;

extern volatile unsigned char uart3_getok;
extern char RxCounter2,RxBuffer2[100];


void Uart1_SendStr(char*SendBuf)//串口1打印数据
{
    while(*SendBuf)
    {
        while((USART1->SR&0X40)==0);//等待发送完成
        USART1->DR = (u8) *SendBuf;
        SendBuf++;
    }
}
void Clear_Buffer(void)//清空缓存
{
    u8 i;
    Uart1_SendStr((char*)RxBuffer);
    for(i=0; i<100; i++)
        RxBuffer[i]=0;//缓存
    RxCounter=0;
    IWDG_Feed();//喂狗

}


void Send_ATcmd(void)//发送AT指令给到模块，从串口1接收指令，串口2控制
{
    char i;
    for(i=0; i<RxCounter1; i++)
    {
        while((USART2->SR&0X40)==0) {} //等待发送完成
        USART2->DR = RxBuffer1[i];
    }
}

void  OPEN_EC600S(void)//对EC600S进行开机
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


void Send_VoltCounterReadCmd(void)
{
    const char *cmdstr_for_chann_1 = "YSR1\n";
    char i;
    for(i = 0; i < strlen(cmdstr_for_chann_1); i++)
    {
        while ((USART3->SR&0X40) == 0) {} //等待发送完成
        USART3->DR = cmdstr_for_chann_1[i];
    }
}

//此代码实现的功能是将STM32作为一个搬运工，信使，串口1发数据给单片机
//单片机把数据发给串口2，串口2发给BC20,BC20反馈给串口2，串口1再通过
//STM32发给串口1进行打印。使用起来仿佛是直接串口1控制模块一个感受！
int main(void)
{
    u16 wait_count = 0;
    u16 ready_to_sendcmd = 1;
    delay_init();	    	 //延时函数初始化
    NVIC_Configuration(); 	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
    LED_Init();		  		//初始化与LED连接的硬件接口
    EC600SCTR_Init();        //初始化EC200U的PWR引脚
    uart_init(115200);//串口1初始化，可连接PC进行打印模块返回数据
    Uart1_SendStr("hello, world!\r\n");
    //uart2_init(115200);//初始化和GPRS连接串口
    uart3_init(115200);
    IWDG_Init(7,625);    //8S一次
    //OPEN_EC600S();//对EC600S开机
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
        if (ready_to_sendcmd && wait_count ++ == 10) {
            Send_VoltCounterReadCmd();
            Uart1_SendStr("waiting to retrieve counter from channel#1 ... \r\n");
            wait_count       = 0;
            ready_to_sendcmd = 0;
        }

        if (uart3_getok) {
            int i;
            Uart1_SendStr("successfully got voltage counter from channel#1 : \r\n");
            for(i = 0; i < RxCounter2; i++)
                UART1_send_byte(RxBuffer2[i]);
            RxCounter2       = 0;
            uart3_getok      = 0;
            ready_to_sendcmd = 1;
        }

        //while (sleep_count ++ < 10)
        delay_ms(500);
        //Uart1_SendStr("wakeup ... Wow\r\n");

        IWDG_Feed();//喂狗
    }
}






