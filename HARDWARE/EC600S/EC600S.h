#ifndef __EC600S_H
#define __EC600S_H
#include "stm32f10x.h"
void Uart1_SendStr(char*SendBuf);//����1��ӡ����;
void Clear_Buffer(void);//��ջ���
void  EC600S_Init(void);//EC600S��ʼ��
void Send_Str(char*data);//��������
void  MQTT_Init(void);
#endif

