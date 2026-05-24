#include "system.h"
#include "SysTick.h"
#include "led.h"
#include "usart.h"
#include "tftlcd.h"
#include "serial_image.h"

#define CAMERA_LINK_BAUDRATE 115200

int main(void)
{
	const u8 *display_frame;

	SysTick_Init(72);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	LED_Init();
	TFTLCD_Init();
	LCD_Fill((u16)(tftlcd_data.width / 2 - 20),
			 (u16)(tftlcd_data.height / 2 - 20),
			 (u16)(tftlcd_data.width / 2 + 20),
			 (u16)(tftlcd_data.height / 2 + 20),
			 GREEN);
	USART1_Init(CAMERA_LINK_BAUDRATE);
	SerialImage_Reset();

	USART1_SendString("TFT camera receiver ready\r\n");

	while (1)
	{
		if (SerialImage_FrameReady())
		{
			display_frame = SerialImage_GetFrame();
			TFTLCD_DisplayColorFrame(display_frame);
			SerialImage_ReleaseFrame();
			LED1 = !LED1;
		}
	}
}
