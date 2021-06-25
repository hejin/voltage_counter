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
#include "time.h"


//����1
extern unsigned char uart1_getok;
extern unsigned char RxCounter1;
extern unsigned char RxBuffer1[255];

//����2
extern unsigned char uart2_getok;
extern unsigned char RxBuffer2[500];
extern unsigned int  RxCounter2;

//����3
extern unsigned char uart3_getok;
extern unsigned char RxCounter3;
extern unsigned char RxBuffer3[255];

struct vol_data
{
    unsigned int  ts;
    char          b0[255];
    char          b1[255];
};

static struct vol_data vd = { 0 };



//���⻹û�н������λ��timestamp
void get_time()
{
/*
    time_t now;
    struct tm *ts;
    char buf[80];
 
    now = time(NULL);
    ts = localtime (& now);
    strftime(buf, sizeof (buf), "% a% Y-% m-% d% H:% M:% S% Z", ts);
    sprintf(buf, "%u", now);
    Uart1_SendStr(buf);
*/
}


int system_init()
{
    delay_init();           //��ʱ������ʼ��
    NVIC_Configuration();   //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
    LED_Init();             //��ʼ����LED���ӵ�Ӳ���ӿ�
    uart_init(115200);      //����1��ʼ�����ӵ��Զ˴�ӡ������Ϣ
    uart2_init(115200);     //����2��ʼ�����Խ�EC600Sģ��
    uart3_init(115200);     //����3��ʼ�����Խӵ�ѹ����ģ��

    Uart1_SendStr("system init ...");
    delay_ms(500);
    PWRKEY=1;               //ģ�鿪��
    EC600S_Init();          //EC600S��ʼ��

    //IWDG_Init(7, 625);     //8Sһ��

    Uart1_SendStr("system init done");
    return 0;
}

//���Ͷ�ȡ��ѹֵ������
//��YSR1����YSR2���͵�����3���Ա��ȡ��ѹֵ
void Send_VoltCounterReadCmd(unsigned int chann)
{
    const char *cmdstr;
    char i;

    if (chann == 0)
        cmdstr = "YSR1\n";
    else
        cmdstr = "YSR2\n";

    for(i = 0; i < strlen(cmdstr); i++)
    {
        while ((USART3->SR&0X40) == 0) {
            /*no-op*/;
        }//�ȴ��������

        USART3->DR = cmdstr[i];
    }
}


//��ȡ��ѹ
int get_voltage(void)
{
    static u16 wait_count = 0;
    static u16 ready_to_sendcmd = 1;
    char msg[128] = { 0 };

    //the channel to read voltage:
    // 0 ==> channel 1
    // 1 ==> channel 2
    static unsigned int chann = 1;
    int got = 0;

    while(wait_count < 100)
    {

        ++wait_count;
        if (wait_count % 5 == 0)
             ready_to_sendcmd = 1;

        if (ready_to_sendcmd && (wait_count % 5 == 0)) {
            Send_VoltCounterReadCmd(chann);
            sprintf(msg, "ReadCMD sent. waiting for reply from channel: ## %u\r\n", chann+1); 
            Uart1_SendStr(msg);

            ready_to_sendcmd = 0;
        }

        delay_ms(100);//wait and check
        if (uart3_getok) {
            char *uV = strstr((const char*)RxBuffer3,(const char*)"uV");
            if (uV != NULL) {
                if (chann == 0) {
                    memset(vd.b0, 0, 255);
                    strncpy(vd.b0, RxBuffer3, uV - RxBuffer3 + 2);
                } else {
                    memset(vd.b1, 0, 255);
                    strncpy(vd.b1, RxBuffer3, uV - RxBuffer3 + 2);
                }
                snprintf(msg, 127, "successfully got voltage counter from channel ## %u: %s\r\n", 
                         chann+1, chann == 0 ? vd.b0 : vd.b1); 

                Uart1_SendStr(msg);

                RxCounter3       = 0;
                uart3_getok      = 0;
                ready_to_sendcmd = 1;
                wait_count       = 0;
                chann            = 1 - chann;  //�л�ȥ������һ��channel�ĵ�ѹֵ
                got              = 1;
            }
        }

        delay_ms(1000);

        sprintf(msg, "wakeup ...ready_to_sendcmd = %d wait_count=%d got=%u\r\n",
                ready_to_sendcmd, wait_count, got);
        Uart1_SendStr(msg);
        IWDG_Feed();//ι��
        if (got)
            break;
    }
    return got;
}


int send_sms(void)
{
    char sendstr[]="EC600S Send English Text!";//�����Լ���Ҫ���͵�Ӣ�Ķ������ݼ���
    char phone[11]="13601149321";//�����Լ����ֻ����뼴��

    Send_SMS(phone, sendstr);//���ŷ���
    Uart1_SendStr("hello, world! This is from my MCU\r\n");
}


int mqtt(void)
{

    Uart1_SendStr("hello, world! from MCU\r\n");


//  printf("AT+QMTCFG=?\r\n");delay_ms(1000);

    printf("AT+QMTCFG=\"recv/mode\",0,0\r\n");delay_ms(1000);

    printf("AT+QMTOPEN=0,\"mqtt.filesystems.io\",1883\r\n"); delay_ms(1000);

    printf("AT+QMTCONN=0,\"someclientID\",\"base_000\",\"iBATERRY668$\"\r\n"); delay_ms(10000);

    printf("AT+QMTPUBEX=0,0,0,0,\"testtopic0\",30\r\n");delay_ms(1000);
    printf("This is test data, hello MQTT.\r\n");delay_ms(1000);

    printf("AT+QMTCLOSE=0\r\n"); delay_ms(1000);

    printf("AT+QMTDISC=0\r\n"); delay_ms(1000);


    Uart1_SendStr("MQTT Done\r\n");

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
int mqtts_init()
{

    Uart1_SendStr("Start to sending msg\r\n");

    printf("AT+QMTCFG=?\r\n");delay_ms(1000); Clear_Buffer2();

    printf("AT+QMTCFG=\"recv/mode\",0,0,1\r\n");delay_ms(1000);Clear_Buffer2();

    printf("AT+QMTCFG=\"SSL\",0,1,2\r\n");delay_ms(1000);Clear_Buffer2();

    printf("AT+QMTOPEN=0,\"mqtt.softxtream.app\",8883\r\n"); delay_ms(1500);Clear_Buffer2();

    printf("AT+QMTCONN=0,\"someclientID\",\"base_000\",\"iBATERRY668$\"\r\n"); delay_ms(1500);Clear_Buffer2();

    Uart1_SendStr("MQTTS Init Done\r\n");
    return 0;
}

int mqtts_pub(char *mqtt_msg, int len)
{
    Uart1_SendStr("MQTTS pub Started\r\n");
    printf("AT+QMTPUBEX=0,0,0,0,\"testtopic0\",%u\r\n", len);delay_ms(1000);
    printf("%s", mqtt_msg);delay_ms(1000);
    Clear_Buffer2();
    Uart1_SendStr("MQTTS pub Done\r\n");
    return 0;
}

int mqtts_disc()
{
    Uart1_SendStr("MQTTS Disc\r\n");

    printf("AT+QMTDISC=0\r\n"); delay_ms(1000);

    Uart1_SendStr("MQTTS Disc Done\r\n");
    return 0;
}


enum {
    MTQQ_MSG_LEN = 256
}; 
char mtqq_msg_json[MTQQ_MSG_LEN];

int main()
{
    int sec = 0;
    int len;
    char msg[512];
    int ts = 12340000;

    system_init();
    send_sms();
    mqtts_init();
    get_voltage();
    get_voltage();
    get_voltage();

    while (1) {
        get_voltage();
        memset(mtqq_msg_json, 0, MTQQ_MSG_LEN);
        len = snprintf(mtqq_msg_json, MTQQ_MSG_LEN - 1, 
                       "{ \"ts\":\"%u\", \"b0\":\"%s\", \"b1\":\"%s\" }\r\n", 
                       ++ts, vd.b0, vd.b1);
        sprintf(msg, "msg=%s len=%u", mtqq_msg_json, len);
        Uart1_SendStr(msg);
        mqtts_pub(mtqq_msg_json, len);
        while(sec ++ < 60) {
            delay_ms(1000);
            sprintf(msg, "%u\r\n", sec);
            Uart1_SendStr(msg);
        }
        sec = 0;
    }
}
