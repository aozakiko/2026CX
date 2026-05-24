#include "system.h"
#include "SysTick.h"
#include "led.h"
#include "usart.h"
#include "wifi_config.h"
#include "wifi_function.h"
#include <string.h>


void (*pNet_Test)(void);

int main()
{
	char cCh;	
	
	SysTick_Init(72);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  //中断优先级分组 分2组
	USART1_Init(9600); //初始化串口1,波特率9600
	LED_Init();
	WiFi_Config();
	printf("Hello, world!\r\n"); // 使用printf发送数据
 // 欢迎信息
    printf("\r\n====================================\r\n");
    printf("STM32 + ESP8266 TCP Client Demo\r\n");
    printf("====================================\r\n");
    
    // 初始化ESP8266并连接WiFi
		ESP8266_STA_TCP_Client_MQTT();
   
	
	while(1){
		
	
	};
}
