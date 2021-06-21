#include "ec600s.h"
#include "usart.h"
#include "stdlib.h"
#include "string.h"
#include "wdg.h"
#include "delay.h"


//char Timestr[100];//ʱ���ַ���
extern char RxBuffer[100];
extern char RxCounter;


void  EC600S_Init(void)
{
    char *strx = NULL, *extstrx; 	//����ֵָ���ж�

    Uart1_SendStr("Init EC600S ... \r\n");

    printf("AT\r\n");
    delay_ms(500);

    printf("AT\r\n");
    delay_ms(500);


    strx=strstr((const char*)RxBuffer,(const char*)"OK");//����OK
    while(strx==NULL)
    {
        Clear_Buffer();
        printf("AT\r\n");
        delay_ms(500);
        strx=strstr((const char*)RxBuffer,(const char*)"OK");//����OK
    }

    printf("ATE0\r\n"); //�رջ���
    delay_ms(500);
    Clear_Buffer();
    printf("AT+CSQ\r\n"); //���CSQ
    delay_ms(500);
    /////////////////////////////////
    printf("AT+CPIN?\r\n");//���SIM���Ƿ���λ
    delay_ms(500);
    strx=strstr((const char*)RxBuffer,(const char*)"+CPIN: READY");//�鿴�Ƿ񷵻�ready
    while(strx==NULL)
    {
        Clear_Buffer();
        printf("AT+CPIN?\r\n");
        delay_ms(500);
        strx=strstr((const char*)RxBuffer,(const char*)"+CPIN: READY");//���SIM���Ƿ���λ���ȴ�����λ�������ʶ�𲻵���ʣ��Ĺ�����û������
    }
    Clear_Buffer();
    ///////////////////////////////////
    printf("AT+QMTCLOSE=0\r\n");//�Ͽ�MQTT����
    delay_ms(500);
    ///////////////////////////////////
    printf("AT+CREG?\r\n");//�鿴�Ƿ�ע��GSM����
    delay_ms(500);
    strx=strstr((const char*)RxBuffer,(const char*)"+CREG: 0,1");//��������
    extstrx=strstr((const char*)RxBuffer,(const char*)"+CREG: 0,5");//��������������
    while(strx==NULL&&extstrx==NULL)
    {
        Clear_Buffer();
        printf("AT+CREG?\r\n");//�鿴�Ƿ�ע��GSM����
        delay_ms(500);
        strx=strstr((const char*)RxBuffer,(const char*)"+CREG: 0,1");//��������
        extstrx=strstr((const char*)RxBuffer,(const char*)"+CREG: 0,5");//��������������
    }
    Clear_Buffer();
    /////////////////////////////////////
    printf("AT+CGREG?\r\n");//�鿴�Ƿ�ע��GPRS����
    delay_ms(500);
    strx=strstr((const char*)RxBuffer,(const char*)"+CGREG: 0,1");//��������Ҫ��ֻ��ע��ɹ����ſ��Խ���GPRS���ݴ��䡣
    extstrx=strstr((const char*)RxBuffer,(const char*)"+CGREG: 0,5");//��������������
    while(strx==NULL&&extstrx==NULL)
    {
        Clear_Buffer();
        printf("AT+CGREG?\r\n");//�鿴�Ƿ�ע��GPRS����
        delay_ms(500);
        strx=strstr((const char*)RxBuffer,(const char*)"+CGREG: 0,1");//��������Ҫ��ֻ��ע��ɹ����ſ��Խ���GPRS���ݴ��䡣
        extstrx=strstr((const char*)RxBuffer,(const char*)"+CGREG: 0,5");//��������������
    }
    Clear_Buffer();
    printf("AT+QIACT=1\r\n");//����
    delay_ms(500);
    Clear_Buffer();
    printf("AT+QIACT?\r\n");//��ȡ��ǰ����IP��ַ
    delay_ms(500);
    Clear_Buffer();
}


void  MQTT_Init(void)
{
    char *strx = NULL; 	//����ֵָ���ж�

    printf("AT+QMTOPEN=0,\"47.92.146.210\",1883\r\n");//ͨ��TCP��ʽȥ����MQTT������
    delay_ms(500);
    strx=strstr((const char*)RxBuffer,(const char*)"+QMTOPEN: 0,0");//���·���״̬
    while(strx==NULL)
    {
        strx=strstr((const char*)RxBuffer,(const char*)"+QMTOPEN: 0,0");//ȷ�Ϸ���ֵ��ȷ
    }
    Clear_Buffer();
    printf("AT+QMTCONN=0,\"clientExample\",\"username\",\"password\"\r\n");//ȥ��¼MQTT���������豸ID���˺ź�����.�û�����ʵ����Ҫ���и���
    delay_ms(500);
    strx=strstr((const char*)RxBuffer,(const char*)"+QMTCONN: 0,0,0");//���·���״̬
    while(strx==NULL)
    {
        strx=strstr((const char*)RxBuffer,(const char*)"+QMTCONN: 0,0,0");//���·���״̬
    }
    Clear_Buffer();
    printf("AT+QMTSUB=0,1,\"MZH_EC600S\",0\r\n");//���ĸ�����
    delay_ms(500);
    strx=strstr((const char*)RxBuffer,(const char*)"+QMTSUB: 0,1,0,0");//���ĳɹ�
    while(strx==NULL)
    {
        strx=strstr((const char*)RxBuffer,(const char*)"+QMTSUB: 0,1,0,0");//���ĳɹ�
    }
    Clear_Buffer();
}

void Send_SMS(char *phone,char *data)//���Ͷ���
{
    char *strx=0; 	//����ֵָ���ж�
    printf("AT+CMGF=1\r\n");//�����ı�ģʽ
    delay_ms(500);
    printf("AT+CSCS=\042GSM\042\r\n");   //����TE�ַ���
    delay_ms(500);
    printf("AT+CMGS=\"");
    printf(phone);//�����ֻ�����
    printf("\"\r\n");
    delay_ms(500);
    printf (data);//�ı�����
    Clear_Buffer();	//������������
    while((USART2->SR&0X40)==0);//
    USART2->DR = 0X1A;   //���ͽ�����
    delay_ms(500);
    while(1)
    {
        strx=strstr((const char*)RxBuffer,(const char*)"+CMGS");//����CMGS�������ŷ��ͳɹ�
        if(strx)
            break;//����
    }
    Clear_Buffer();	//������������
}

