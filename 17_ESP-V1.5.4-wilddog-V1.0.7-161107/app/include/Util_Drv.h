
#ifndef __UTIL_DRV_H__
#define  __UTIL_DRV_H__

#include "ets_sys.h"
#include "osapi.h"
#include "at_custom.h"
#include "ip_addr.h"
#include "espconn.h"
#include "mem.h"
#include "user_interface.h"
#include "user_devicefind.h"
#include "user_webserver.h"
#include "GlobalDef.h"

#define Util_fabs(x)  (((x) < 0) ? (-(x)) : (x) )
double Util_pow(double x, double y);
void FLASH_SAVE Util_PrintTime(void);

#endif

