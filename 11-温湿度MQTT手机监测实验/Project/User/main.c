#include "system.h"
#include "SysTick.h"
#include "led.h"
#include "usart.h"
#include "wifi_config.h"
#include "wifi_function.h"
#include "env_monitor.h"
#include <stdio.h>

#define MQTT_RETRY_INTERVAL_SECONDS 5

static void FormatClock(u32 seconds, char *buffer)
{
	u32 hour = seconds / 3600;
	u32 minute = (seconds % 3600) / 60;
	u32 second = seconds % 60;

	hour %= 24;
	sprintf(buffer, "%02u:%02u:%02u",
	        (unsigned int)hour,
	        (unsigned int)minute,
	        (unsigned int)second);
}

static void PrintTemperatureValue(int temperature_x100)
{
	if(temperature_x100 < 0)
	{
		temperature_x100 = -temperature_x100;
		printf("-");
	}
	else
	{
		printf("+");
	}

	printf("%d.%02d", temperature_x100 / 100, temperature_x100 % 100);
}

int main(void)
{
	EnvMonitor_Data env_data;
	u32 uptime_seconds = 0;
	char clock_text[12];
	bool mqtt_ready = false;
	u8 mqtt_retry_countdown = 0;

	SysTick_Init(72);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	USART1_Init(115200);
	LED_Init();
	WiFi_Config();
	EnvMonitor_Init();

	printf("\r\n====================================\r\n");
	printf("Environment MQTT Monitor Output\r\n");
	printf("PC USART1: PA9(TX) PA10(RX), 115200 8N1\r\n");
	printf("ESP8266 testwifi pins: USART3 PB10/PB11, CH PA4, RST PA15\r\n");
	printf("Light sensor: PF8 / ADC channel 6\r\n");
	printf("DHT11 humidity sensor: PG11\r\n");
	printf("Frame: ENV,uptime_s,temp_x100,humidity,light_percent,lux\r\n");
	printf("MQTT broker: 192.168.166.177:1883\r\n");
	printf("MQTT topics: stm32/env, stm32/temperature, stm32/humidity\r\n");
	printf("====================================\r\n");

	mqtt_ready = ESP8266_MQTT_Connect_Default();
	if(!mqtt_ready)
	{
		printf("MQTT startup failed, serial monitor keeps running.\r\n");
		mqtt_retry_countdown = MQTT_RETRY_INTERVAL_SECONDS;
	}

	while(1)
	{
		FormatClock(uptime_seconds, clock_text);
		EnvMonitor_Read(&env_data);

		printf("[%s] Temp=", clock_text);
		PrintTemperatureValue(env_data.temperature_x100);
		if(env_data.humidity_percent >= 0)
		{
			printf("C Humidity=%d%% Light=%d%% Lux=%d\r\n",
			       env_data.humidity_percent,
			       env_data.light_percent,
			       env_data.light_lux);
		}
		else
		{
			printf("C Humidity=-- Light=%d%% Lux=%d\r\n",
			       env_data.light_percent,
			       env_data.light_lux);
		}

		printf("ENV,%u,%d,%d,%d,%d\r\n",
		       (unsigned int)uptime_seconds,
		       env_data.temperature_x100,
		       env_data.humidity_percent,
		       env_data.light_percent,
		       env_data.light_lux);

		if(mqtt_ready)
		{
			ESP8266_MQTT_Poll();
			mqtt_ready = ESP8266_MQTT_Publish_Environment(uptime_seconds,
			                                               env_data.temperature_x100,
			                                               env_data.humidity_percent,
			                                               env_data.light_percent,
			                                               env_data.light_lux);
			if(!mqtt_ready)
			{
				printf("MQTT publish failed, reconnect after %d seconds.\r\n",
				       MQTT_RETRY_INTERVAL_SECONDS);
				mqtt_retry_countdown = MQTT_RETRY_INTERVAL_SECONDS;
			}
		}
		else if(mqtt_retry_countdown == 0)
		{
			printf("Retrying MQTT connection...\r\n");
			mqtt_ready = ESP8266_MQTT_Connect_Default();
			if(!mqtt_ready)
			{
				mqtt_retry_countdown = MQTT_RETRY_INTERVAL_SECONDS;
			}
		}
		else
		{
			mqtt_retry_countdown--;
		}

		LED1 = !LED1;
		delay_ms(1000);
		uptime_seconds++;
	}
}
