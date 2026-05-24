#include "adc_temp.h"
#include "SysTick.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_rcc.h"

/* STM32F103 内部温度传感器：ADC1 通道 16 */
#define TEMP_V25        1.43f
#define TEMP_AVG_SLOPE  0.0043f

void ADC_Temp_Init(void)
{
	ADC_InitTypeDef ADC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);

	ADC_TempSensorVrefintCmd(ENABLE);
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_Init(ADC1, &ADC_InitStructure);

	ADC_RegularChannelConfig(ADC1, ADC_Channel_TempSensor, 1, ADC_SampleTime_239Cycles5);
	ADC_Cmd(ADC1, ENABLE);

	ADC_ResetCalibration(ADC1);
	while (ADC_GetResetCalibrationStatus(ADC1));
	ADC_StartCalibration(ADC1);
	while (ADC_GetCalibrationStatus(ADC1));
}

float ADC_Temp_ReadCelsius(void)
{
	u16 adcx;
	float voltage;
	u8 i;

	adcx = 0;
	for (i = 0; i < 10; i++)
	{
		ADC_SoftwareStartConvCmd(ADC1, ENABLE);
		while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
		adcx += ADC_GetConversionValue(ADC1);
		delay_ms(2);
	}
	adcx /= 10;

	voltage = (float)adcx * 3.3f / 4096.0f;
	return (TEMP_V25 - voltage) / TEMP_AVG_SLOPE + 25.0f;
}
