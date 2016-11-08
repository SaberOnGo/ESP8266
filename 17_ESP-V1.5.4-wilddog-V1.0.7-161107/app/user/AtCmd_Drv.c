
#include "AtCmd_Drv.h"









/****************************************************************************************************
机智云接入串口协议
命令格式类似如下:
header(2B) = 0xFFFF, len(2B),  cmd(1B), sn(1B), params(x B), checksum(1B)
1:  header 固定为0xFFFF
2:  长度len 是指从 cmd开始到整个数据包结束所占用的字节数
3:  多于一个字节的整型数字以大端字节序编码
4   消息序号(sn)由发送方给出，接收方响应命令时需把消息序号返回给发送方
5:  检验和(checksum)的计算方式为把数据包按字节求和得出的结果取出最低字节的内容
6:  所有发送的命令都带有确认，如在200毫秒内没有收到接收方的响应，发送方应重发，最多重发3次

附加约定:  报文的调制与解调:
7   未调制前的报文成为原报文, 调制后的报文称为新报文, 校验码的值是对旧报文运算的结果
8   调制: 报文中出现一个字节的值为0xCC, 则填0xDD (包括校验码), 如果为0x00, 则规定填充为0xCC
9:  解调: 算法一:  报文中出现0xCC, 则恢复为0x00, 如果出现0xDD, 则恢复为0xCC, 此时检查校验码
                  如果第一种算法中, 校验码对报文校验错误, 则运行算法2:
                  报文中出现0xCC, 恢复为0x00, 如果出现0xDD, 则不处理, 此时检查校验码
*****************************************************************************************************/

// 消息解调
static E_RESULT FLASH_SAVE GIZ_MsgDemodulate(uint8_t * data, int16_t len, uint8_t *outCmd)
{
   uint16_t checkSumLen = 0;    // all msg data except the byte of checksum
   uint16_t validLen = 0;  // valid len = header.len
   uint16_t i;
   uint8_t xorResult = 0;  //the checksum result
   T_MSG_HEADER *header = (T_MSG_HEADER *)data;
 
   if(NULL == data)
   {
      GIZ_PRINT("err, %s, %d\n", __FILE__, __LINE__);
	  return APP_FAILED;
   }
   
   *outCmd = CMD_Reserved;
   if(data[0] == GIZ_HEADER_VAL && data[1] == GIZ_HEADER_VAL)
   {
      // demodulate msg len first
      for(i = 0; i < 4; i++)
      {
         if(data[i] == GIZ_CODE_1_VAL)
         {
            data[i] = 0x00;   // if the value is 0xCC,then recover to 0x00
         }
		 else if(data[i] == GIZ_CODE_2_VAL)
		 {
		   data[i] = GIZ_CODE_1_VAL;  // if the value is 0xDD,then recover to 0xCC
		 }
		 else if(data[i] == GIZ_CODE_3_VAL)
		 {
		    data[i] = 0x0A;
		 }
		 xorResult += data[i];
      }
	  validLen = header->len;
	  checkSumLen = validLen + MSG_PACKET_HEAD_SIZE - GIZ_CHECKSUM_SIZE; 

      GIZ_PRINT("pklen = %d, ckLen = %d\n", validLen, checkSumLen);
      //  check checkSumLen is valid?
      if(checkSumLen > len)
      {
          GIZ_PRINT("err ckLen = %d, real len = %d\n", checkSumLen, len);
		  
		  return APP_FAILED;
      }
	  
      for(i = 4; i < checkSumLen; i++)  // demodulate the left bytes
      {
         if(data[i] == GIZ_CODE_1_VAL)
         {
            data[i] = 0x00;   // if the value is 0xCC,then recover to 0x00
         }
		 else if(data[i] == GIZ_CODE_2_VAL)
		 {
		   data[i] = GIZ_CODE_1_VAL;  // if the value is 0xDD,then recover to 0xCC
		 }
		 else if(data[i] == GIZ_CODE_3_VAL)
		 {
		    data[i] = 0x0A;
		 }
		 xorResult += data[i];
      }

	  if(len)
	  {
	     #if GIZ_DEBUG_EN  
	     GIZ_PRINT("newXor = 0x%x, oldXor = 0x%x\n", xorResult, data[checkSumLen]);
		 for(i = 0; i < (checkSumLen + 1); i++)
		 {
		  	 GIZ_PRINT("0x%x ", data[i]);
		 }
		 GIZ_PRINT("\r\n");
         #endif
		
	     if(data[checkSumLen] == xorResult)  // compare checksum
	     {
			 GIZ_PRINT("GIZ: decode ok cmd = 0x%x\n", header->cmd);
			*outCmd = header->cmd;   // get cmd code
			
	        return APP_SUCCESS;
	     }
		 else
		 {
		    GIZ_PRINT("GIZ: xor error, msg: \n");
			return APP_FAILED;
		 }
	  }
	  else
	  {
	     GIZ_PRINT("GIZ: error, len < 0\n");
		 return APP_FAILED;
	  } 
   }
   else
   {
      GIZ_PRINT("not giz msg\n");
      return APP_FAILED;
   }
   return APP_SUCCESS;
}


// 消息调制
/*
7   未调制前的报文成为原报文, 调制后的报文称为新报文, 校验码的值是对旧报文运算的结果
8   调制: 报文中出现一个字节的值为0xCC, 则填0xDD (包括校验码), 如果为0x00, 则规定填充为0xCC
9:  解调: 算法一:  报文中出现0xCC, 则恢复为0x00, 如果出现0xDD, 则恢复为0xCC, 此时检查校验码
                  如果第一种算法中, 校验码对报文校验错误, 则运行算法2:
                  报文中出现0xCC, 恢复为0x00, 如果出现0xDD, 则不处理, 此时检查校验码
*/
static E_RESULT FLASH_SAVE GIZ_MsgModulate(void *pMsg, int16_t totalLen)
{
   uint8_t *data = (uint8_t *)pMsg;
   uint16_t i;

   for(i = 0; i < totalLen; i++)
   {
      if(0x0A == data[i])
      {
         data[i] = GIZ_CODE_3_VAL;
      }
      if(GIZ_CODE_1_VAL == data[i])
      {
         data[i] = GIZ_CODE_2_VAL;
      }
	  else if(0x00 == data[i])
	  {
	     data[i] = GIZ_CODE_1_VAL;
	  }
   }

   return APP_SUCCESS;
}

// 判断报文是否是机智云串口协议命令
static E_RESULT FLASH_SAVE GIZ_IsCmdCorrect(uint8_t * data, int16_t len, uint8_t *outCmd)
{
   return GIZ_MsgDemodulate(data, len, outCmd);
}

/*****************************************************************************
 * @\fn  : GIZ_FillMsgHeader
 * @\author : Wen Yucong
 * @\date : 2016 - 10 - 21
 * @\brief : 报文固定的帧头填写
 * @\param[in] : void * pMsg   报文起始指针
               uint16_t len  : 报文有效长度, 从cmd字段开始, 不包括header, len字段, 包括checksum 校验字段
               uint8_t cmd    命令码
               uint8_t sn       消息序号
 * @\param[out] :   none
 * @\return : 构造结果: APP_SUCCESS or APP_FAILED
 * @\attention : 
 * @\note [others] : 

*****************************************************************************/
static E_RESULT FLASH_SAVE GIZ_FillMsgHeader(void * pMsg, uint16_t len, uint8_t cmd, uint8_t sn)
{
   T_MSG_HEADER *header = (T_MSG_HEADER *)pMsg;

   if(NULL == pMsg)
   {
      GIZ_PRINT("err %s %d\n", __FILE__, __LINE__);
	  return APP_FAILED;
   }
   header->head = 0xFFFF;
   header->len = len;
   header->cmd = cmd;
   header->sn  = sn;
   header->flags = 0x0000;

   GIZ_PRINT("header:cmd:0x%x len:0x%x\n", header->cmd, header->len);
   return APP_SUCCESS;
}

// 填充消息尾, 即校验值
static void FLASH_SAVE GIZ_FillMsgTailer(void *pMsg)
{
   uint8_t xorResult = 0;
   uint16_t checksumLen = 0;
   uint16_t i;
   uint8_t *data = (uint8_t *)pMsg;
   T_MSG_HEADER *header = (T_MSG_HEADER *)pMsg;
   	
   if(NULL == pMsg)
   {
      GIZ_PRINT("err %s %d\n", __FILE__, __LINE__);
	  return ;
   }
   checksumLen =  MSG_PACKET_HEAD_SIZE  + header->len - GIZ_CHECKSUM_SIZE;
   GIZ_PRINT("cmd = 0x%x, cklen = %d\n", header->cmd, checksumLen);
   for(i = 0; i < checksumLen; i++)
   {
      xorResult += data[i];
	  
   }
   data[checksumLen] = xorResult;
}

// 产生序列号
static uint8_t FLASH_SAVE GIZ_GenSerial(void)
{
   static uint8_t serial = 0;

   return serial++;
}



// 构造设备请求命令消息
#define CMDFN_ConstructGeneralMsg(pTxMsg, T_CMD_TYPE, cmd_val) \
	{\
	GIZ_PRINT("sizeof(");\
	GIZ_PRINT(#T_CMD_TYPE);\
	GIZ_PRINT(") = %d\n", sizeof(T_CMD_TYPE));\
	GIZ_FillMsgHeader(pTxMsg, sizeof(T_CMD_TYPE) - MSG_PACKET_HEAD_SIZE, cmd_val, GIZ_GenSerial());\
	GIZ_FillMsgTailer(pTxMsg);\
	GIZ_MsgModulate(pTxMsg, sizeof(T_CMD_TYPE));\
	}

static uint8_t FLASH_SAVE CMDFN_MakeNothing(void * pMsg)
{
   return APP_SUCCESS;
}
static uint8_t FLASH_SAVE CMDFN_ParseNothing(void *pRxMsg)
{
   return APP_SUCCESS;
}
/*****************************************************************************
 * @\fn  : CMDFN_MakeDeviceInfo_Req
 * @\author : Wen Yucong
 * @\date : 2016 - 10 - 11
 * @\brief : WIFI 模组请求外部设备信息
 * @\param[in] : void  
 * @\param[out] : none
 * @\return : static
 * @\attention : 
 * @\note [others] :  消息已调制

*****************************************************************************/
static uint8_t FLASH_SAVE CMDFN_MakeDeviceInfo_Req(void *pTxMsg)
{
   uint8_t buf[sizeof(T_DeviceInfo_Req) + 5];
   uint16_t i, len;
   
   os_memset(buf, 0, sizeof(buf));
   CMDFN_ConstructGeneralMsg(buf, T_DeviceInfo_Req, CMD_DeviceInfo_Req);
   len = os_strlen(buf);
   os_strncpy(&buf[len], "\r\n", 2);  // to remark the end of the msg
   GIZ_UartSend(buf);   // 消息发送

   #if GIZ_DEBUG_EN
   GIZ_PRINT("DevInfoReq %d ms\n", Sys_GetRunTime());
   len = os_strlen(buf);
   for(i = 0; i < len; i++)
   {
      GIZ_PRINT("0x%x ", buf[i]);
   }
   GIZ_PRINT("\n");
   #endif
   
   return APP_SUCCESS;
}

static uint8_t FLASH_SAVE CMDFN_ParseDeviceInfo_Req(void *pRxMsg)
{
   uint8_t buf[sizeof(T_DeviceInfo_Resp) + 5];
   T_DeviceInfo_Req *pReqCmd = (T_DeviceInfo_Req *)(pRxMsg);
   T_DeviceInfo_Resp *pRespCmd = (T_DeviceInfo_Resp *)buf;
   uint16_t len = 0;

   if(NULL == pRxMsg)
   {
      GIZ_PRINT("err %s %d\n", __FILE__, __LINE__);
	  return APP_FAILED;
   }
   GIZ_PRINT("parse DevInfoReq %d ms\n", Sys_GetRunTime());
   os_memset(buf, 0, sizeof(buf));
   if(sizeof(buf) < (sizeof(T_DeviceInfo_Resp) + 2))
   {
      GIZ_PRINT("err %s %d\n", __FILE__, __LINE__);
	  return APP_FAILED;
   }
   
   GIZ_FillMsgHeader(pRespCmd, sizeof(T_DeviceInfo_Resp) - MSG_PACKET_HEAD_SIZE, CMD_DeviceInfo_Resp, pReqCmd->header.sn);
   Sys_GetVersion(pRespCmd->protocol_ver, sizeof(pRespCmd->protocol_ver), E_PROTOCOL_VER);
   Sys_GetVersion(pRespCmd->private_ver,  sizeof(pRespCmd->private_ver),  E_PRIVATE_VER);
   Sys_GetVersion(pRespCmd->hard_ver,     sizeof(pRespCmd->hard_ver),      E_HARD_VER);
   Sys_GetVersion(pRespCmd->soft_ver,     sizeof(pRespCmd->soft_ver),      E_SOFT_VER);
   Sys_GetVersion(pRespCmd->product_key,  sizeof(pRespCmd->product_key),  E_PRODUCT_KEY);
   pRespCmd->binable_timeout = 0;  // if you need, change it
   GIZ_FillMsgTailer(pRespCmd);
   GIZ_MsgModulate(pRespCmd, sizeof(T_DeviceInfo_Resp));
   len = os_strlen(buf);
   os_strncpy(&buf[len], "\r\n", 2);  // to remark the end of the msg
   
   GIZ_UartSend(buf);
   
   return APP_SUCCESS;
}

// WIFI 模组收到外部设备的设备信息的响应
static os_timer_t tDeviceInfoReqManagerTimer;
static E_BOOL isGetDeviceInfoResp = E_FALSE;
static uint8_t FLASH_SAVE CMDFN_ParseDeviceInfo_Resp(void *pRxMsg)
{
    GIZ_PRINT("parse DevInfoResp %d ms\n", Sys_GetRunTime());
    #if (GIZ_PLATFORM == USE_ESP8266)
    T_DeviceInfo_Resp *pCmd = (T_DeviceInfo_Resp *)pRxMsg;
    uint8_t string[36];
	
    if(NULL == pRxMsg)
    {
       GIZ_PRINT("err %s %d\n", __FILE__, __LINE__);
	   return APP_FAILED;
    }
	
	isGetDeviceInfoResp = E_TRUE;
	
	os_memset(string, 0, sizeof(string));
	os_strncpy(string, pCmd->protocol_ver, sizeof(pCmd->protocol_ver));
	GIZ_PRINT("pro_ver = %s\n", string);

	os_memset(string, 0, sizeof(string));
	os_strncpy(string, pCmd->private_ver, sizeof(pCmd->private_ver));
	GIZ_PRINT("pri_ver = %s\n", string);

	os_memset(string, 0, sizeof(string));
	os_strncpy(string, pCmd->hard_ver, sizeof(pCmd->hard_ver));
	GIZ_PRINT("hard_ver = %s\n", string);

	os_memset(string, 0, sizeof(string));
	os_strncpy(string, pCmd->soft_ver, sizeof(pCmd->soft_ver));
	GIZ_PRINT("soft_ver = %s\n", string);

	os_memset(string, 0, sizeof(string));
	os_strncpy(string, pCmd->product_key, sizeof(pCmd->product_key));
	GIZ_PRINT("soft_ver = %s\n", string);

	GIZ_PRINT("bt = 0x%x\n", (uint32_t)pCmd->binable_timeout);
	#endif
	
	return APP_SUCCESS;
}

static void FLASH_SAVE DeviceInfoReqMsgTimer_CallBack(void *arg)
{
   GIZ_PRINT("DevInfoReqCb %d ms\n", Sys_GetRunTime());
   #if (GIZ_PLATFORM == USE_ESP8266)
   os_timer_disarm(&tDeviceInfoReqManagerTimer);
   if(E_FALSE == isGetDeviceInfoResp)  // had not get the device info
   {
      CMDFN_MakeDeviceInfo_Req(NULL);  // send req msg to the device
      os_timer_setfn(&tDeviceInfoReqManagerTimer, (os_timer_func_t *)DeviceInfoReqMsgTimer_CallBack, NULL);
      os_timer_arm(&tDeviceInfoReqManagerTimer, 1000, 0);   // 1 s timer
      return;
   }
   else
   {
      isGetDeviceInfoResp = E_FALSE;
   }
   GIZ_PRINT("rx DevInfo %d ms\n", Sys_GetRunTime());
   #endif
}
static void FLASH_SAVE ATCMD_DeviceInfoReqTimerManager(void)
{
    #if (GIZ_PLATFORM == USE_ESP8266)
    os_timer_disarm(&tDeviceInfoReqManagerTimer);
    os_timer_setfn(&tDeviceInfoReqManagerTimer, (os_timer_func_t *)DeviceInfoReqMsgTimer_CallBack, NULL);
    os_timer_arm(&tDeviceInfoReqManagerTimer, 1000, 0);   // 1 s timer
    #endif
}

// WIFI 模组对外部设备发送心跳请求
static os_timer_t tHeartTickReqTimer;
static uint8_t FLASH_SAVE CMDFN_MakeHeartTick_Req(void *pRxMsg)
{
    uint8_t buf[sizeof(T_HeartTick_Req) + 5];
    uint8_t len = 0;
	 
	os_memset(buf, 0, sizeof(buf));
    CMDFN_ConstructGeneralMsg(buf, T_HeartTick_Req, CMD_HeartTick_Req);  // 消息生成
    len = os_strlen(buf);
	os_strncpy(&buf[len], "\r\n", 2);
    GIZ_UartSend(buf);    // send msg to outside device

    return APP_SUCCESS;
}
// WIFI 模组收到外部设备的心跳响应
static uint8_t FLASH_SAVE CMDFN_ParseHeartTick_Req(void *pRxMsg)
{
   uint8_t buf[sizeof(T_HeartTick_Resp) + 5];
   T_HeartTick_Req *pReqCmd = (T_HeartTick_Req *)(pRxMsg);
   T_HeartTick_Resp *pRespCmd = (T_HeartTick_Resp *)buf;
   uint16_t len = 0;
   if(NULL == pRxMsg)
   {
      GIZ_PRINT("err %s %d\n", __FILE__, __LINE__);
	  return APP_FAILED;
   }
   GIZ_PRINT("parse HtReq %d ms\n", Sys_GetRunTime());
   os_memset(buf, 0, sizeof(buf));
   GIZ_FillMsgHeader(pRespCmd, sizeof(T_HeartTick_Resp) - MSG_PACKET_HEAD_SIZE, CMD_HeartTick_Resp, pReqCmd->header.sn);
   GIZ_FillMsgTailer(pRespCmd);
   GIZ_MsgModulate(pRespCmd, sizeof(T_HeartTick_Resp));
   len = os_strlen(buf);
   os_strncpy(&buf[len], "\r\n", 2);  // to remark the end of the msg
   GIZ_UartSend(buf);  
   return APP_SUCCESS;
}
static uint8_t FLASH_SAVE CMDFN_ParseHeartTick_Resp(void *pRxMsg)
{
    T_HeartTick_Resp * pCmd = (T_HeartTick_Resp *)pRxMsg;
	GIZ_PRINT("parse HtResp\n");
	return APP_SUCCESS;
}
static void FLASH_SAVE HeartTickReqTimer_CallBack(void *arg)
{
    GIZ_PRINT("HtReqCb %d ms\n", Sys_GetRunTime());
	#if(GIZ_PLATFORM == USE_ESP8266)
    os_timer_disarm(&tHeartTickReqTimer);
	CMDFN_MakeHeartTick_Req(NULL);
    os_timer_setfn(&tHeartTickReqTimer, (os_timer_func_t *)HeartTickReqTimer_CallBack, NULL);
    os_timer_arm(&tHeartTickReqTimer, 1000 * 170, 0);   // 180 s timer
    #endif
}
static void FLASH_SAVE HeartTickTimerManager(void)
{
    GIZ_PRINT("ht manager %d ms\r\n", Sys_GetRunTime());
    #if(GIZ_PLATFORM == USE_ESP8266)
    CMDFN_MakeHeartTick_Req(NULL);
    os_timer_disarm(&tHeartTickReqTimer);
    os_timer_setfn(&tHeartTickReqTimer, (os_timer_func_t *)HeartTickReqTimer_CallBack, NULL);
    os_timer_arm(&tHeartTickReqTimer, 1000 * 170, 0);   // 180 s timer
    #endif
}
uint8_t FLASH_SAVE CMDFN_SendConfigMode_Req(E_CONFIG_MODE configMode)
{
   uint8_t buf[sizeof(T_ConfigMode_Req) + 5];
   T_ConfigMode_Req *pReqCmd = (T_ConfigMode_Req *)buf;
   uint16_t len = 0;

   GIZ_PRINT("tx CfgModeReq %d ms\n", Sys_GetRunTime());
   os_memset(buf, 0, sizeof(buf));
   GIZ_FillMsgHeader(buf, sizeof(T_ConfigMode_Req) - MSG_PACKET_HEAD_SIZE, CMD_ConfigMode_Req, GIZ_GenSerial());
   pReqCmd->config_mode = configMode;  // data must be set before filling the tailer(xorResult)
   GIZ_FillMsgTailer(buf);
   GIZ_MsgModulate(buf, sizeof(T_ConfigMode_Req));
   len = os_strlen(buf);
   os_strncpy(&buf[len], "\r\n", 2);
   GIZ_UartSend(buf);  
	
	return APP_SUCCESS;
}

// WIFI 模组响应外部设备
static uint8_t FLASH_SAVE CMDFN_MakeConfigMode_Resp(void *pRxMsg)
{
   uint8_t buf[sizeof(T_ConfigMode_Resp) + 5];
   T_ConfigMode_Resp *pRespCmd = (T_ConfigMode_Resp *)buf;
   T_ConfigMode_Req *pReqCmd = (T_ConfigMode_Req *)pRxMsg;
   uint16_t len = 0;

   if(NULL == pRxMsg)
    {
       GIZ_PRINT("err %s %d\n", __FILE__, __LINE__);
	   return APP_FAILED;
    }
   GIZ_PRINT("mk CfgModeResp\n");
   os_memset(buf, 0, sizeof(buf));
   GIZ_FillMsgHeader(buf, sizeof(T_ConfigMode_Resp) - MSG_PACKET_HEAD_SIZE, CMD_ConfigMode_Resp, pReqCmd->header.sn);
   GIZ_FillMsgTailer(buf);
   GIZ_MsgModulate(buf, sizeof(T_ConfigMode_Resp));
   len = os_strlen(buf);
   os_strncpy(&buf[len], "\r\n", 2);
   GIZ_UartSend(buf);   

   return APP_SUCCESS;
}
#if GIZ_CMD_TEST_EN
static E_BOOL isRxCfgModeReq = E_FALSE;
E_BOOL FLASH_SAVE GetRxCfgModeReqFlag(void){ return isRxCfgModeReq; }
void FLASH_SAVE SetRxCfgModeReqFlag(E_BOOL flag){ isRxCfgModeReq = flag; }
#endif

static uint8_t FLASH_SAVE CMDFN_ParseConfigMode_Req(void *pRxMsg)
{
    T_ConfigMode_Req * pCmd = (T_ConfigMode_Req *)pRxMsg;

    if(NULL == pRxMsg)
    {
       GIZ_PRINT("err %s %d\n", __FILE__, __LINE__);
	   return APP_FAILED;
    }
	GIZ_PRINT("parse CfgModeReq %d ms\n", Sys_GetRunTime());
	GIZ_PRINT("cfg_mode = 0x%x\n", (uint32_t)pCmd->config_mode);

	#if GIZ_CMD_TEST_EN
	isRxCfgModeReq = E_TRUE;  
	#endif
	CMDFN_MakeConfigMode_Resp(pRxMsg);
	
    return APP_SUCCESS;
}

static uint8_t FLASH_SAVE CMDFN_ParseConfigMode_Resp(void *pRxMsg)
{
	GIZ_PRINT("rx CfgModeResp\n");
    return APP_SUCCESS;
}
uint8_t FLASH_SAVE CMDFN_SendRestartWifi_Req(void)
{
	uint8_t buf[sizeof(T_RestartWifi_Req) + 5];
	T_RestartWifi_Req *pReqCmd = (T_RestartWifi_Req *)buf;
	uint16_t len = 0;
	GIZ_PRINT("tx RestartWifiReq %d ms\n", Sys_GetRunTime());
	os_memset(buf, 0, sizeof(buf));
	CMDFN_ConstructGeneralMsg(pReqCmd, T_RestartWifi_Req, CMD_RestartWifi_Req); 
	len = os_strlen(buf);
	os_strncpy(&buf[len], "\r\n", 2);
	GIZ_UartSend(buf);	
	return APP_SUCCESS;
}

static uint8_t FLASH_SAVE CMDFN_MakeRestartWifi_Resp(void *pMsg)
{
   uint8_t buf[sizeof(T_RestartWifi_Resp) + 5];
   T_RestartWifi_Req *pReqCmd = (T_RestartWifi_Req *)pMsg;
   uint16_t len = 0;

   GIZ_PRINT("mk RestartWifiResp %d ms\n", Sys_GetRunTime());
   os_memset(buf, 0, sizeof(buf));
   GIZ_FillMsgHeader(buf, sizeof(T_RestartWifi_Resp) - MSG_PACKET_HEAD_SIZE, CMD_RestartWifi_Resp, pReqCmd->header.sn);
   GIZ_FillMsgTailer(buf);
   GIZ_MsgModulate(buf, sizeof(T_RestartWifi_Resp));
   len = os_strlen(buf);
   os_strncpy(&buf[len], "\r\n", 2);
   GIZ_UartSend(buf); 
   return APP_SUCCESS;
}
#if GIZ_CMD_TEST_EN
static E_BOOL isRxRestartWifiResp = E_FALSE;
E_BOOL FLASH_SAVE GetRxRestartWifiReqFlag(void){ return isRxRestartWifiResp; }
void FLASH_SAVE SetRxRestartWifiReqFlag(E_BOOL flag){ isRxRestartWifiResp = flag; }
#endif
static uint8_t FLASH_SAVE CMDFN_ParseRestartWifi_Req(void *pRxMsg)
{
    T_RestartWifi_Req * pCmd = (T_RestartWifi_Req *)pRxMsg;

	GIZ_PRINT("parse RestartWifiReq, %d ms\n", Sys_GetRunTime());
	if(NULL == pRxMsg)
    {
       GIZ_PRINT("err %s %d\n", __FILE__, __LINE__);
	   return APP_FAILED;
    }
    CMDFN_MakeRestartWifi_Resp(pRxMsg);
	
#if GIZ_CMD_TEST_EN
	isRxRestartWifiResp = E_TRUE;  
#endif

	return APP_SUCCESS;
}
static uint8_t FLASH_SAVE CMDFN_ParseRestartWifi_Resp(void *pRxMsg)
{
	GIZ_PRINT("rx RestartWifiResp, %d ms\n", Sys_GetRunTime());

    return APP_SUCCESS;
}
uint8_t FLASH_SAVE  CMDFN_SendWifiStatusChangeNotify_Req(uint16_t wifi_status)
{
   uint8_t buf[sizeof(T_WifiStatusChangeNotify_Req) + 5];
   T_WifiStatusChangeNotify_Req *pReqCmd = (T_WifiStatusChangeNotify_Req *)buf;
   uint16_t len = 0;
   GIZ_PRINT("tx WifiStaReq, sta = 0x%x\n", (uint32_t)wifi_status);
   
   os_memset(buf, 0, sizeof(buf));
   GIZ_FillMsgHeader(buf, sizeof(T_WifiStatusChangeNotify_Req) - MSG_PACKET_HEAD_SIZE,
   	                    CMD_WifiStatusChangeNotify_Req, GIZ_GenSerial());
   pReqCmd->wifi_status = wifi_status;  // WIFI status 
   GIZ_FillMsgTailer(buf);
   GIZ_MsgModulate(buf, sizeof(T_WifiStatusChangeNotify_Req));
   len = os_strlen(buf);
   os_strncpy(&buf[len], "\r\n", 2);
   GIZ_UartSend(buf);   
   
   return APP_SUCCESS;
}


// 外部设备的响应
static  uint8_t FLASH_SAVE CMDFN_ParseWifiStatusChangeNotify_Req(void * pRxMsg)
{
   uint8_t buf[sizeof(T_WifiStatusChangeNotify_Resp) + 5];
   T_WifiStatusChangeNotify_Req *pReqCmd = (T_WifiStatusChangeNotify_Req *)(pRxMsg);
   T_WifiStatusChangeNotify_Resp *pRespCmd = (T_WifiStatusChangeNotify_Resp *)buf;
   uint16_t len = 0;

    if(NULL == pRxMsg)
    {
      GIZ_PRINT("err %s %d\n", __FILE__, __LINE__);
	   return APP_FAILED;
    }
   GIZ_PRINT("parse WifiStaRq 0x%x, %d ms\n", (uint32_t)pReqCmd->wifi_status, Sys_GetRunTime());
   os_memset(buf, 0, sizeof(buf));
   if(sizeof(buf) < (sizeof(T_WifiStatusChangeNotify_Resp) + 2))
   {
      GIZ_PRINT("err %s %d\n", __FILE__, __LINE__);
	  return APP_FAILED;
   }
   GIZ_FillMsgHeader(pRespCmd, sizeof(T_WifiStatusChangeNotify_Resp) - MSG_PACKET_HEAD_SIZE, 
   	                    CMD_WifiStatusChangeNotify_Resp, pReqCmd->header.sn);
   GIZ_FillMsgTailer(pRespCmd);
   GIZ_MsgModulate(pRespCmd, sizeof(T_WifiStatusChangeNotify_Resp));
   len = os_strlen(buf);
   os_strncpy(&buf[len], "\r\n", 2);  // to mark the end of the msg
   GIZ_UartSend(buf);  
   return APP_SUCCESS;
}
static uint8_t FLASH_SAVE CMDFN_ParseWifiStatusChangeNotify_Resp(void *pRxMsg)
{
	GIZ_PRINT("parse WifiStaResp %d ms\n", Sys_GetRunTime());
	return APP_SUCCESS;
}



// WIFI 读取外部设备当前状态的请求
uint8_t FLASH_SAVE CMDFN_SendReadDeviceCurStatus_Req(void)
{
   uint8_t buf[sizeof(T_ReadDeviceCurStatus_Req) + 5];
   T_ReadDeviceCurStatus_Req * pReqCmd = (T_ReadDeviceCurStatus_Req *)buf;
   uint16_t len = 0;

   GIZ_PRINT("tx ReadDevCurStaReq\n");
   os_memset(buf, 0, sizeof(buf));
   GIZ_FillMsgHeader(buf, sizeof(T_ReadDeviceCurStatus_Req) - MSG_PACKET_HEAD_SIZE,
   	                    CMD_ReadDeviceCurStatus_Req, GIZ_GenSerial());
   pReqCmd->action = 0x02; 
   GIZ_FillMsgTailer(buf);
   GIZ_MsgModulate(buf, sizeof(T_ReadDeviceCurStatus_Req));
   len = os_strlen(buf);
   os_strncpy(&buf[len], "\r\n", 2);
   GIZ_UartSend(buf);   
   
   return APP_SUCCESS;
}


static T_DEVICE_STATUS tDeviceStatus, tRxSensorStatus;
static E_BOOL isRxTemperatureData = E_FALSE;
static void FLASH_SAVE Sensor_IsRxTemperatureData(E_BOOL flag){ isRxTemperatureData = flag; }
void FLASH_SAVE DeviceCurTimeUpdate(uint32_t hour, uint32_t min, uint32_t sec)
{
    tDeviceStatus.hour = hour;
	tDeviceStatus.min = min;
	tDeviceStatus.sec = sec;	
}
void FLASH_SAVE DeviceCurSysStatusUpdate(uint8_t sys_status)
{
   tDeviceStatus.sys_status = sys_status;
}
void FLASH_SAVE DeviceCurTempValUpdate(uint32_t tempVal)
{
   tDeviceStatus.temperatue = tempVal;
}
void FLASH_SAVE DeviceCurHumiValUpdate(uint32_t humiVal)
{
   tDeviceStatus.humiduty = humiVal;
}
void FLASH_SAVE DeviceCurFormaValUpdate(uint32_t formaVal)
{
   tDeviceStatus.forma = formaVal;
}
void FLASH_SAVE DeviceCurPM25ValUpdate(uint16_t pm25Val)
{
   tDeviceStatus.pm25 = pm25Val;
}
void FLASH_SAVE DeviceCurBatteryCapacityUpdate(uint32_t batCap)
{
   tDeviceStatus.batCapacity = batCap;
}
static void FLASH_SAVE DeviceCurStatusCopy(T_DEVICE_STATUS *descStatus, T_DEVICE_STATUS * srcStatus)
{
   descStatus->pm25 = srcStatus->pm25;
   descStatus->forma = srcStatus->forma;
   descStatus->temperatue = srcStatus->temperatue;
   descStatus->humiduty = srcStatus->humiduty;
   descStatus->batCapacity = srcStatus->batCapacity;
   descStatus->sys_status = srcStatus->sys_status;
   descStatus->hour = srcStatus->hour;
   descStatus->min = srcStatus->min;
   descStatus->sec = srcStatus->sec;
}
static void FLASH_SAVE DevCurStatusSet(T_DEVICE_STATUS * dev_status)
{
   uint32_t hour = 0, min = 0, sec = 0;
   E_BOOL result;
   result = Sensor_GetHourMinSec(&hour, &min, &sec);
   if(result == E_TRUE)DeviceCurTimeUpdate(hour, min, sec);
   DeviceCurSysStatusUpdate(Sensor_GetSysState());
   DeviceCurPM25ValUpdate(Sensor_GetPM25Val());
   DeviceCurFormaValUpdate(Sensor_GetFormaldehydeVal());
   DeviceCurTempValUpdate(Sensor_GetTemperatureVal());
   DeviceCurHumiValUpdate(Sensor_GetHumidityVal());
   DeviceCurBatteryCapacityUpdate(Sensor_GetBatCapacityVal());
   DeviceCurStatusCopy(dev_status, &tDeviceStatus);
}
static uint8_t FLASH_SAVE CMDFN_MakeReadDeviceCurStatus_Resp(void * pRxMsg)
    {
   uint8_t buf[sizeof(T_ReadDeviceCurStatus_Resp) + 5];
   T_ReadDeviceCurStatus_Req *pReqCmd = (T_ReadDeviceCurStatus_Req *)pRxMsg;
   T_ReadDeviceCurStatus_Resp *pRespCmd = (T_ReadDeviceCurStatus_Resp *)buf;
   uint16_t len = 0;
   GIZ_PRINT("mk DevCurStaResp %d ms\n", Sys_GetRunTime());
   os_memset(buf, 0, sizeof(buf));
   GIZ_FillMsgHeader(buf, sizeof(T_ReadDeviceCurStatus_Resp) - MSG_PACKET_HEAD_SIZE, CMD_ReadDeviceCurStatus_Resp, pReqCmd->header.sn);
   DevCurStatusSet(&pRespCmd->tDevStatus);
   GIZ_FillMsgTailer(buf);
   GIZ_MsgModulate(buf, sizeof(T_ReadDeviceCurStatus_Resp));
   len = os_strlen(buf);
   os_strncpy(&buf[len], "\r\n", 2);
   GIZ_UartSend(buf); 
   return APP_SUCCESS;
    }

static uint8_t FLASH_SAVE CMDFN_ParseReadDeviceCurStatus_Req(void * pRxMsg)
    {
   GIZ_PRINT("parse DevCurStaReq %d ms\n", Sys_GetRunTime());
   return CMDFN_MakeReadDeviceCurStatus_Resp(pRxMsg);
    }

E_BOOL FLASH_SAVE Sensor_HasTemperatureData(uint32_t *temperatureVal)
{
   E_BOOL result = E_FALSE;
   if(isRxTemperatureData == E_TRUE)
   {
      *temperatureVal = tRxSensorStatus.temperatue;
	  result = E_TRUE;
   }
   isRxTemperatureData = E_FALSE;
   return result;
}

static uint8_t FLASH_SAVE CMDFN_ParseReadDeviceCurStatus_Resp(void *pRxMsg)
{
    T_ReadDeviceCurStatus_Resp *pRespCmd = (T_ReadDeviceCurStatus_Resp *)pRxMsg;
	if(NULL == pRxMsg){ GIZ_PRINT("err %s %d\n", __FILE__, (uint32_t)__LINE__); return APP_FAILED; }
	GIZ_PRINT("parse devcurstaresp\n");
    GIZ_PRINT("parse DevCurStaResp %d ms\n", Sys_GetRunTime());
    GIZ_PRINT("hour = %d, min = %d, sec = %d\n", pRespCmd->tDevStatus.hour, pRespCmd->tDevStatus.min, pRespCmd->tDevStatus.sec);
	GIZ_PRINT("sys state = 0x%x\n", (uint32_t)pRespCmd->tDevStatus.sys_status);
	GIZ_PRINT("pm25 = %d, forma = %d, temp = %d'C, humi = %d%, BatCap = %d\n", (uint32_t)pRespCmd->tDevStatus.pm25, 
		        pRespCmd->tDevStatus.forma, pRespCmd->tDevStatus.temperatue, pRespCmd->tDevStatus.humiduty,
		        pRespCmd->tDevStatus.batCapacity);
	Sensor_IsRxTemperatureData(E_TRUE);
	tRxSensorStatus.temperatue = pRespCmd->tDevStatus.temperatue;
	
    return APP_SUCCESS;
}

// 外部设备主动上报
uint8_t FLASH_SAVE CMDFN_SendReadDeviceCurStatusReport(void)
{
    uint8_t buf[sizeof(T_ReadDeviceCurStatus_Report) + 5];
   T_ReadDeviceCurStatus_Report *pReport = (T_ReadDeviceCurStatus_Report *)buf;
   uint16_t len = 0;
	
   GIZ_PRINT("RpDevCurSta tk = %d ms\n", Sys_GetRunTime());
	os_memset(buf, 0, sizeof(buf));
   GIZ_FillMsgHeader(buf, sizeof(T_ReadDeviceCurStatus_Report) - MSG_PACKET_HEAD_SIZE, CMD_ReadDeviceCurStatus_Report, GIZ_GenSerial());
   DevCurStatusSet(&pReport->tDevStatus);
   
	//pReqCmd->dev_status
   GIZ_FillMsgTailer(buf);
   GIZ_MsgModulate(buf, sizeof(T_ReadDeviceCurStatus_Report));
   len = os_strlen(buf);
   os_strncpy(&buf[len], "\r\n", 2);
   GIZ_UartSend(buf); 
   

	return APP_SUCCESS;
}

static uint8_t FLASH_SAVE CMDFN_MakeReadDeviceCurStatusReport_Resp(void *pRxMsg)
{
   uint8_t buf[sizeof(T_ReadDeviceCurStatus_Report_Resp) + 5];
   T_ReadDeviceCurStatus_Report_Resp * pReportResp = (T_ReadDeviceCurStatus_Report_Resp *)buf;
   T_ReadDeviceCurStatus_Report *pReport = (T_ReadDeviceCurStatus_Report *)pRxMsg;
   uint16_t len = 0;

   os_memset(buf, 0, sizeof(buf));
   GIZ_FillMsgHeader(buf, sizeof(T_ReadDeviceCurStatus_Report_Resp) - MSG_PACKET_HEAD_SIZE,
   	                    CMD_ReadDeviceCurStatus_Report_Resp, pReport->header.sn);
   GIZ_FillMsgTailer(buf);
   GIZ_MsgModulate(buf, sizeof(T_ReadDeviceCurStatus_Report_Resp));
   len = os_strlen(buf);
   os_strncpy(&buf[len], "\r\n", 2);
   GIZ_UartSend(buf); 
   return APP_SUCCESS;
}
// wifi 接收到外部设备的状态主动报告
static uint8_t FLASH_SAVE CMDFN_ParseReadDeviceCurStatusReport_Req(void *pRxMsg)
{
    T_ReadDeviceCurStatus_Report *pCmd = (T_ReadDeviceCurStatus_Report *)pRxMsg;


	GIZ_PRINT("parse RpDevCurStaReq %d ms\n", Sys_GetRunTime());
    GIZ_PRINT("hour = %d, min = %d, sec = %d\n", pCmd->tDevStatus.hour, pCmd->tDevStatus.min, pCmd->tDevStatus.sec);
	GIZ_PRINT("sys state = 0x%x\n", (uint32_t)pCmd->tDevStatus.sys_status);
	GIZ_PRINT("pm25 = %d, forma = %d, temp = %d'C, humi = %d%, batcap = %d\n", (uint32_t)pCmd->tDevStatus.pm25, 
		        pCmd->tDevStatus.forma, pCmd->tDevStatus.temperatue, pCmd->tDevStatus.humiduty,
		        pCmd->tDevStatus.batCapacity);
	Sensor_IsRxTemperatureData(E_TRUE);
	tRxSensorStatus.temperatue = pCmd->tDevStatus.temperatue;
	
	CMDFN_MakeReadDeviceCurStatusReport_Resp(pRxMsg);
    return APP_SUCCESS;
}
static uint8_t FLASH_SAVE CMDFN_ParseReadDeviceCurStatus_ReportResp(void * pRxMsg)
{
   GIZ_PRINT("rx DevCurStaRpResp %d ms\n", Sys_GetRunTime());
   return APP_SUCCESS;
}

// WIFI 设置外部设备时间
uint8_t FLASH_SAVE CMDFN_SendDeviceTimeSet_Req(void)
{
   uint8_t buf[sizeof(T_DeviceTimeSet_Req) + 5];
   T_DeviceTimeSet_Req *pReqCmd = (T_DeviceTimeSet_Req *)buf;
   T_TimeHSM tTimeHSM;
   uint16_t len = 0;
	   
   GIZ_PRINT("tx timesetreq\n");
   os_memset(buf, 0, sizeof(buf)); 
   GIZ_FillMsgHeader(buf, sizeof(T_DeviceTimeSet_Req) - MSG_PACKET_HEAD_SIZE, CMD_DeviceTimeSet_Req, GIZ_GenSerial());
   pReqCmd->action = 0x01;
   pReqCmd->attr_flags = 0x07;
   //pReqCmd->attr_vals[0];  设置时间
   Sensor_GetHourMinSec(&tTimeHSM.hour, &tTimeHSM.min, &tTimeHSM.sec);
   pReqCmd->timeSet = tTimeHSM;
   GIZ_FillMsgTailer(buf);
   GIZ_MsgModulate(buf, sizeof(T_DeviceTimeSet_Req));
   len = os_strlen(buf);
   os_strncpy(&buf[len], "\r\n", 2);
   GIZ_UartSend(buf); 
   
   return APP_SUCCESS;
}
static uint8_t FLASH_SAVE CMDFN_MakeDeviceTimeSet_Resp(void * pRxMsg)
{
   uint8_t buf[sizeof(T_DeviceTimeSet_Resp) + 5];
   T_DeviceTimeSet_Req *pReqCmd = (T_DeviceTimeSet_Req *)pRxMsg;
   uint16_t len = 0;
   os_memset(buf, 0, sizeof(buf));
   GIZ_FillMsgHeader(buf, sizeof(T_DeviceTimeSet_Resp) - MSG_PACKET_HEAD_SIZE, CMD_HeartTick_Resp, pReqCmd->header.sn);
   GIZ_FillMsgTailer(buf);
   GIZ_MsgModulate(buf, sizeof(T_DeviceTimeSet_Resp));
   len = os_strlen(buf);
   os_strncpy(&buf[len], "\r\n", 2);  // to remark the end of the msg
   GIZ_UartSend(buf);  
   return APP_SUCCESS;
}
#if GIZ_CMD_TEST_EN
static E_BOOL isRxTimeSetReq = E_FALSE;
E_BOOL FLASH_SAVE GetRxTimeSetReqFlag(void){ return isRxTimeSetReq; }
void FLASH_SAVE SetRxTimeSetReqFlag(E_BOOL flag){ isRxTimeSetReq = flag; }
#endif
static uint8_t FLASH_SAVE CMDFN_ParseDeviceTimeSet_Req(void *pRxMsg)
{ 	
   T_DeviceTimeSet_Req *pReqCmd = (T_DeviceTimeSet_Req *)pRxMsg;
   if(NULL == pRxMsg){ GIZ_PRINT("err %s %d\n", __FILE__, __LINE__);  return APP_FAILED; }
   GIZ_PRINT("parse TimeSetReq %d ms\n", Sys_GetRunTime());
   GIZ_PRINT("hour %d, min = %d, sec = %d\n", pReqCmd->timeSet.hour, pReqCmd->timeSet.min, pReqCmd->timeSet.sec);
   #if GIZ_CMD_TEST_EN
   isRxTimeSetReq = E_TRUE;
   #endif
   return CMDFN_MakeDeviceTimeSet_Resp(pRxMsg);
}
static uint8_t FLASH_SAVE CMDFN_ParseDeviceTimeSet_Resp(void *pRxMsg)
{

   GIZ_PRINT("parse TimeSetResp %d ms\n", Sys_GetRunTime());

   return APP_SUCCESS;
}


// 命令映射表
static T_CMD_MAP tCmdTable[] = 
{
    {CMD_DeviceInfo_Req,              	    CMDFN_ParseDeviceInfo_Req,                   CMDFN_MakeNothing, },
    {CMD_DeviceInfo_Resp,             	CMDFN_ParseDeviceInfo_Resp,              	CMDFN_MakeNothing, },
    {CMD_ReadDeviceCurStatus_Req,    	    CMDFN_ParseReadDeviceCurStatus_Req,     	    CMDFN_MakeNothing,},
    {CMD_ReadDeviceCurStatus_Resp,      CMDFN_ParseReadDeviceCurStatus_Resp,       CMDFN_MakeNothing, },
    {CMD_ReadDeviceCurStatus_Report,        CMDFN_ParseReadDeviceCurStatusReport_Req,  CMDFN_MakeNothing, },
    {CMD_ReadDeviceCurStatus_Report_Resp,  CMDFN_ParseReadDeviceCurStatus_ReportResp, CMDFN_MakeNothing, },
    {CMD_HeartTick_Req, 					    CMDFN_ParseHeartTick_Req, 					 CMDFN_MakeNothing, },
    {CMD_HeartTick_Resp,     				CMDFN_ParseHeartTick_Resp, 					CMDFN_MakeNothing, },
    {CMD_ConfigMode_Req, 					CMDFN_ParseConfigMode_Req,                   CMDFN_MakeNothing, },
    {CMD_ConfigMode_Resp,                    CMDFN_ParseConfigMode_Resp,                   CMDFN_MakeNothing, },
    {CMD_RestartWifi_Req,                CMDFN_ParseRestartWifi_Req, 					CMDFN_MakeNothing, },
    {CMD_RestartWifi_Resp,    			    CMDFN_ParseRestartWifi_Resp, 				 CMDFN_MakeNothing, },
    {CMD_WifiStatusChangeNotify_Req,        CMDFN_ParseWifiStatusChangeNotify_Req, 	 CMDFN_MakeNothing, },
    {CMD_WifiStatusChangeNotify_Resp,   CMDFN_ParseWifiStatusChangeNotify_Resp, 	CMDFN_MakeNothing, },
    {CMD_DeviceTimeSet_Req,     			    CMDFN_ParseDeviceTimeSet_Req,  				 CMDFN_MakeNothing, },
    {CMD_DeviceTimeSet_Resp,    			 CMDFN_ParseDeviceTimeSet_Resp,   			CMDFN_MakeNothing, },
    
   	{ CMD_Reserved,           				 CMDFN_ParseNothing,              	        CMDFN_MakeNothing, },
};

#define CMD_TABLE_SIZE (sizeof(tCmdTable) / sizeof(tCmdTable[0]))

// 接收串口数据

#define GIZ_RX_BUF_SIZE 128
void FLASH_SAVE ATUART_RxIntServer(uint8_t *data, int32_t len)
{
   uint8_t buff[GIZ_RX_BUF_SIZE];
   uint8_t cmdCode = 0, cmdIndex = 0;

   if(len > sizeof(buff)){ GIZ_PRINT("len too long\n"); return; };
   
   os_memset(buff, 0, sizeof(buff));
   os_strncpy(buff, data, len);
   SerialDrv_SetRxCompleteBool(E_FALSE);
   if(GIZ_IsCmdCorrect(buff, len, &cmdCode) == APP_SUCCESS)
   {
      GIZ_PRINT("search cmd success\r\n");
      for(cmdIndex = 0; cmdIndex < CMD_TABLE_SIZE; cmdIndex++)
      {
         if(cmdCode == tCmdTable[cmdIndex].cmd)break;
      }

	  if(cmdIndex < CMD_TABLE_SIZE)
	  {
	     GIZ_PRINT("parse cmd 0x%x\r\n", cmdCode);
	     tCmdTable[cmdIndex].ParseCmdFunc(buff);  // parse process
	  }
   }
   else{  GIZ_PRINT("search cmd failed\r\n");  }
}

static os_timer_t tTestTimer;
void FLASH_SAVE TestTimerCallBack(void *param)
{
   SERIAL_DEBUG("testT cb %d ms\r\n", Sys_GetRunTime());
}
void FLASH_SAVE ATCMD_Init(void)
{
    SerialDrv_Init((ATCMD_RxProcessFunc)ATUART_RxIntServer);
  

   ATCMD_DeviceInfoReqTimerManager();  // request device msg
   HeartTickTimerManager();
}

