
#ifndef __SERIAL_DRV_H__
#define  __SERIAL_DRV_H__

#include "giz_includes.h"


#if SERIAL_DEBUG_EN
#define SERIAL_DEBUG  os_printf
#else
#define SERIAL_DEBUG(...)
#endif


typedef void (*ATCMD_RxProcessFunc)(uint8* data,int32 len);


void FLASH_SAVE SerialDrv_SetRxCompleteBool(E_BOOL isComplete);
void FLASH_SAVE SerialDrv_Init(ATCMD_RxProcessFunc func);




#endif

