
#ifndef __GIZCMDTEST_H__
#define __GIZCMDTEST_H__

#include "giz_includes.h"


#if TEST_DEBUG_EN
#define TEST_DEBUG  os_printf
#else
#define TEST_DEBUG(...) 
#endif


void FLASH_SAVE GizCmdTestInit(void);

#if (GIZ_PLATFORM == USE_ARDUINO)  // device MCU API
void FLASH_SAVE CmdTest_DevCurStatusReportReq_Manager(void);
void FLASH_SAVE CmdTest_SendRestartWifiReq_Manager(void);
void FLASH_SAVE CmdTest_SendConfigModeReq_Manager(void);
#else  // wifi module API
void FLASH_SAVE CmdTest_WifiTimingToCheckStatus_Manager(void);
void FLASH_SAVE CmdTest_SendReadDevCurStatusReq_Manager(void);
void FLASH_SAVE CmdTest_TimeSetReqTimer_Manager(void);
#endif


#endif

