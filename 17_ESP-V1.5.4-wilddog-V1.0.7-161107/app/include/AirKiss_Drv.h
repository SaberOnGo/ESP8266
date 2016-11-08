
#ifndef __AIRKISS_DRV_H__
#define __AIRKISS_DRV_H__

#include "ets_sys.h"
#include "osapi.h"
#include "ip_addr.h"
#include "espconn.h"
#include "mem.h"
#include "user_interface.h"
#include "smartconfig.h"
#include "airkiss.h"
#include "GlobalDef.h"

#if AIRKISS_DEBUG_EN
#define AIRKISS_DEBUG   os_printf
#else
#define AIRKISS_DEBUG(...) 
#endif
void ICACHE_FLASH_ATTR AirKiss_Init(void);
uint8_t FLASH_SAVE AirKiss_IsNetConnected(void);


#endif

