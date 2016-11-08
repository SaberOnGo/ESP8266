
#ifndef __GLOBAL_FUNC_H__
#define  __GLOBAL_FUNC_H__

#include "ets_sys.h"
#include "osapi.h"
#include "mem.h"
#include "GlobalDef.h"

typedef enum
{
   E_PROTOCOL_VER = 0,
   E_PRIVATE_VER,
   E_HARD_VER,
   E_SOFT_VER,
   E_PRODUCT_KEY,
}E_VERION_TYPE;

uint32_t FLASH_SAVE Sys_GetRunTime(void);
void FLASH_SAVE Sys_DelayMs(uint32_t ms);
E_BOOL FLASH_SAVE Sys_IsBigEndian(void);
E_RESULT Sys_GetVersion(uint8_t *outString, int16_t stringSize, E_VERION_TYPE index);

#endif

