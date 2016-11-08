

#ifndef __TCP_CLIENT_H__
#define __TCP_CLIENT_H__

#include "GlobalDef.h"
#if TCP_CLIENT_DEBUG_EN 
#define TCP_CLIENT_DEBUG os_printf
#else
#define TCP_CLIENT_DEBUG(...)
#endif
void ICACHE_FLASH_ATTR user_set_station_config(void);


#endif

