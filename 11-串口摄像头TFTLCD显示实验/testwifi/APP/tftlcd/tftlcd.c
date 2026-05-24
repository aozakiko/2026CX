#include "tftlcd.h"
#include "serial_image.h"
#include "SysTick.h"
#include "stm32f10x_fsmc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

u16 FRONT_COLOR = WHITE;
u16 BACK_COLOR = BLACK;
_tftlcd_data tftlcd_data;

static void TFTLCD_FSMC_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	FSMC_NORSRAMInitTypeDef FSMC_NORSRAMInitStructure;
	FSMC_NORSRAMTimingInitTypeDef FSMC_ReadWriteTimingStructure;
	FSMC_NORSRAMTimingInitTypeDef FSMC_WriteTimingStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE |
						   RCC_APB2Periph_GPIOG | RCC_APB2Periph_GPIOB |
						   RCC_APB2Periph_AFIO, ENABLE);

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 |
								  GPIO_Pin_5 | GPIO_Pin_8 | GPIO_Pin_9 |
								  GPIO_Pin_10 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 |
								  GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 |
								  GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_12;
	GPIO_Init(GPIOG, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	LCD_LED = 1;

	FSMC_ReadWriteTimingStructure.FSMC_AddressSetupTime = 0x0F;
	FSMC_ReadWriteTimingStructure.FSMC_AddressHoldTime = 0x00;
	FSMC_ReadWriteTimingStructure.FSMC_DataSetupTime = 60;
	FSMC_ReadWriteTimingStructure.FSMC_BusTurnAroundDuration = 0x00;
	FSMC_ReadWriteTimingStructure.FSMC_CLKDivision = 0x00;
	FSMC_ReadWriteTimingStructure.FSMC_DataLatency = 0x00;
	FSMC_ReadWriteTimingStructure.FSMC_AccessMode = FSMC_AccessMode_A;

	FSMC_WriteTimingStructure.FSMC_AddressSetupTime = 9;
	FSMC_WriteTimingStructure.FSMC_AddressHoldTime = 0x00;
	FSMC_WriteTimingStructure.FSMC_DataSetupTime = 8;
	FSMC_WriteTimingStructure.FSMC_BusTurnAroundDuration = 0x00;
	FSMC_WriteTimingStructure.FSMC_CLKDivision = 0x00;
	FSMC_WriteTimingStructure.FSMC_DataLatency = 0x00;
	FSMC_WriteTimingStructure.FSMC_AccessMode = FSMC_AccessMode_A;

	FSMC_NORSRAMDeInit(FSMC_Bank1_NORSRAM4);
	FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM4;
	FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
	FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_SRAM;
	FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
	FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
	FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
	FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
	FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Enable;
	FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
	FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &FSMC_ReadWriteTimingStructure;
	FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &FSMC_WriteTimingStructure;
	FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);
	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM4, ENABLE);
}

void LCD_WriteCmd(u16 cmd)
{
	TFTLCD->LCD_CMD = cmd;
}

void LCD_WriteData(u16 data)
{
	TFTLCD->LCD_DATA = data;
}

void LCD_WriteCmdData(u16 cmd, u16 data)
{
	LCD_WriteCmd(cmd);
	LCD_WriteData(data);
}

void LCD_WriteData_Color(u16 color)
{
	TFTLCD->LCD_DATA = color >> 8;
	TFTLCD->LCD_DATA = color & 0xFF;
}

u16 LCD_ReadData(void)
{
	return TFTLCD->LCD_DATA;
}

static void TFTLCD_HX8357DN_InitSequence(void)
{
	LCD_WriteCmd(0x11);
	delay_ms(120);

	LCD_WriteCmd(0xB9);
	LCD_WriteData(0xFF);
	LCD_WriteData(0x83);
	LCD_WriteData(0x57);
	delay_ms(5);

	LCD_WriteCmd(0xB1);
	LCD_WriteData(0x00);
	LCD_WriteData(0x14);
	LCD_WriteData(0x1C);
	LCD_WriteData(0x1C);
	LCD_WriteData(0xC3);
	LCD_WriteData(0x44);
	LCD_WriteData(0x70);
	delay_ms(5);

	LCD_WriteCmd(0xB4);
	LCD_WriteData(0x22);
	LCD_WriteData(0x40);
	LCD_WriteData(0x00);
	LCD_WriteData(0x2A);
	LCD_WriteData(0x2A);
	LCD_WriteData(0x20);
	LCD_WriteData(0x91);
	delay_ms(5);

	LCD_WriteCmd(0x36);
	LCD_WriteData(0x4C);

	LCD_WriteCmd(0xC0);
	LCD_WriteData(0x50);
	LCD_WriteData(0x50);
	LCD_WriteData(0x01);
	LCD_WriteData(0x3C);
	LCD_WriteData(0xC8);
	LCD_WriteData(0x08);
	LCD_WriteData(0x00);
	LCD_WriteData(0x08);
	LCD_WriteData(0x04);
	delay_ms(5);

	LCD_WriteCmd(0xE0);
	LCD_WriteData(0x0B);
	LCD_WriteData(0x11);
	LCD_WriteData(0x1E);
	LCD_WriteData(0x30);
	LCD_WriteData(0x3A);
	LCD_WriteData(0x43);
	LCD_WriteData(0x4E);
	LCD_WriteData(0x56);
	LCD_WriteData(0x45);
	LCD_WriteData(0x3F);
	LCD_WriteData(0x39);
	LCD_WriteData(0x32);
	LCD_WriteData(0x2F);
	LCD_WriteData(0x2A);
	LCD_WriteData(0x29);
	LCD_WriteData(0x21);
	LCD_WriteData(0x0B);
	LCD_WriteData(0x11);
	LCD_WriteData(0x1E);
	LCD_WriteData(0x30);
	LCD_WriteData(0x3A);
	LCD_WriteData(0x43);
	LCD_WriteData(0x4E);
	LCD_WriteData(0x56);
	LCD_WriteData(0x45);
	LCD_WriteData(0x3F);
	LCD_WriteData(0x39);
	LCD_WriteData(0x32);
	LCD_WriteData(0x2F);
	LCD_WriteData(0x2A);
	LCD_WriteData(0x29);
	LCD_WriteData(0x21);
	LCD_WriteData(0x00);
	LCD_WriteData(0x01);

	LCD_WriteCmd(0x3A);
	LCD_WriteData(0x05);
	LCD_WriteCmd(0x29);
	delay_ms(20);
}

void LCD_Display_Dir(u8 dir)
{
	tftlcd_data.dir = dir;
	LCD_WriteCmd(0x36);

	if (dir == 0)
	{
		LCD_WriteData(0x4C);
		tftlcd_data.width = 320;
		tftlcd_data.height = 480;
	}
	else
	{
		LCD_WriteData(0x2C);
		tftlcd_data.width = 480;
		tftlcd_data.height = 320;
	}
}

void LCD_Set_Window(u16 sx, u16 sy, u16 ex, u16 ey)
{
	LCD_WriteCmd(0x2A);
	LCD_WriteData(sx >> 8);
	LCD_WriteData(sx & 0xFF);
	LCD_WriteData(ex >> 8);
	LCD_WriteData(ex & 0xFF);

	LCD_WriteCmd(0x2B);
	LCD_WriteData(sy >> 8);
	LCD_WriteData(sy & 0xFF);
	LCD_WriteData(ey >> 8);
	LCD_WriteData(ey & 0xFF);
	LCD_WriteCmd(0x2C);
}

void TFTLCD_Init(void)
{
	TFTLCD_FSMC_Init();
	delay_ms(50);
	tftlcd_data.id = 0x8357;
	TFTLCD_HX8357DN_InitSequence();
	LCD_Display_Dir(TFTLCD_DIR);
	LCD_Clear(BLACK);
}

void LCD_Clear(u16 color)
{
	u32 total;

	LCD_Set_Window(0, 0, tftlcd_data.width - 1, tftlcd_data.height - 1);
	total = (u32)tftlcd_data.width * tftlcd_data.height;
	while (total > 0)
	{
		LCD_WriteData_Color(color);
		total--;
	}
}

void LCD_Fill(u16 xState, u16 yState, u16 xEnd, u16 yEnd, u16 color)
{
	u32 total;

	if ((xState > xEnd) || (yState > yEnd))
	{
		return;
	}
	if ((xState >= tftlcd_data.width) || (yState >= tftlcd_data.height))
	{
		return;
	}
	if (xEnd >= tftlcd_data.width)
	{
		xEnd = tftlcd_data.width - 1;
	}
	if (yEnd >= tftlcd_data.height)
	{
		yEnd = tftlcd_data.height - 1;
	}

	LCD_Set_Window(xState, yState, xEnd, yEnd);
	total = (u32)(xEnd - xState + 1) * (yEnd - yState + 1);
	while (total > 0)
	{
		LCD_WriteData_Color(color);
		total--;
	}
}

void LCD_DrawPoint(u16 x, u16 y)
{
	LCD_DrawFRONT_COLOR(x, y, FRONT_COLOR);
}

void LCD_DrawFRONT_COLOR(u16 x, u16 y, u16 color)
{
	if ((x >= tftlcd_data.width) || (y >= tftlcd_data.height))
	{
		return;
	}

	LCD_Set_Window(x, y, x, y);
	LCD_WriteData_Color(color);
}

void TFTLCD_DisplayColorFrame(const u8 *frame)
{
	u16 draw_w;
	u16 draw_h;
	u16 x0;
	u16 y0;
	u16 x;
	u16 y;
	u16 src_x;
	u16 src_y;
	u32 src_index;
	u16 color;

	if (frame == 0)
	{
		return;
	}
	if ((tftlcd_data.width == 0) || (tftlcd_data.height == 0))
	{
		return;
	}

	if ((u32)tftlcd_data.width * SERIAL_IMAGE_HEIGHT <=
		(u32)tftlcd_data.height * SERIAL_IMAGE_WIDTH)
	{
		draw_w = tftlcd_data.width;
		draw_h = (u16)(((u32)draw_w * SERIAL_IMAGE_HEIGHT) / SERIAL_IMAGE_WIDTH);
	}
	else
	{
		draw_h = tftlcd_data.height;
		draw_w = (u16)(((u32)draw_h * SERIAL_IMAGE_WIDTH) / SERIAL_IMAGE_HEIGHT);
	}

	if ((draw_w == 0) || (draw_h == 0))
	{
		return;
	}

	x0 = (tftlcd_data.width - draw_w) / 2;
	y0 = (tftlcd_data.height - draw_h) / 2;

	for (y = 0; y < draw_h; y++)
	{
		src_y = (u16)(((u32)y * SERIAL_IMAGE_HEIGHT) / draw_h);
		LCD_Set_Window(x0, y0 + y, x0 + draw_w - 1, y0 + y);

		for (x = 0; x < draw_w; x++)
		{
			src_x = (u16)(((u32)x * SERIAL_IMAGE_WIDTH) / draw_w);
			src_index = ((u32)src_y * SERIAL_IMAGE_WIDTH + src_x) * SERIAL_IMAGE_BYTES_PER_PIXEL;
			color = ((u16)frame[src_index] << 8) | frame[src_index + 1];
			LCD_WriteData_Color(color);
		}
	}
}
