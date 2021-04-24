#include "led.h"


//初始化PC6 7 
//LED IO初始化
void LED_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 //使能PC端口时钟
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;				 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
    GPIO_Init(GPIOB, &GPIO_InitStructure);					 //根据设定参数初始化GPIOC6 7
	
   		
}
 

void EC600SCTR_Init(void)
{  
    GPIO_InitTypeDef  GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 //使能PA,PB端口时钟
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;				 //PWRKEY-->PB9 端口配置
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
    GPIO_Init(GPIOB, &GPIO_InitStructure);		    //PB9 输出高
    

}


