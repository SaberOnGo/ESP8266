
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

//#if ESP_PLATFORM
//#include "user_esp_platform.h"
//#endif


#include "user_tcpclient.h"
#include "user_httpclient.h"

#include "AtCmd_Drv.h"
#include "AirKiss_Drv.h"
#include "WildDog_Drv.h"


void user_rf_pre_init(void)
{
}




void user_init(void)
{
    ATCMD_Init();
    GizCmdTestInit();

	AirKiss_Init();

	user_set_station_config();  // TCP Client, ¿ªÆô DHCP Ä£Ê½

	//HttpClient_TestInit(NULL);
	//http_test();
	
    WD_Init();
}


