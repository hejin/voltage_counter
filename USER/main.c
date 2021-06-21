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


char *strx=0,*Readystrx,*Errstrx; 	//����ֵָ���ж�

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

//�򴮿�1��ӡ����
void Uart1_SendStr(char *SendBuf)
{
    while (*SendBuf != '\0') {
        while((USART1->SR&0x40) == 0)
            ;//�ȴ��������

        USART1->DR = (u8) *SendBuf;
        SendBuf++;
    }
}



//��ջ���
void Clear_Buffer(void)
{
    u8 i;

    Uart1_SendStr((char*)RxBuffer);

    for(i = 0; i < 100; i++)
        RxBuffer[i]=0;//����

    RxCounter=0;
    IWDG_Feed();//ι��
}


//��EC600S���п���
void OPEN_EC600S(void)
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

//���Ͷ�ȡ��ѹֵ������
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
        } //�ȴ��������

        USART3->DR = cmdstr_for_chann_1[i];
    }
}

//�˴���ʵ�ֵĹ����ǽ�STM32��Ϊһ�����˹�����ʹ������1�����ݸ���Ƭ��
//��Ƭ�������ݷ�������2������2����BC20,BC20����������2������1��ͨ��
//STM32��������1���д�ӡ��ʹ�������·���ֱ�Ӵ���1����ģ��һ�����ܣ�
int main_voltage(void)
{
    u16 wait_count = 0;
    u16 ready_to_sendcmd = 1;
    char msg[128] = { 0 };

    delay_init();             //��ʱ������ʼ��
    NVIC_Configuration();     //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
    LED_Init();               //��ʼ����LED���ӵ�Ӳ���ӿ�
    EC600SCTR_Init();         //��ʼ��EC200U��PWR����

    uart_init(115200);        //����1��ʼ����������PC���д�ӡģ�鷵������
    Uart1_SendStr("hello, world!\r\n");

    uart3_init(115200);
    IWDG_Init(7, 625);        //8Sһ��
    //OPEN_EC600S();          //��EC600S����

    while(1)
    {

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

        IWDG_Feed();//ι��
    }
}

int main_sms(void)
{
    u8 sendstr[]="EC600S Send English Text!";//�����Լ���Ҫ���͵�Ӣ�Ķ������ݼ���
    u8 phone[11]="13601149321";//�����Լ����ֻ����뼴��
    delay_init();           //��ʱ������ʼ��
    NVIC_Configuration();   //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
    LED_Init();             //��ʼ����LED���ӵ�Ӳ���ӿ�
    uart_init(115200);      //����1��ʼ�����ӵ��Զ˴�ӡ������Ϣ
    uart2_init(115200);     //����2��ʼ�����Խ�EC600Sģ��
    delay_ms(500);
    PWRKEY=1;//ģ�鿪��
    EC600S_Init(); //EC600S��ʼ��
    Send_SMS(phone, sendstr);//���ŷ���
    Uart1_SendStr("hello, world! This is from my MCU\r\n");
    while(1)
    {
        delay_ms(500);
        Clear_Buffer();
    }
}


int main_mqtt(void)
{
    char sendstr[]="How are you, Hua? Greetings!";//�����Լ���Ҫ���͵�Ӣ�Ķ������ݼ���
    char phone[11]="13601149321";//�����Լ����ֻ����뼴��
    char sms[64];
    int i = 0;


    delay_init();            //��ʱ������ʼ��
    NVIC_Configuration();    //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
    LED_Init();              //��ʼ����LED���ӵ�Ӳ���ӿ�
    uart_init(115200);       //����1��ʼ�����ӵ��Զ˴�ӡ������Ϣ
    uart2_init(115200);      //����2��ʼ�����Խ�EC600Sģ��
    delay_ms(500);
    PWRKEY=1;//ģ�鿪��

    Uart1_SendStr("hello, world! from MCU\r\n");
    EC600S_Init(); //EC600S��ʼ��


    //Send_Text(phone, sendstr);//���ŷ���
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

//MQTTS  ������������:
/*
[16:30:42.532]???Init EC600S ... 

[16:30:44.029]???
OK

OK

OK

[16:30:45.028]???
+CSQ: 31,99

OK

+CPIN: READY

OK

[16:30:46.042]???
OK

+QMTCLOSE: 0,0

+CREG: 0,1

OK

[16:30:46.541]???
+CGREG: 0,1

OK

[16:30:47.056]???
ERROR

[16:30:47.555]???
+QIACT: 1,1,1,"10.2.198.114","240E:404:2500:9D0E:168A:86BE:913C:1849"

OK
Start to sending SMS

[16:30:55.269]???MQTTS Done

[16:30:56.271]???MQTTS

+QMTCFG: "version",(0-5),(3,4)
+QMTCFG: "pdpcid",(0-5),(1-15)
+QMTCFG: "ssl",(0-5),(0,1),(0-5)
+QMTCFG: "keepalive",(0-5),(0-3600)
+QMTCFG: "session",(0-5),(0,1)
+QMTCFG: "timeout",(0-5),(1-1200),(1-10),(0,1)
+QMTCFG: "will",(0-5),(0,1),(0-2),(0,1),"willtopic","willmessage"
+QMTCFG: "willex",(0-5),(0,1),(0-2),(0,1),"willtopic",(0-256)
+QMTCFG: "recv/mode",(0-5),(0,1),(0,1)
+QMTCFG: "send/mode",(0-5),(0,1)
+QMTCFG: "onenet",(0-5),"product id","access key"
+QMTCFG: "aliauth",(0-5),"product key","device name","device secret"
+QMTCFG: "hwauth",(0-5),"device id","device secret"
+QMTCFG: "hwprodid",(0-5),"product id","product secret","nodeid"
+QMTCFG: "qmtping",(0-5),(5-60)
+QMTCFG: "dataformat",(0-5),(0,1),(0,1)
+QMTCFG: "view/mode",(0-5),(0,1)
+QMTCFG: "CTWing",(0-5),"client_id","feature_string"
+QMTCFG: "edit/timeout",(0-5),(0,1),(1-180)

OK

OK

OK

OK

+QMTOPEN: 0,0

OK

+QMTCONN: 0,0,0

> 
OK

+QMTPUBEX: 0,0,0

OK

+QMTDISC: 0,0

[16:30:57.366]???MQTTS

[16:30:58.366]???MQTTS
*/
int main(void)
{
    u8 sendstr[]="How are you, Hua? Happy Dragon Boat!";//�����Լ���Ҫ���͵�Ӣ�Ķ������ݼ���
    u8 phone[11]="13601149321";//�����Լ����ֻ����뼴��
    int i = 0;

    delay_init();            //��ʱ������ʼ��
    NVIC_Configuration();    //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
    LED_Init();              //��ʼ����LED���ӵ�Ӳ���ӿ�
    uart_init(115200);       //����1��ʼ�����ӵ��Զ˴�ӡ������Ϣ
    uart2_init(115200);      //����2��ʼ�����Խ�EC600Sģ��
    delay_ms(500);
    PWRKEY=1;//ģ�鿪��
    EC600S_Init(); //EC600S��ʼ��

    Uart1_SendStr("Start to sending SMS\r\n");
//    Send_SMS(phone, sendstr);//���ŷ���


    printf("AT+QMTCFG=?\r\n");delay_ms(1000);

    printf("AT+QMTCFG=\"recv/mode\",0,0,1\r\n");delay_ms(1000);

    printf("AT+QMTCFG=\"SSL\",0,1,2\r\n");delay_ms(1000);

    printf("AT+QMTOPEN=0,\"mqtt.softxtream.app\",8883\r\n"); delay_ms(1000);

    printf("AT+QMTCONN=0,\"someclientID\",\"base_000\",\"iBATERRY668$\"\r\n"); delay_ms(10000);

    printf("AT+QMTPUBEX=0,0,0,0,\"testtopic0\",30\r\n");delay_ms(1000);
    printf("This is test data, from MQTTS.\r\n");delay_ms(1000);

//    printf("AT+QMTCLOSE=0\r\n"); delay_ms(1000);

    printf("AT+QMTDISC=0\r\n"); delay_ms(1000);


    Uart1_SendStr("MQTTS Done\r\n");
    while(1)
    {
        delay_ms(1000);
         Uart1_SendStr("MQTTS\r\n");
        Clear_Buffer();
    }
}