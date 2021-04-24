#ifndef __LED_H
#define __LED_H	 
#include "sys.h"

void LED_Init(void);//≥ı ºªØ
void EC600SCTR_Init(void);
#define LED1     PBout(0)
#define LED2  	 PCout(7)
#define PWRKEY  PBout(9)
#define RESET   PAout(8)

#endif
