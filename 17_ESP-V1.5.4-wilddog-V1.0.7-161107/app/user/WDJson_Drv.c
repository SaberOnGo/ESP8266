
#include "WDJson_Drv.h"
#include "Util_Drv.h"

#include "SensorInterface.h"

#define DEVICE_INFO_URL  "coaps://esp8266-wae1024.wilddogio.com/"

static E_BOOL isNextDataToPush = E_TRUE;  // 进行下一个数据的推送

void FLASH_SAVE WDJson_PushToCloud(void *arg);

// 创建节点树
static Wilddog_Node_T * FLASH_SAVE WDJson_CreateNodeTree(void)
{
	//创建根节点，该节点为一个对象，key 为 mac地址 
	Wilddog_Node_T *packetHead = wilddog_node_createObject("66:77:88:99:AA:BB");

	//创建子节点 "DeviceInfo"，同样为对象
    Wilddog_Node_T *DeviceInfo = wilddog_node_createObject("DeviceInfo");
	
	//创建叶节点 "mac"，是一个 UTF-8 字符串
    Wilddog_Node_T *mac = wilddog_node_createUString("mac", "66:77:88:99:AA:BB");
	Wilddog_Node_T *PcbVer = wilddog_node_createUString("PcbVer", "PCB Ver V1.50A");
    Wilddog_Node_T *SoftVer = wilddog_node_createUString("SoftVer", "ESP-Scratch V1.01R1P01");
	Wilddog_Node_T *UartVer = wilddog_node_createUString("UartVer", "Uart Ver V1.05A");
	Wilddog_Node_T *JsonVer = wilddog_node_createUString("JsonVer", "JSON Ver V0.03A");
	Wilddog_Node_T *Serial = wilddog_node_createUString("Serial", "341256789esdef");
	Wilddog_Node_T *DeviceAttr = wilddog_node_createUString("DeviceAttr", "ESP8266 Scratch Device");

	//将叶节点插入到 "DeviceInfo" 节点中
	wilddog_node_addChild(DeviceInfo, mac);
	wilddog_node_addChild(DeviceInfo, PcbVer);
	wilddog_node_addChild(DeviceInfo, SoftVer);
	wilddog_node_addChild(DeviceInfo, UartVer);
	wilddog_node_addChild(DeviceInfo, JsonVer);
	wilddog_node_addChild(DeviceInfo, Serial);
	wilddog_node_addChild(DeviceInfo, DeviceAttr);

    Wilddog_Node_T *TempSensor0 = wilddog_node_createFloat(KEY_NAME_TEMP_SENSOR_0, -12.1);
    Wilddog_Node_T *HumiSensor0 = wilddog_node_createNum(KEY_NAME_HUMI_SENSOR_0, -10);
	
	//将 子节点插入根节点中
    wilddog_node_addChild(packetHead, DeviceInfo);
    wilddog_node_addChild(packetHead, TempSensor0);
    wilddog_node_addChild(packetHead, HumiSensor0);
	
	return packetHead;
}

//  数据变化所触发的回调函数
//  p_snapshot是取回的数据镜像（err为200时）或者NULL，退出函数后即被销毁, arg为用户传递的值, err为状态码
STATIC void FLASH_SAVE 
WDJson_onObserverCallback(const Wilddog_Node_T* p_snapshot, void* arg, Wilddog_Return_T err)
{
    T_WDJSON_NODE_SYNC *nodeSync = (T_WDJSON_NODE_SYNC *)arg;
	
    if(err != WILDDOG_HTTP_OK){
        wilddog_debug("observe failed!\n");
        return;
    }
    
	if(NULL != nodeSync)
	{
	   // 回调函数中设为TRUE, 因此可以在WDJson_ObserverSync() 函数中得知是否成功
       nodeSync->isObserverFinish = E_TRUE;  
    }
	wilddog_debug("observe data!\n");
	
    return;
}

//服务端回应或者回应超时触发的回调函数
//arg为用户传递的值,
//err为状态码
STATIC void FLASH_SAVE WDJson_OnSetCallBack(void* arg, Wilddog_Return_T err)
{
    T_WDJSON_NODE_SYNC *nodeSync = (T_WDJSON_NODE_SYNC *)arg;
    uint32_t msgId = 65535;

	
	if(nodeSync != NULL)
	{
	   msgId = nodeSync->msgId;
	}
    if(err < WILDDOG_HTTP_OK || err >= WILDDOG_HTTP_NOT_MODIFIED)
    {
        wilddog_debug("setValue error!, sys time: %ld ms, msgId = %ld\n", system_get_time() / 1000, msgId);
        return;
    }
    wilddog_debug("setValue success! sys time: %ld ms, msgId = %ld\n", system_get_time() / 1000, msgId);
    if(NULL != nodeSync)nodeSync->isSetFinish = E_TRUE;   // 服务器回应成功, 此此处置 为true
    return;
}

static void FLASH_SAVE WDJson_OnSetSync(void *arg);

static void FLASH_SAVE WDJson_OnObserverSync(void *arg)
{
    T_WDJSON_NODE_SYNC *nodeSync = (T_WDJSON_NODE_SYNC *)arg;
	if(NULL == arg)
	{
		wilddog_debug("error: WDJson_OnObserverSync() arg is null\n");
        return;
	}

    if(nodeSync->isObserverFinish == E_TRUE)
    {
        wilddog_debug("observer sync success\n");
        nodeSync->isObserverFinish = E_FALSE;  //重新设置接收状态为FALSE
    }
	else
	{
	   wilddog_debug("observer sync failed\n");
	}
}

// 数据推送到云端回调, 即如果数据没有推送到云端, 此函数会被重新调用
static void FLASH_SAVE WDJson_OnSetSync(void *arg)
{
    T_WDJSON_NODE_SYNC *nodeSync = (T_WDJSON_NODE_SYNC *)arg;
    static uint8_t count = 0;
	
    os_printf("OnSetSync %ld ms\n", system_get_time() / 1000);
	count++;
	if(count > 5)
	{
	   count = 0;
	   system_show_malloc();
	   os_printf("left head = %d\n", system_get_free_heap_size());
	}
    if(NULL == arg)
    {
       wilddog_debug("arg is null, return\n");
	   return;
    }
    if(nodeSync->isSetFinish == E_TRUE)  // 服务器回应成功, 此值在 WDJson_OnSetCallBack() 里被改变
    {   
        os_timer_disarm(&nodeSync->timer);
        nodeSync->isSetFinish = E_FALSE;

        if(isNextDataToPush == E_FALSE)
        {
           isNextDataToPush = E_TRUE;  // 可以推送下一个数据
        }
        //监听某节点的数据变化。一旦该数据发生改变, WDJson_onObserverCallback() 函数将被调用
        if(nodeSync->isAddObserver == E_TRUE)  // 添加观察者
        {
	        wilddog_addObserver(nodeSync->refId, WD_ET_VALUECHANGE, WDJson_onObserverCallback, arg);
	        os_timer_setfn(&nodeSync->timer, (os_timer_func_t *)WDJson_OnObserverSync, arg);
	        os_timer_arm(&nodeSync->timer, 1000, 0);    
        }
		if(E_TRUE == nodeSync->isDeleteNode)
		{
		   //数据已经推送，删除刚才建立的节点
           wilddog_node_delete(nodeSync->node);
		}

		if(E_TRUE == nodeSync->isFreeSelfMem)  // 释放内存
		{
		   wilddog_debug("free mem addr = 0x%d, msgId = %ld\n", (long)nodeSync, nodeSync->msgId);
		   os_free(nodeSync);
		   
		}
    }
    else
    {   
        wilddog_trySync();   // 尝试重新推送数据到云端
        wilddog_increaseTime(1100);
        os_timer_setfn(&nodeSync->timer, (os_timer_func_t *)WDJson_OnSetSync, arg);
        os_timer_arm(&nodeSync->timer, 1000, 0);        
    }
}

/*****************************************************************************
 * @\fn  : WDJson_SetNodeFloatValue
 * @\author : pi
 * @\date : 2016 - 9 - 20
 * @\brief : 设置WIFI 模组下的节点的值
 * @\param[in] :Wilddog_T fatherRefId:  父节点的引用 id
 *                        Wilddog_Node_T *fatherNode: 父节点的指针
 *                        uint8_t *nodeKey: 当前要设置的节点的KEY值
 *                        float value: 要设置的值
 *                       int validDecimalPointSize: 小数点后的有效位数
 * @\param[out] : none
 * @\return : void
 * @\attention : 
 * @\note [others] : 

*****************************************************************************/
E_RESULT FLASH_SAVE 
WDJson_SetNodeFloatValue(Wilddog_T fatherRefId, 
								   Wilddog_Node_T *fatherNode, 
								   uint8_t *nodeKey,
									float value, 
									int validDecimalPointSize)
{
   T_WDJSON_NODE_SYNC * pTempSync = NULL;
   static T_WDJSON_NODE_SYNC tempSync;
   Wilddog_T childId;
   Wilddog_Node_T *childNode = NULL;

   
   childId = wilddog_getChild(fatherRefId, nodeKey);  // 找到子节点的refId
   if(0 == childId)
   {
      wilddog_debug("get child id failed\n");
	  return APP_FAILED;
   }

   childNode = wilddog_node_find(fatherNode, nodeKey);
   if(NULL == childNode)
   {
      wilddog_debug("get child node failed\n");
	  return APP_FAILED;
   }
   tempSync.refId = childId;
   tempSync.node = childNode;
   tempSync.timerMs = 3500;
   tempSync.msgId++;
   pTempSync = &tempSync;
  
   wilddog_node_setValue(childNode, (u8 *)&value, sizeof(wFloat));
   wilddog_setValue(childId, childNode, WDJson_OnSetCallBack, (void *)pTempSync);  //推送到云端

   #if 1
   os_timer_disarm(&pTempSync->timer);
   os_timer_setfn(&pTempSync->timer, (os_timer_func_t *)WDJson_OnSetSync, (void*)pTempSync);
   os_timer_arm(&pTempSync->timer, pTempSync->timerMs, 0); 
   #endif
}

E_RESULT FLASH_SAVE 
WDJson_SetNodeNumValue(Wilddog_T fatherRefId, 
								   Wilddog_Node_T *fatherNode, 
								   uint8_t *nodeKey,
								   s32 value)
{
   T_WDJSON_NODE_SYNC * pTempSync = NULL;
   static T_WDJSON_NODE_SYNC tempSync;
   Wilddog_T childId;
   Wilddog_Node_T *childNode = NULL;

   os_printf("SetNum %ld ms\n", system_get_time() / 1000);
   
   childId = wilddog_getChild(fatherRefId, nodeKey);  // 找到子节点的refId
   if(0 == childId)
   {
      wilddog_debug("get child id failed\n");
	  return APP_FAILED;
   }

   childNode = wilddog_node_find(fatherNode, nodeKey);
   if(NULL == childNode)
   {
      wilddog_debug("get child node failed\n");
	  return APP_FAILED;
   }

   tempSync.refId = childId;
   tempSync.node = childNode;
   tempSync.timerMs = 1000;
   tempSync.msgId++;

   pTempSync = &tempSync;
   #if 0
   pTempSync = (T_WDJSON_NODE_SYNC *)os_malloc(sizeof(T_WDJSON_NODE_SYNC));
   if(NULL == pTempSync)
   {
      wilddog_debug("malloc failed\n");
	  return APP_FAILED;
   }
   pTempSync->isFreeSelfMem = E_TRUE;
   #endif
   
   wilddog_node_setValue(childNode, (u8 *)&value, sizeof(s32));
   wilddog_setValue(childId, childNode, WDJson_OnSetCallBack, (void *)pTempSync);  //推送到云端

   #if 1
   os_timer_disarm(&pTempSync->timer);
   os_timer_setfn(&pTempSync->timer, (os_timer_func_t *)WDJson_OnSetSync, (void*)pTempSync);
   os_timer_arm(&pTempSync->timer, pTempSync->timerMs, 0); 
   #endif
}

STATIC void FLASH_SAVE TemperatueNode_onPushCallback(u8 *p_path,void* arg, Wilddog_Return_T err){
    if(err < WILDDOG_HTTP_OK || err >= WILDDOG_HTTP_NOT_MODIFIED){
        wilddog_debug("set failed, error code is %ld", err);
        return;
    }
    wilddog_debug("new path is %s", p_path);
    return;
}

static void FLASH_SAVE TemperatueNode_OnSetCallBack(void* arg, Wilddog_Return_T err)
{
    if(err < WILDDOG_HTTP_OK || err >= WILDDOG_HTTP_NOT_MODIFIED)
    {
        wilddog_debug("setValue error %ld ms\n", Sys_GetRunTime());
        return;
    }
    wilddog_debug("setValue OK %ld ms\n", Sys_GetRunTime());
    return;
}

E_RESULT FLASH_SAVE 
WDJson_SetNodeStringValue(Wilddog_T fatherRefId, 
								   Wilddog_Node_T *fatherNode, 
								   uint8_t *nodeKey,
								   uint8_t *stringValue)
{
   Wilddog_T childId;
   Wilddog_Node_T *tempChildNode = NULL;
   
   os_printf("SetNumToString %ld ms\n", system_get_time() / 1000);

   childId = wilddog_getChild(fatherRefId, nodeKey);  // 找到子节点的refId
   if(0 == childId)
   {
      wilddog_debug("error: file = %s, line = %d\n", __FILE__, __LINE__);
	  return APP_FAILED;
   }
   // 建立一个字符串子节点
   tempChildNode = wilddog_node_createUString((Wilddog_Str_T * )nodeKey, (Wilddog_Str_T *)stringValue); 
   wilddog_setValue(childId, tempChildNode, TemperatueNode_OnSetCallBack, NULL);
   wilddog_trySync();
}

static T_WDJSON_NODE_SYNC tFatherNodeSync;
static os_timer_t tTemperaturePushToCloudTimer;
static void FLASH_SAVE TemperaturePushToCloud_CallBack(void *arg)
{
    uint8_t temperatureString[12];
	uint32_t temperatureVal;
	E_BOOL result;
	
	os_printf("temp push to cloud cb %d ms \n", Sys_GetRunTime());
    os_timer_disarm(&tTemperaturePushToCloudTimer);
	#if 1
	if(Sensor_HasTemperatureData(&temperatureVal) == E_TRUE)
	{
	   WDJSON_DEBUG("has temp data, %ld ms %ld.%ld 'C\n", Sys_GetRunTime(), temperatureVal / 1000, temperatureVal % 1000);
	   os_memset(temperatureString, 0, sizeof(temperatureString));
	   os_sprintf((char *)temperatureString, "%d.%d 'C", temperatureVal / 1000, temperatureVal % 1000);
	   WDJSON_DEBUG("temp: %s\n", temperatureString);
	   WDJson_SetNodeStringValue(tFatherNodeSync.refId, tFatherNodeSync.node, "TempSensor_1", temperatureString);
	}
	#endif
	os_timer_setfn(&tTemperaturePushToCloudTimer,  TemperaturePushToCloud_CallBack, NULL);
	os_timer_arm(&tTemperaturePushToCloudTimer, 1000, 0);
}
void  FLASH_SAVE
	WDJson_WriteToCloud(void)
{
    tFatherNodeSync.node = WDJson_CreateNodeTree();
	tFatherNodeSync.refId = wilddog_initWithUrl(DEVICE_INFO_URL);  
	tFatherNodeSync.timerMs = 1000;
	
	// 设置当前节点的数据
	#if 1
    wilddog_setValue(tFatherNodeSync.refId, tFatherNodeSync.node, WDJson_OnSetCallBack, (void*)&tFatherNodeSync);

    #if 0
    WDJson_PushToCloud((void *)&tFatherNodeSync);   // 定时发送数据进行测试
	
    os_timer_disarm(&tFatherNodeSync.timer);
    os_timer_setfn(&tFatherNodeSync.timer, (os_timer_func_t *)WDJson_OnSetSync, (void*)&tFatherNodeSync);
    os_timer_arm(&tFatherNodeSync.timer, tFatherNodeSync.timerMs, 0);  
    #endif
	
    os_timer_disarm(&tTemperaturePushToCloudTimer);
	os_timer_setfn(&tTemperaturePushToCloudTimer,  TemperaturePushToCloud_CallBack, NULL);
	os_timer_arm(&tTemperaturePushToCloudTimer, 1000, 0);
	#endif
}

static os_timer_t tPushTimer;
static void FLASH_SAVE WDJson_PushSync(void *arg)
{
    T_WDJSON_NODE_SYNC * nodeSync = (T_WDJSON_NODE_SYNC *)arg;
    static float temp = 240;

	

    os_timer_disarm(&tPushTimer);

	if(E_TRUE == isNextDataToPush)
	{
	    isNextDataToPush = E_FALSE;
	    temp += 1.5;
		
		#if 0
		WDJson_SetNodeFloatValue(nodeSync->refId,
									nodeSync->node,
									KEY_NAME_TEMP_SENSOR_0,
									temp,
									1);
		#endif

		WDJson_SetNodeNumValue(nodeSync->refId,
									nodeSync->node,
									KEY_NAME_TEMP_SENSOR_0,
									(u32)280);
	}
	os_printf("PushSync %ld ms\n", system_get_time() / 1000);
	
	os_timer_setfn(&tPushTimer, (os_timer_func_t *)WDJson_PushSync, arg);
    os_timer_arm(&tPushTimer, 1000, 0);
}

// 测试推送变化的数据到云端
void FLASH_SAVE WDJson_PushToCloud(void *arg)
{
	os_timer_disarm(&tPushTimer);
    os_timer_setfn(&tPushTimer, (os_timer_func_t *)WDJson_PushSync, arg);
    os_timer_arm(&tPushTimer, 0, 0);
}

