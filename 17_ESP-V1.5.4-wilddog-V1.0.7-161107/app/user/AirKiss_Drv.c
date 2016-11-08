
#include "AirKiss_Drv.h"

//保存在FLASH当中的WiFI Station 配置
struct station_config DefaultStationConfig;  
LOCAL os_timer_t StaIsConnTimer;  //定时检查ESP8266是否已联网
static uint16_t StaConnCount = 0;  // 连接计数
static bool  AirKissIsOn = FALSE;   // AisKiss 是否启动




#define DEVICE_TYPE 		"gh_9e2cff3dfa51" //wechat public number
#define DEVICE_ID 			"122475" //model ID

#define DEFAULT_LAN_PORT 	12476

LOCAL esp_udp ssdp_udp;
LOCAL struct espconn pssdpudpconn;
LOCAL os_timer_t ssdp_time_serv;

uint8_t  lan_buf[200];
uint16_t lan_buf_len;
uint8 	 udp_sent_cnt = 0;


#if 1
const airkiss_config_t akconfig =
{
	(airkiss_memset_fn)&memset,
	(airkiss_memcpy_fn)&memcpy,
	(airkiss_memcmp_fn)&memcmp,
	0,
};
#endif

LOCAL void ICACHE_FLASH_ATTR
airkiss_wifilan_time_callback(void)
{
	uint16 i;
	airkiss_lan_ret_t ret;
	
	if ((udp_sent_cnt++) >30) {
		udp_sent_cnt = 0;
		os_timer_disarm(&ssdp_time_serv);//s
		//return;
	}

	ssdp_udp.remote_port = DEFAULT_LAN_PORT;
	ssdp_udp.remote_ip[0] = 255;
	ssdp_udp.remote_ip[1] = 255;
	ssdp_udp.remote_ip[2] = 255;
	ssdp_udp.remote_ip[3] = 255;
	lan_buf_len = sizeof(lan_buf);
	ret = airkiss_lan_pack(AIRKISS_LAN_SSDP_NOTIFY_CMD,
		DEVICE_TYPE, DEVICE_ID, 0, 0, lan_buf, &lan_buf_len, &akconfig);
	if (ret != AIRKISS_LAN_PAKE_READY) {
		AIRKISS_DEBUG("Pack lan packet error!");
		return;
	}
	
	ret = espconn_sendto(&pssdpudpconn, lan_buf, lan_buf_len);
	if (ret != 0) {
		AIRKISS_DEBUG("UDP send error!");
	}
	AIRKISS_DEBUG("Finish send notify!\n");
}

LOCAL void ICACHE_FLASH_ATTR
airkiss_wifilan_recv_callbk(void *arg, char *pdata, unsigned short len)
{
	uint16 i;
	remot_info* pcon_info = NULL;
		
	airkiss_lan_ret_t ret = airkiss_lan_recv(pdata, len, &akconfig);
	airkiss_lan_ret_t packret;
	
	switch (ret){
	case AIRKISS_LAN_SSDP_REQ:
		espconn_get_connection_info(&pssdpudpconn, &pcon_info, 0);
		AIRKISS_DEBUG("remote ip: %d.%d.%d.%d \r\n",pcon_info->remote_ip[0],pcon_info->remote_ip[1],
			                                    pcon_info->remote_ip[2],pcon_info->remote_ip[3]);
		AIRKISS_DEBUG("remote port: %d \r\n",pcon_info->remote_port);
      
        pssdpudpconn.proto.udp->remote_port = pcon_info->remote_port;
		os_memcpy(pssdpudpconn.proto.udp->remote_ip,pcon_info->remote_ip,4);
		ssdp_udp.remote_port = DEFAULT_LAN_PORT;
		
		lan_buf_len = sizeof(lan_buf);
		packret = airkiss_lan_pack(AIRKISS_LAN_SSDP_RESP_CMD,
			DEVICE_TYPE, DEVICE_ID, 0, 0, lan_buf, &lan_buf_len, &akconfig);
		
		if (packret != AIRKISS_LAN_PAKE_READY) {
			AIRKISS_DEBUG("Pack lan packet error!");
			return;
		}

		AIRKISS_DEBUG("\r\n\r\n");
		for (i=0; i<lan_buf_len; i++)
			AIRKISS_DEBUG("%c",lan_buf[i]);
		AIRKISS_DEBUG("\r\n\r\n");
		
		packret = espconn_sendto(&pssdpudpconn, lan_buf, lan_buf_len);
		if (packret != 0) {
			AIRKISS_DEBUG("LAN UDP Send err!");
		}
		
		break;
	default:
		AIRKISS_DEBUG("Pack is not ssdq req!%d\r\n",ret);
		break;
	}
}

void ICACHE_FLASH_ATTR
airkiss_start_discover(void)
{
	ssdp_udp.local_port = DEFAULT_LAN_PORT;
	pssdpudpconn.type = ESPCONN_UDP;
	pssdpudpconn.proto.udp = &(ssdp_udp);
	espconn_regist_recvcb(&pssdpudpconn, airkiss_wifilan_recv_callbk);
	espconn_create(&pssdpudpconn);

	os_timer_disarm(&ssdp_time_serv);
	os_timer_setfn(&ssdp_time_serv, (os_timer_func_t *)airkiss_wifilan_time_callback, NULL);
	os_timer_arm(&ssdp_time_serv, 1000, 1);//1s
}


static void ICACHE_FLASH_ATTR
//smartconfig_done(sc_status status, void *pdata)
SmartConfig_Done(sc_status status, void *pdata)
{
    switch(status) {
        case SC_STATUS_WAIT:
            AIRKISS_DEBUG("SC_STATUS_WAIT\n");
            break;
        case SC_STATUS_FIND_CHANNEL:
            AIRKISS_DEBUG("SC_STATUS_FIND_CHANNEL\n");
            break;
        case SC_STATUS_GETTING_SSID_PSWD:
            AIRKISS_DEBUG("SC_STATUS_GETTING_SSID_PSWD\n");
			sc_type *type = pdata;
            if (*type == SC_TYPE_ESPTOUCH) {
                AIRKISS_DEBUG("SC_TYPE:SC_TYPE_ESPTOUCH\n");
            } else {
                AIRKISS_DEBUG("SC_TYPE:SC_TYPE_AIRKISS\n");
            }
            break;
        case SC_STATUS_LINK:
            AIRKISS_DEBUG("SC_STATUS_LINK\n");
            struct station_config *sta_conf = pdata;
	
	        wifi_station_set_config(sta_conf);
	        wifi_station_disconnect();
	        wifi_station_connect();
            break;
        case SC_STATUS_LINK_OVER:
            AIRKISS_DEBUG("SC_STATUS_LINK_OVER\n");
            if (pdata != NULL) {
				//SC_TYPE_ESPTOUCH
                uint8 phone_ip[4] = {0};

                os_memcpy(phone_ip, (uint8*)pdata, 4);
                AIRKISS_DEBUG("Phone ip: %d.%d.%d.%d\n",phone_ip[0],phone_ip[1],phone_ip[2],phone_ip[3]);
            } else {
            	//SC_TYPE_AIRKISS - support airkiss v2.0
				airkiss_start_discover();
            }
            smartconfig_stop();
            break;
    }
	
}

/*****************************************************************************
 * @\fn  : CheckStationConnectStatus
 * @\author : Wen Yucong
 * @\date : 2016 - 7 - 9
 * @\brief :  Timer to check ESP8266 is get IP or not
 * @\param[in] : void *arg  
 * @\param[out] : none
 * @\return : 
 * @\attention : 
 * @\note [others] : 

*****************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
AirKiss_CheckStationConnectStatus(void *arg)
{
    bool Result = false;  
    uint8_t StationStatus = 0;
	
    memset((void *)&DefaultStationConfig, 0, sizeof(DefaultStationConfig));
	Result = wifi_station_get_config_default(&DefaultStationConfig);
	AIRKISS_DEBUG("get_config_default = %d\n", (uint8_t)Result);
	if(false == Result)  // FLASH 中没有保存可以连接的WIFI 热点
	{
	   if( ! AirKissIsOn)
	   {
StartAirKiss:   
	       //启动 AirKiss
	       /************************AirKiss  init *******************************/
		   AirKissIsOn = TRUE;
		   
		   AIRKISS_DEBUG("\r\nAirKiss Start, StatCount = %d\r\n", StaConnCount);
		   
           smartconfig_set_type(SC_TYPE_AIRKISS); //SC_TYPE_ESPTOUCH,SC_TYPE_AIRKISS,SC_TYPE_ESPTOUCH_AIRKISS
           wifi_set_opmode(STATION_MODE);
           //smartconfig_start(smartconfig_done);
	       smartconfig_start(SmartConfig_Done);
		}
	}
	else
	{
	    StationStatus = wifi_station_get_connect_status();
		if(StationStatus != STATION_CONNECTING && StationStatus != STATION_GOT_IP)
		{
		    //启动定时器回调本函数
		    os_timer_disarm(&StaIsConnTimer);
            os_timer_setfn(&StaIsConnTimer, (os_timer_func_t *)AirKiss_CheckStationConnectStatus, NULL);
            os_timer_arm(&StaIsConnTimer, 2000, 0);  // 2 s定时
            StaConnCount++; //连接次数计数

			AIRKISS_DEBUG("Station Conn Count = %d\n", StaConnCount);
			if(StaConnCount > 10)
			{
			   StaConnCount = 0;
			   os_timer_disarm(&StaIsConnTimer);
			   goto StartAirKiss;
			}
		}
		else
		{
		   AIRKISS_DEBUG("Station Status = %d\n", StationStatus);
		   AIRKISS_DEBUG("AirKiss Status = %d\n", (uint8_t)AirKissIsOn);
		}
	}
}

/*****************************************************************************
 * @\fn  : AirKiss_Init
 * @\author : Wen Yucong
 * @\date : 2016 - 7 - 9
 * @\brief : AirKiss Init 
 * @\param[in] : void  
 * @\param[out] : none
 * @\return : static
 * @\attention : 
 * @\note [others] : 

*****************************************************************************/
void ICACHE_FLASH_ATTR AirKiss_Init(void)
{
	AirKiss_CheckStationConnectStatus(NULL);
}

//static os_timer_t tNetTimer;
uint8_t FLASH_SAVE AirKiss_IsNetConnected(void)
{
    struct ip_info ipconfig;

   //disarm timer first
  //  os_timer_disarm(&tNetTimer);

   //get ip info of ESP8266 station
    wifi_get_ip_info(STATION_IF, &ipconfig);

    if (wifi_station_get_connect_status() == STATION_GOT_IP &&  \
            ipconfig.ip.addr != 0) 
    {
        return 1;
    }
	else
	{
	    return 0;
	}
}
