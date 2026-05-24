#include "dht11.h"

u8 DHT11_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG, ENABLE);

	GPIO_InitStructure.GPIO_Pin = DHT11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIO_DHT11, &GPIO_InitStructure);
	GPIO_SetBits(GPIO_DHT11, DHT11);

	DHT11_Rst();
	return DHT11_Check();
}

void DHT11_Rst(void)
{
	DHT11_IO_OUT();
	DHT11_DQ_OUT = 0;
	delay_ms(20);
	DHT11_DQ_OUT = 1;
	delay_us(30);
}

u8 DHT11_Check(void)
{
	u8 retry = 0;

	DHT11_IO_IN();
	while(DHT11_DQ_IN && retry < 100)
	{
		retry++;
		delay_us(1);
	}

	if(retry >= 100)
	{
		return 1;
	}

	retry = 0;
	while(!DHT11_DQ_IN && retry < 100)
	{
		retry++;
		delay_us(1);
	}

	if(retry >= 100)
	{
		return 1;
	}

	return 0;
}

u8 DHT11_Read_Bit(void)
{
	u8 retry = 0;

	while(DHT11_DQ_IN && retry < 100)
	{
		retry++;
		delay_us(1);
	}

	retry = 0;
	while(!DHT11_DQ_IN && retry < 100)
	{
		retry++;
		delay_us(1);
	}

	delay_us(40);
	if(DHT11_DQ_IN)
	{
		return 1;
	}

	return 0;
}

u8 DHT11_Read_Byte(void)
{
	u8 i;
	u8 data = 0;

	for(i = 0; i < 8; i++)
	{
		data <<= 1;
		data |= DHT11_Read_Bit();
	}

	return data;
}

u8 DHT11_Read_Data(u8 *temp, u8 *humi)
{
	u8 buf[5];
	u8 i;

	DHT11_Rst();
	if(DHT11_Check() != 0)
	{
		return 1;
	}

	for(i = 0; i < 5; i++)
	{
		buf[i] = DHT11_Read_Byte();
	}

	if((u8)(buf[0] + buf[1] + buf[2] + buf[3]) != buf[4])
	{
		return 1;
	}

	*humi = buf[0];
	*temp = buf[2];
	return 0;
}

void DHT11_IO_OUT(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = DHT11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIO_DHT11, &GPIO_InitStructure);
}

void DHT11_IO_IN(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = DHT11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIO_DHT11, &GPIO_InitStructure);
}
