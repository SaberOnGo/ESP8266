
#ifndef __WDJSON_DRV_H__
#define  __WDJSON_DRV_H__

#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "user_config.h"
#include "espconn.h"
#include "os_type.h"
#include "mem.h"
    
#include "wilddog.h"
#include "GlobalDef.h"


#define KEY_NAME_TEMP_SENSOR_0     "TempSensor_0"
#define KEY_NAME_HUMI_SENSOR_0     "HumiSensor_0"

typedef enum
{
   E_Obj_Unknown = 0,
   E_Obj_WifiModule = 1,
   E_Obj_App = 2,
   E_Obj_Server = 3,
}E_CMD_OBJ;

// 命令方向
typedef enum
{
   E_DIR_REQ = 0,  // 请求命令
   E_DIR_RESP = 1, // 响应命令
}E_DIR;

// 命令类型
typedef enum
{
   E_CMDTYPE_NETCONNECT = 0,   // 网络连接命令
   E_CMDTYPE_REPORT = 1,      // 上报命令
   E_CMDTYPE_SET = 2,         // 设置命令
   E_CMDTYPE_QUERY = 3,       // 查询命令
   E_CMDTYPE_HEARTTICK = 4,   // 心跳命令
   
   E_CMDTYPE_END,
}E_CMDTYPE;

typedef enum
{
   E_CMDCODE_Reserved = 0,
   E_CMDCODE_NetConnect = 1,  // 网络连接
   E_CMDCODE_ReportDeviceVersion = 2,  // 上报版本信息
   
}E_CMD_CODE; // 命令值

typedef struct
{
   Wilddog_T refId;        // 节点的引用 id
   Wilddog_Node_T *node;  // WildDog 数据节点
   Wilddog_Node_T *child;  // 子节点
   os_timer_t timer;        // 同步数据用的定时器
   E_BOOL isFinish;            //上一步操作是否完成
   E_BOOL isSetFinish;        // 推送完成
   E_BOOL isObserverFinish;   // 本次观察成功
   E_BOOL isAddObserver;      // 是否注册Observer
   E_BOOL isDeleteNode;       // 是否删除本节点
   E_BOOL isFreeSelfMem;      // 是否释放本结构体所在的内存
   uint32_t timerMs;           // 定时 ms值
   uint32_t msgId;             // 本次推送的消息id
}T_WDJSON_NODE_SYNC;  // WildDog 数据同步


void FLASH_SAVE WDJson_WriteToCloud(void);


#endif

