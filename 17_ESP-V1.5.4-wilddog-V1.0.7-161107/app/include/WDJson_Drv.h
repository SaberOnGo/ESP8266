
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

// �����
typedef enum
{
   E_DIR_REQ = 0,  // ��������
   E_DIR_RESP = 1, // ��Ӧ����
}E_DIR;

// ��������
typedef enum
{
   E_CMDTYPE_NETCONNECT = 0,   // ������������
   E_CMDTYPE_REPORT = 1,      // �ϱ�����
   E_CMDTYPE_SET = 2,         // ��������
   E_CMDTYPE_QUERY = 3,       // ��ѯ����
   E_CMDTYPE_HEARTTICK = 4,   // ��������
   
   E_CMDTYPE_END,
}E_CMDTYPE;

typedef enum
{
   E_CMDCODE_Reserved = 0,
   E_CMDCODE_NetConnect = 1,  // ��������
   E_CMDCODE_ReportDeviceVersion = 2,  // �ϱ��汾��Ϣ
   
}E_CMD_CODE; // ����ֵ

typedef struct
{
   Wilddog_T refId;        // �ڵ������ id
   Wilddog_Node_T *node;  // WildDog ���ݽڵ�
   Wilddog_Node_T *child;  // �ӽڵ�
   os_timer_t timer;        // ͬ�������õĶ�ʱ��
   E_BOOL isFinish;            //��һ�������Ƿ����
   E_BOOL isSetFinish;        // �������
   E_BOOL isObserverFinish;   // ���ι۲�ɹ�
   E_BOOL isAddObserver;      // �Ƿ�ע��Observer
   E_BOOL isDeleteNode;       // �Ƿ�ɾ�����ڵ�
   E_BOOL isFreeSelfMem;      // �Ƿ��ͷű��ṹ�����ڵ��ڴ�
   uint32_t timerMs;           // ��ʱ msֵ
   uint32_t msgId;             // �������͵���Ϣid
}T_WDJSON_NODE_SYNC;  // WildDog ����ͬ��


void FLASH_SAVE WDJson_WriteToCloud(void);


#endif

