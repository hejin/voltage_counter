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
char *strx=0,*Readystrx,*Errstrx ; 	//����ֵָ���ж�
extern unsigned char  RxBuffer[500];
extern unsigned int RxCounter;
extern unsigned char uart1_getok;
extern unsigned char uart2_getok;
extern char RxCounter1,RxBuffer1[100];
extern unsigned char Timeout,restflag;

extern volatile unsigned char uart3_getok;
extern char RxCounter2,RxBuffer2[100];


void Uart1_SendStr(char*SendBuf)//����1��ӡ����
{
    while(*SendBuf)
    {
        while((USART1->SR&0X40)==0);//�ȴ��������
        USART1->DR = (u8) *SendBuf;
        SendBuf++;
    }
}
void Clear_Buffer(void)//��ջ���
{
    u8 i;
    Uart1_SendStr((char*)RxBuffer);
    for(i=0; i<100; i++)
        RxBuffer[i]=0;//����
    RxCounter=0;
    IWDG_Feed();//ι��

}


void Send_ATcmd(void)//����ATָ�����ģ�飬�Ӵ���1����ָ�����2����
{
    char i;
    for(i=0; i<RxCounter1; i++)
    {
        while((USART2->SR&0X40)==0) {} //�ȴ��������
        USART2->DR = RxBuffer1[i];
    }
}

void  OPEN_EC600S(void)//��EC600S���п���
{
    printf("AT\r\n");
    delay_ms(300);
    strx=strstr((const char*)RxBuffer,(const char*)"OK");//����OK
    printf("AT\r\n");
    delay_ms(300);
    strx=strstr((const char*)RxBuffer,(const char*)"OK");//����OK
    IWDG_Feed();//ι��
    if(strx==NULL)
    {
        PWRKEY=1;//����
        delay_ms(300);
        delay_ms(300);
        delay_ms(300);
        delay_ms(300);
        PWRKEY=0;//������������
        IWDG_Feed();//ι��
    }
    printf("AT\r\n");
    delay_ms(300);
    IWDG_Feed();//ι��
    strx=strstr((const char*)RxBuffer,(const char*)"OK");//����OK
    while(strx==NULL)
    {
        Clear_Buffer();
        printf("AT\r\n");
        delay_ms(300);
        LED1=!LED1;
        strx=strstr((const char*)RxBuffer,(const char*)"OK");//����OK
    }
    LED1=0;
    IWDG_Feed();//ι��
}


void Send_VoltCounterReadCmd(void)
{
    const char *cmdstr_for_chann_1 = "YSR1\n";
    char i;
    for(i = 0; i < strlen(cmdstr_for_chann_1); i++)
    {
        while ((USART3->SR&0X40) == 0) {} //�ȴ��������
        USART3->DR = cmdstr_for_chann_1[i];
    }
}

//�˴���ʵ�ֵĹ����ǽ�STM32��Ϊһ�����˹�����ʹ������1�����ݸ���Ƭ��
//��Ƭ�������ݷ�������2������2����BC20,BC20����������2������1��ͨ��
//STM32��������1���д�ӡ��ʹ�������·���ֱ�Ӵ���1����ģ��һ�����ܣ�
int main(void)
{
    u16 wait_count = 0;
    u16 ready_to_sendcmd = 1;
    delay_init();	    	 //��ʱ������ʼ��
    NVIC_Configuration(); 	 //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
    LED_Init();		  		//��ʼ����LED���ӵ�Ӳ���ӿ�
    EC600SCTR_Init();        //��ʼ��EC200U��PWR����
    uart_init(115200);//����1��ʼ����������PC���д�ӡģ�鷵������
    Uart1_SendStr("hello, world!\r\n");
    //uart2_init(115200);//��ʼ����GPRS���Ӵ���
    uart3_init(115200);
    IWDG_Init(7,625);    //8Sһ��
    //OPEN_EC600S();//��EC600S����
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

        IWDG_Feed();//ι��
    }
}






