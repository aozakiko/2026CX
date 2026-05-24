#include "system.h"
#include "SysTick.h"
#include "led.h"
#include "usart.h"
#include "adc_temp.h"
#include "tftlcd.h"
#include <stdio.h>


static void LCD_Show_Temperature(int temp)
{
	char temp_buf[16];

	if(temp < 0)
	{
		temp = -temp;
		sprintf(temp_buf, "-%d.%02d", temp / 100, temp % 100);
	}
	else
	{
		sprintf(temp_buf, "+%d.%02d", temp / 100, temp % 100);
	}
	LCD_ShowString(70, 100, tftlcd_data.width, tftlcd_data.height, 24, (u8 *)temp_buf);
	LCD_ShowString(150, 100, tftlcd_data.width, tftlcd_data.height, 24, (u8 *)"C");
}

/*******************************************************************************
* 函 数 名         : main
* 函数功能		   : 主函数
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/
int main()
{
	u8 i = 0;
	int temp = 0;

	SysTick_Init(72);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	LED_Init();
	USART1_Init(115200);
	ADC_Temp_Init();
	TFTLCD_Init();

	FRONT_COLOR = BLACK;
	LCD_ShowString(10, 10, tftlcd_data.width, tftlcd_data.height, 16, (u8 *)"PRECHIN STM32F1");
	LCD_ShowString(10, 30, tftlcd_data.width, tftlcd_data.height, 16, (u8 *)"www.prechin.net");
	LCD_ShowString(10, 50, tftlcd_data.width, tftlcd_data.height, 16, (u8 *)"Internal Temp Test");
	LCD_ShowString(10, 100, tftlcd_data.width, tftlcd_data.height, 16, (u8 *)"Temp:");
	FRONT_COLOR = RED;

	while(1)
	{
		i++;
		if(i % 20 == 0)
		{
			LED1 = !LED1;
		}

		if(i % 50 == 0)
		{
			temp = Get_Temperture();
			LCD_Show_Temperature(temp);

			if(temp < 0)
			{
				temp = -temp;
				printf("内部温度检测值为：-");
			}
			else
			{
				printf("内部温度检测值为：+");
			}
			printf("%.2f°C\r\n", (float)temp / 100);
		}
		delay_ms(10);
	}
}
