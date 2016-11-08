#ifndef __GLOBAL_DEF_H__
#define __GLOBAL_DEF_H__

#include "Osapi.h"

#if 1
#define  SSID         "NETGEAR94"
#define  PASSWORD     "heavygadfly334"
#else
#define  SSID         "MERCURY_2786"
#define  PASSWORD     "WYCjiasen31415(@^wtf"
#endif

#define  TEST_URL     "coaps://esp8266-wae1024.wilddogio.com/"
#define  TEST_LED     "MyWildDogDemo"

#define  DEF_LED_HARDWARE 1


#define FLASH_SAVE ICACHE_FLASH_ATTR
#define RO_DATA_IN_FLASH  ICACHE_RODATA_ATTR
// if enable all the following debug switch at the same time, RAM will be run out, so be careful !
// debug printf enable(1) or disable(0)
#define  DEBUG_PRINT_EN      0
#define  WILDDOG_DEBUG_EN    1
#define  AIRKISS_DEBUG_EN    1
#define  GIZ_DEBUG_EN        1    // giz module
#define  SERIAL_DEBUG_EN     1    // giz module
#define  TEST_DEBUG_EN       1    // giz module
#define  HTTP_DEBUG_EN       0
#define  WEBSERVER_DEBUG_EN  0
#define  TCP_CLIENT_DEBUG_EN 0

typedef enum
{
   APP_SUCCESS = 0,
   APP_FAILED = 1,
   APP_NULL = 0xFF,
}E_RESULT;

typedef enum
{
   E_TRUE = 1,
   E_FALSE = 0,
}E_BOOL;

typedef enum
{
   OS_OK = 0,
   OS_ERROR = 1,
}OS_RESULT;
#if  DEBUG_PRINT_EN
#define APP_DEBUG_PRINT  os_printf
#else
#define APP_DEBUG_PRINT(...)
#endif
#if  WILDDOG_DEBUG_EN
#define WDJSON_DEBUG  os_printf
#else
#define WDJSON_DEBUG(...)
#endif
//*************************** GIZ MODULE BEGIN ******************************/
#define  USE_ARDUINO    0
#define  USE_ESP8266    1
#define GIZ_PLATFORM      USE_ESP8266
#define GIZ_CMD_TEST_EN    1    //  do not touch it !
#define ALL_STRING_SAVE_IN_FLASH      0 
//*************************** GIZ MODULE END   *****************************/
#endif
