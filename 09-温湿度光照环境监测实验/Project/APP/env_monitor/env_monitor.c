#include "env_monitor.h"
#include "SysTick.h"
#include "dht11.h"

#define ENV_ADC_READ_TIMES        10
#define ENV_HUMIDITY_NOT_PRESENT  (-1)

static u8 s_dht11_ready = 0;

static void EnvMonitor_InitAdc(ADC_TypeDef *adc)
{
	ADC_InitTypeDef ADC_InitStructure;

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_Init(adc, &ADC_InitStructure);

	ADC_Cmd(adc, ENABLE);

	ADC_ResetCalibration(adc);
	while(ADC_GetResetCalibrationStatus(adc));

	ADC_StartCalibration(adc);
	while(ADC_GetCalibrationStatus(adc));
}

static u16 EnvMonitor_ReadAdcAverage(ADC_TypeDef *adc, u8 channel, u8 times)
{
	u32 value = 0;
	u8 i;

	ADC_RegularChannelConfig(adc, channel, 1, ADC_SampleTime_239Cycles5);
	for(i = 0; i < times; i++)
	{
		ADC_SoftwareStartConvCmd(adc, ENABLE);
		while(!ADC_GetFlagStatus(adc, ADC_FLAG_EOC));
		value += ADC_GetConversionValue(adc);
		delay_ms(5);
	}

	return value / times;
}

void EnvMonitor_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF | RCC_APB2Periph_ADC1 | RCC_APB2Periph_ADC3, ENABLE);
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOF, &GPIO_InitStructure);

	ADC_TempSensorVrefintCmd(ENABLE);
	EnvMonitor_InitAdc(ADC1);
	EnvMonitor_InitAdc(ADC3);

	s_dht11_ready = (DHT11_Init() == 0);
}

int EnvMonitor_GetTemperatureX100(void)
{
	u32 adc_value;
	double temperature;

	adc_value = EnvMonitor_ReadAdcAverage(ADC1, ADC_Channel_16, ENV_ADC_READ_TIMES);
	temperature = (float)adc_value * (3.3 / 4096);
	temperature = (1.43 - temperature) / 0.0043 + 25;

	return (int)(temperature * 100);
}

u8 EnvMonitor_GetLightPercent(void)
{
	u32 adc_value;

	adc_value = EnvMonitor_ReadAdcAverage(ADC3, ADC_Channel_6, ENV_ADC_READ_TIMES);
	if(adc_value > 4000)
	{
		adc_value = 4000;
	}

	return (u8)(100 - (adc_value / 40));
}

int EnvMonitor_GetHumidityPercent(void)
{
	u8 dht_temp = 0;
	u8 humidity = 0;

	if(!s_dht11_ready)
	{
		s_dht11_ready = (DHT11_Init() == 0);
		if(!s_dht11_ready)
		{
			return ENV_HUMIDITY_NOT_PRESENT;
		}
	}

	if(DHT11_Read_Data(&dht_temp, &humidity) != 0)
	{
		s_dht11_ready = 0;
		return ENV_HUMIDITY_NOT_PRESENT;
	}

	return humidity;
}

u16 EnvMonitor_LightPercentToLux(u8 light_percent)
{
	return (u16)light_percent * 10;
}

void EnvMonitor_Read(EnvMonitor_Data *data)
{
	if(data == 0)
	{
		return;
	}

	data->temperature_x100 = EnvMonitor_GetTemperatureX100();
	data->humidity_percent = EnvMonitor_GetHumidityPercent();
	data->light_percent = EnvMonitor_GetLightPercent();
	data->light_lux = EnvMonitor_LightPercentToLux(data->light_percent);
}
