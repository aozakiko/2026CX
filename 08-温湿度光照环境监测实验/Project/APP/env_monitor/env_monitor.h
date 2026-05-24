#ifndef _env_monitor_H
#define _env_monitor_H

#include "system.h"

typedef struct
{
	int temperature_x100;
	int humidity_percent;
	u8 light_percent;
	u16 light_lux;
} EnvMonitor_Data;

void EnvMonitor_Init(void);
int EnvMonitor_GetTemperatureX100(void);
int EnvMonitor_GetHumidityPercent(void);
u8 EnvMonitor_GetLightPercent(void);
u16 EnvMonitor_LightPercentToLux(u8 light_percent);
void EnvMonitor_Read(EnvMonitor_Data *data);

#endif
