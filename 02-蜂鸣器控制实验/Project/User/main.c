#include "system.h"
#include "SysTick.h"
#include "led.h"
#include "beep.h"


int main()
{
	SysTick_Init(72);
	LED_Init();
	BEEP_Init();
	
	while(1)
	{
		LED1=!LED1;
		BEEP=!BEEP;
		delay_ms(500);
	}
}
