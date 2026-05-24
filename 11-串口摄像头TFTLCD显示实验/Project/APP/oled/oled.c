#include "oled.h"
#include "SysTick.h"

#define OLED_SCL_HIGH() GPIO_SetBits(OLED_I2C_PORT, OLED_I2C_SCL_PIN)
#define OLED_SCL_LOW()  GPIO_ResetBits(OLED_I2C_PORT, OLED_I2C_SCL_PIN)
#define OLED_SDA_HIGH() GPIO_SetBits(OLED_I2C_PORT, OLED_I2C_SDA_PIN)
#define OLED_SDA_LOW()  GPIO_ResetBits(OLED_I2C_PORT, OLED_I2C_SDA_PIN)

static void OLED_I2C_Delay(void)
{
	delay_us(5);
}

static void OLED_I2C_Start(void)
{
	OLED_SDA_HIGH();
	OLED_SCL_HIGH();
	OLED_I2C_Delay();
	OLED_SDA_LOW();
	OLED_I2C_Delay();
	OLED_SCL_LOW();
}

static void OLED_I2C_Stop(void)
{
	OLED_SDA_LOW();
	OLED_SCL_HIGH();
	OLED_I2C_Delay();
	OLED_SDA_HIGH();
	OLED_I2C_Delay();
}

static void OLED_I2C_WriteByte(u8 data)
{
	u8 i;

	for (i = 0; i < 8; i++)
	{
		if ((data & 0x80) != 0)
		{
			OLED_SDA_HIGH();
		}
		else
		{
			OLED_SDA_LOW();
		}

		OLED_I2C_Delay();
		OLED_SCL_HIGH();
		OLED_I2C_Delay();
		OLED_SCL_LOW();
		data <<= 1;
	}

	OLED_SDA_HIGH();
	OLED_I2C_Delay();
	OLED_SCL_HIGH();
	OLED_I2C_Delay();
	OLED_SCL_LOW();
}

static void OLED_WriteCommandTo(u8 addr, u8 command)
{
	OLED_I2C_Start();
	OLED_I2C_WriteByte(addr);
	OLED_I2C_WriteByte(0x00);
	OLED_I2C_WriteByte(command);
	OLED_I2C_Stop();
}

static void OLED_WriteCommand(u8 command)
{
	OLED_WriteCommandTo(OLED_I2C_ADDR, command);
#if OLED_SEND_TO_BOTH_ADDRS
	OLED_WriteCommandTo(OLED_I2C_ADDR_ALT, command);
#endif
}

static void OLED_WriteDataBufferTo(u8 addr, const u8 *data, u8 len)
{
	u8 i;

	OLED_I2C_Start();
	OLED_I2C_WriteByte(addr);
	OLED_I2C_WriteByte(0x40);
	for (i = 0; i < len; i++)
	{
		OLED_I2C_WriteByte(data[i]);
	}
	OLED_I2C_Stop();
}

static void OLED_WriteDataBuffer(const u8 *data, u8 len)
{
	OLED_WriteDataBufferTo(OLED_I2C_ADDR, data, len);
#if OLED_SEND_TO_BOTH_ADDRS
	OLED_WriteDataBufferTo(OLED_I2C_ADDR_ALT, data, len);
#endif
}

static void OLED_SetPos(u8 page, u8 col)
{
	OLED_WriteCommand((u8)(0xB0 + page));
	col += OLED_COLUMN_OFFSET;
	OLED_WriteCommand((u8)(0x00 + (col & 0x0F)));
	OLED_WriteCommand((u8)(0x10 + ((col >> 4) & 0x0F)));
}

void OLED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(OLED_I2C_RCC, ENABLE);

	GPIO_InitStructure.GPIO_Pin = OLED_I2C_SCL_PIN | OLED_I2C_SDA_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(OLED_I2C_PORT, &GPIO_InitStructure);

	OLED_SCL_HIGH();
	OLED_SDA_HIGH();
	delay_ms(100);

	OLED_WriteCommand(0xAE);
	OLED_WriteCommand(0x20);
	OLED_WriteCommand(0x02);
	OLED_WriteCommand(0xB0);
	OLED_WriteCommand(0xC8);
	OLED_WriteCommand(0x00);
	OLED_WriteCommand(0x10);
	OLED_WriteCommand(0x40);
	OLED_WriteCommand(0x81);
	OLED_WriteCommand(0x7F);
	OLED_WriteCommand(0xA1);
	OLED_WriteCommand(0xA6);
	OLED_WriteCommand(0xA8);
	OLED_WriteCommand(0x3F);
	OLED_WriteCommand(0xA4);
	OLED_WriteCommand(0xD3);
	OLED_WriteCommand(0x00);
	OLED_WriteCommand(0xD5);
	OLED_WriteCommand(0x80);
	OLED_WriteCommand(0xD9);
	OLED_WriteCommand(0xF1);
	OLED_WriteCommand(0xDA);
	OLED_WriteCommand(0x12);
	OLED_WriteCommand(0xDB);
	OLED_WriteCommand(0x40);
	OLED_WriteCommand(0x8D);
	OLED_WriteCommand(0x14);
	OLED_WriteCommand(0xAF);

	OLED_Clear();
}

void OLED_Clear(void)
{
	u8 page;
	u8 col;
	u8 zeros[16] = {0};

	for (page = 0; page < OLED_PAGE_COUNT; page++)
	{
		OLED_SetPos(page, 0);
		for (col = 0; col < OLED_WIDTH; col += sizeof(zeros))
		{
			OLED_WriteDataBuffer(zeros, sizeof(zeros));
		}
	}
}

void OLED_DisplayFrame(const u8 *frame)
{
	u8 page;
	u8 col;

	for (page = 0; page < OLED_PAGE_COUNT; page++)
	{
		OLED_SetPos(page, 0);
		for (col = 0; col < OLED_WIDTH; col += 16)
		{
			OLED_WriteDataBuffer(&frame[page * OLED_WIDTH + col], 16);
		}
	}
}

void OLED_TestPattern(void)
{
	u16 i;
	static u8 pattern[OLED_FRAME_SIZE];

	for (i = 0; i < OLED_FRAME_SIZE; i++)
	{
		if ((i / OLED_WIDTH) == 0 || (i / OLED_WIDTH) == 7)
		{
			pattern[i] = 0xFF;
		}
		else if ((i & 0x01) == 0)
		{
			pattern[i] = 0xAA;
		}
		else
		{
			pattern[i] = 0x55;
		}
	}

	OLED_DisplayFrame(pattern);
}
