
#include "GizCmdTest.h"



#if GIZ_CMD_TEST_EN

#if (GIZ_PLATFORM == USE_ARDUINO)
static os_timer_t tSendCfgModeReqTimer;
static void FLASH_SAVE CfgModeReqTimer_CallBack(void *param)
{
   TEST_DEBUG("CfgModeReqCb %d ms\n", Sys_GetRunTime());
   os_timer_disarm(&tSendCfgModeReqTimer);
   if(E_FALSE == GetRxCfgModeReqFlag())  // had not get the resp
   {
      CMDFN_SendConfigMode_Req(E_WIFI_SoftAp);  // send req msg to wifi module again
      os_timer_setfn(&tSendCfgModeReqTimer, (os_timer_func_t *)CfgModeReqTimer_CallBack, NULL);
      os_timer_arm(&tSendCfgModeReqTimer, 2000, 0);   
      return;
   }
   else
   {
      SetRxCfgModeReqFlag(E_FALSE);
   }
   TEST_DEBUG("rx CfgModeReq, stop timer tk = %d\n", Sys_GetRunTime());
}
void FLASH_SAVE CmdTest_SendConfigModeReq_Manager(void)
{
    TEST_DEBUG("CfgModeReq_Mag\n");
    CMDFN_SendConfigMode_Req(E_WIFI_SoftAp);
	os_timer_disarm(&tSendCfgModeReqTimer);
	os_timer_setfn(&tSendCfgModeReqTimer, (os_timer_func_t *)CfgModeReqTimer_CallBack, NULL);
	os_timer_arm(&tSendCfgModeReqTimer, 2000, 0);  // 1s timer
}

static os_timer_t tRestartWifiReqMsgTimer;
static void FLASH_SAVE RestartWifiReqTimer_CallBack(void *param)
{
   os_timer_disarm(&tRestartWifiReqMsgTimer);
   TEST_DEBUG("RestartWifiReqCb, %d ms\n", Sys_GetRunTime());
   if(E_FALSE == GetRxRestartWifiReqFlag())  // had not get the resp
   {
      CMDFN_SendRestartWifi_Req();  // send req msg to wifi module again
      os_timer_setfn(&tRestartWifiReqMsgTimer, (os_timer_func_t *)RestartWifiReqTimer_CallBack, NULL);
      os_timer_arm(&tRestartWifiReqMsgTimer, 3000, 0);   
      return;
   }
   else
   {
      SetRxRestartWifiReqFlag(E_FALSE);
   }
   TEST_DEBUG("rx restart wifi req, stop timer %d ms\n", Sys_GetRunTime());
}
void FLASH_SAVE CmdTest_SendRestartWifiReq_Manager(void)
{
    CMDFN_SendRestartWifi_Req();
	os_timer_disarm(&tRestartWifiReqMsgTimer);
	os_timer_setfn(&tRestartWifiReqMsgTimer, (os_timer_func_t *)RestartWifiReqTimer_CallBack, NULL);
	os_timer_arm(&tRestartWifiReqMsgTimer, 3000, 0); 
}
static os_timer_t tDevCurStatusReportTimer;
static void FLASH_SAVE DevCurStatusReportTimer_CallBack(void * param)
{
     os_timer_disarm(&tDevCurStatusReportTimer);
     TEST_DEBUG("report DevCurSta cb tk = %d ms\n", Sys_GetRunTime());
	 CMDFN_SendReadDeviceCurStatusReport();
	 os_timer_setfn(&tDevCurStatusReportTimer, (os_timer_func_t * )DevCurStatusReportTimer_CallBack, NULL);
	 os_timer_arm(&tDevCurStatusReportTimer, 2000, 0);
}
void FLASH_SAVE CmdTest_DevCurStatusReportReq_Manager(void)
{
    os_timer_disarm(&tDevCurStatusReportTimer);
	os_timer_setfn(&tDevCurStatusReportTimer, (os_timer_func_t * )DevCurStatusReportTimer_CallBack, NULL);
	os_timer_arm(&tDevCurStatusReportTimer, 2000, 0);
}
#else  // wifi module
static os_timer_t tWifiTimingToCheckStatusTimer;
static void FLASH_SAVE WifiTimingToCheckStatus_CallBack(void *param)
{
    uint16_t wifi_status = 0;
    static uint16_t last_status = 0;
    uint16_t temp_status = 0;
	uint8_t result = 0;

	TEST_DEBUG("wifi check sta cb,tk = %d\n", Sys_GetRunTime());
	result = wifi_get_opmode() & 0x03;
	if(result == STATION_MODE || result == STATIONAP_MODE)temp_status |= WIFI_BIT_IsStationOn; // station is on
    else if(result == SOFTAP_MODE || result == STATIONAP_MODE)temp_status |= WIFI_BIT_IsSoftApOn; // soft AP is on
	if(my_wifi_is_config_mode_on() == E_TRUE)temp_status |= WIFI_BIT_IsCfgModeOn;
	if(my_wifi_is_binding_on() == E_TRUE)temp_status |= WIFI_BIT_IsBindingOn;
	result = wifi_station_get_connect_status();
	if(result == STATION_CONNECTING || result == STATION_GOT_IP)temp_status |= WIFI_BIT_IsConnectAp;
	if(my_wifi_is_connect_cloud() == E_TRUE)temp_status |= WIFI_BIT_IsConnectCloud;
	if(temp_status & WIFI_BIT_IsConnectAp)
	{
	   result = wifi_station_get_rssi();
	   if(result >= 10){} // get rssi failed, do nothing
	   else if(result >= 7) temp_status |= (((uint16_t)7) << WIFI_BIT_RSSI_OFFSET);
	   else{ temp_status |= ((uint16_t)result << WIFI_BIT_RSSI_OFFSET); }
	}
	if(temp_status != last_status)  // wifi status changed
	{
	   TEST_DEBUG("wifi sta changed, sta = 0x%x\n", (uint32_t)temp_status);
	   result = CMDFN_SendWifiStatusChangeNotify_Req(temp_status);
	   if(APP_SUCCESS == result)last_status = temp_status;
	}
}
void FLASH_SAVE CmdTest_WifiTimingToCheckStatus_Manager(void)
{
	os_timer_disarm(&tWifiTimingToCheckStatusTimer);
	os_timer_setfn(&tWifiTimingToCheckStatusTimer, (os_timer_func_t *)WifiTimingToCheckStatus_CallBack, NULL);
	os_timer_arm(&tWifiTimingToCheckStatusTimer, 5000, 1); 
}
static os_timer_t tReadDevCurStatusReqTimer;
static void FLASH_SAVE ReadDevCurStatusTimer_CallBack(void * param)
{
   TEST_DEBUG("read dev cur sta cb tk = %d\n", Sys_GetRunTime());
   CMDFN_SendReadDeviceCurStatus_Req();
}
void FLASH_SAVE CmdTest_SendReadDevCurStatusReq_Manager(void)
{
   os_timer_disarm(&tReadDevCurStatusReqTimer);
   os_timer_setfn(&tReadDevCurStatusReqTimer, (os_timer_func_t * )ReadDevCurStatusTimer_CallBack, NULL);
   os_timer_arm(&tReadDevCurStatusReqTimer, 1000 * 30, 1);
}
static os_timer_t tTimeSetReqTimer;
static void FLASH_SAVE TimeSetReqTimer_CallBack(void *param)
{
   TEST_DEBUG("TimeSetReqCb %d ms\n", Sys_GetRunTime());
   os_timer_disarm(&tTimeSetReqTimer);
   if(E_FALSE == GetRxTimeSetReqFlag())
   {
   CMDFN_SendDeviceTimeSet_Req();
   os_timer_setfn(&tTimeSetReqTimer, (os_timer_func_t *)TimeSetReqTimer_CallBack, NULL);
   os_timer_arm(&tTimeSetReqTimer, 3000, 0);
	  return;
   }
   else
   {
      SetRxTimeSetReqFlag(E_FALSE);
   }
   TEST_DEBUG("rx TimeSetReq stop timer\n");
}
void FLASH_SAVE CmdTest_TimeSetReqTimer_Manager(void)
{
   CMDFN_SendDeviceTimeSet_Req();
   os_timer_disarm(&tTimeSetReqTimer);
   os_timer_setfn(&tTimeSetReqTimer, (os_timer_func_t *)TimeSetReqTimer_CallBack, NULL);
   os_timer_arm(&tTimeSetReqTimer, 3000, 0);
}
#endif   // end (GIZ_PLATFORM == USE_ARDUINO)

#endif  // GIZ_CMD_TEST_EN

void FLASH_SAVE GizCmdTestInit(void)
{
    #if GIZ_CMD_TEST_EN
	#if (GIZ_PLATFORM == USE_ARDUINO)
    CmdTest_SendConfigModeReq_Manager();
	CmdTest_SendRestartWifiReq_Manager();
	CmdTest_DevCurStatusReportReq_Manager();
	#else  // wifi module
	//CmdTest_WifiTimingToCheckStatus_Manager();
	//CmdTest_SendReadDevCurStatusReq_Manager();
	//CmdTest_TimeSetReqTimer_Manager();
	#endif  
    #if 1
	if(E_TRUE == Sys_IsBigEndian())
	{
	   os_printf("CPU is big endian: MSB in low addr\n");
	}
	else
	{
	   os_printf("CPU is little endian: LSB in low addr\n");
	}
	#endif  
	#endif
}

