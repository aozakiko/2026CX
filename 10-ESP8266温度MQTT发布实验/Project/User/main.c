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
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  //?��?????????? ??2??
	USART1_Init(9600); //?????????1,??????9600
	LED_Init();
	WiFi_Config();
	printf("Hello, world!\r\n"); // ???printf????????
 // ??????
    printf("\r\n====================================\r\n");
    printf("STM32 + ESP8266 TCP Client Demo\r\n");
    printf("====================================\r\n");
    
    // ?????ESP8266??????WiFi
		ESP8266_STA_TCP_Client_MQTT();
   
	
	while ( 1 ) { }
}
