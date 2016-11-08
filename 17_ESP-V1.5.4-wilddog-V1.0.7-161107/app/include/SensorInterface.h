
#ifndef __SENSOR_INTERFACE_H__
#define  __SENSOR_INTERFACE_H__

#include "giz_includes.h"

E_BOOL FLASH_SAVE Sensor_GetHourMinSec(uint32_t * hour, uint32_t * min, uint32_t * sec);
uint8_t FLASH_SAVE Sensor_GetSysState(void);
uint16_t FLASH_SAVE Sensor_GetPM25Val(void);
uint32_t FLASH_SAVE Sensor_GetFormaldehydeVal(void);
uint32_t FLASH_SAVE Sensor_GetTemperatureVal(void);
uint32_t FLASH_SAVE Sensor_GetHumidityVal(void);
uint32_t FLASH_SAVE Sensor_GetBatCapacityVal(void);

#endif

