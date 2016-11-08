
#ifndef __ATCMD_DRV_H__
#define  __ATCMD_DRV_H__

#include "giz_includes.h"



/************************************ MACRO SWITCH OPTION BEGIN *****************************************/

#if   GIZ_DEBUG_EN
#define GIZ_PRINT   os_printf
#else
#define GIZ_PRINT(...)
#endif

#if 1 
#define GIZ_DEBUG os_printf
#else
#define GIZ_DEBUG(...) 
#endif


#define GIZ_UartSend(str)   at_port_print((const char *)str)    

/************************************ MACRO SWITCH OPTION END *****************************************/


/****************************************************************************************************
�����ƽ��봮��Э��
�����ʽ��������:
header(2B) = 0xFFFF, len(2B),  cmd(1B), sn(1B), params(x B), checksum(1B)
1:  header �̶�Ϊ0xFFFF
2:  ����len ��ָ�� cmd��ʼ���������ݰ�������ռ�õ��ֽ���
3:  ����һ���ֽڵ����������Դ���ֽ������
4   ��Ϣ���(sn)�ɷ��ͷ����������շ���Ӧ����ʱ�����Ϣ��ŷ��ظ����ͷ�
5:  �����(checksum)�ļ��㷽ʽΪ�����ݰ����ֽ���͵ó��Ľ��ȡ������ֽڵ�����
6:  ���з��͵��������ȷ�ϣ�����200������û���յ����շ�����Ӧ�����ͷ�Ӧ�ط�������ط�3��

����Լ��:  ���ĵĵ�������:
7   δ����ǰ�ı��ĳ�Ϊԭ����, ���ƺ�ı��ĳ�Ϊ�±���, У�����ֵ�ǶԾɱ�������Ľ��
8   ����: �����г���һ���ֽڵ�ֵΪ0xCC, ����0xDD (����У����), ���Ϊ0x00, ��涨���Ϊ0xCC
9:  ���: �㷨һ:  �����г���0xCC, ��ָ�Ϊ0x00, �������0xDD, ��ָ�Ϊ0xCC, ��ʱ���У����
                  �����һ���㷨��, У����Ա���У�����, �������㷨2:
                  �����г���0xCC, �ָ�Ϊ0x00, �������0xDD, �򲻴���, ��ʱ���У����
*****************************************************************************************************/

#define GIZ_VERSION_STRING_LEN    8     // �豸����ذ汾�ַ�������
#define GIZ_MAX_STRING_LEN        32
#define GIZ_PRODUCT_KEY_STR_LEN   GIZ_MAX_STRING_LEN
#define MSG_PACKET_HEAD_SIZE      4   // header(2B) + len(2B)
#define GIZ_CHECKSUM_SIZE         1   // У���ֶ���ռ����

#define GIZ_HEADER_VAL     0xFF    // ��Ϣͷ�Ĺ̶�ֵ
#define GIZ_CODE_1_VAL     0xCC    // ����ֵ1
#define GIZ_CODE_2_VAL     0xDD    
#define GIZ_CODE_3_VAL     0xEE


typedef enum
{
   CMD_DeviceInfo_Req  = 0x01,
   CMD_DeviceInfo_Resp = 0x02,
   CMD_ReadDeviceCurStatus_Req = 0x03,
   CMD_ReadDeviceCurStatus_Resp = 0x04, 
   CMD_ReadDeviceCurStatus_Report = 0x05,
   CMD_ReadDeviceCurStatus_Report_Resp = 0x06,
   CMD_HeartTick_Req   = 0x07,
   CMD_HeartTick_Resp  = 0x08,
   CMD_ConfigMode_Req  = 0x09,
   CMD_ConfigMode_Resp = 0x0A,
   CMD_RestartWifi_Req = 0x0B,
   CMD_RestartWifi_Resp = 0x0C,
   CMD_WifiStatusChangeNotify_Req = 0x0D,
   CMD_WifiStatusChangeNotify_Resp = 0x0E,
   CMD_DeviceTimeSet_Req = 0x13,
   CMD_DeviceTimeSet_Resp = 0x14,
   
   CMD_Reserved = 0xFF,
}E_CMD_VAL;

#pragma pack (1)
typedef struct
{
   uint16_t head;   
   uint16_t len;
   uint8_t  cmd;
   uint8_t  sn;       // ��Ϣ���
   uint16_t flags;   // 2B
}T_MSG_HEADER;  // ����ͷ

typedef struct
{
   uint8_t  checksum;  // 1B
}T_MSG_TAILER;   // ����β

typedef struct
{
   T_MSG_HEADER header;
   T_MSG_TAILER tailer;
}T_General_Cmd;

typedef T_General_Cmd T_DeviceInfo_Req;   // WIFI �����豸��Ϣ����
typedef struct
{
   T_MSG_HEADER header;

   uint8_t  protocol_ver[GIZ_VERSION_STRING_LEN];  // ����Э��汾��
   uint8_t  private_ver[GIZ_VERSION_STRING_LEN];   // �Զ���˽�а汾��
   uint8_t  hard_ver[GIZ_VERSION_STRING_LEN];      
   uint8_t  soft_ver[GIZ_VERSION_STRING_LEN];      
   uint8_t  product_key[GIZ_PRODUCT_KEY_STR_LEN];

   /* 1. �󶨳�ʱ(binable_timeout)��ֵΪ0ʱ����ʾ�豸���ھ��������󶨣�
      ��ֵ������ʱ����ʾ�����°󶨰�ť���û������ڸ�ʱ�䷶Χ����ɰ󶨲��� */
   uint16_t binable_timeout;   // �󶨳�ʱ 

   T_MSG_TAILER tailer;
}T_DeviceInfo_Resp; // �豸��Ϣ��Ӧ

typedef T_General_Cmd T_HeartTick_Req, T_HeartTick_Resp;   // ��������, ��Ӧ

typedef enum
{
   E_WIFI_SoftAp = 1,
   E_WIFI_AirLink = 2,
   E_WIFI_WPS = 3,
   E_WIFI_NFC = 4,
   E_WIFI_AppleWAC = 5,
   
}E_CONFIG_MODE;
typedef struct
{
   T_MSG_HEADER header;
   uint8_t config_mode;  
   T_MSG_TAILER tailer;
}T_ConfigMode_Req;  // ����ģʽ����
typedef T_General_Cmd T_ConfigMode_Resp;  // ����ģʽ��Ӧ

typedef T_General_Cmd T_RestartWifi_Req, T_RestartWifi_Resp;   // ����WIFI ����, ��Ӧ

#define WIFI_BIT_IsSoftApOn   0x0001
#define WIFI_BIT_IsStationOn  0x0002
#define WIFI_BIT_IsCfgModeOn  0x0004
#define WIFI_BIT_IsBindingOn  0x0008
#define WIFI_BIT_IsConnectAp  0x0010
#define WIFI_BIT_IsConnectCloud 0x0020
#define WIFI_BIT_RSSI_OFFSET    6     // bit6-bit8
typedef struct
{
   T_MSG_HEADER header;
   uint16_t wifi_status;   // WIFI ״̬
   T_MSG_TAILER tailer;
}T_WifiStatusChangeNotify_Req;   //WIFI ״̬�仯֪ͨ����
typedef T_General_Cmd T_WifiStatusChangeNotify_Resp; //WIFI ״̬�仯֪ͨ��Ӧ

typedef struct
{
   T_MSG_HEADER header;
   uint8_t action;
   T_MSG_TAILER tailer;
}T_ReadDeviceCurStatus_Req;   // ��ȡ�豸��ǰ״̬����
typedef struct
{
    uint32_t hour;
	uint32_t min;
	uint32_t sec;
	uint8_t  sys_status;
	uint16_t pm25;   // 0 - 9999
	uint32_t forma;  
	uint32_t temperatue;
	uint32_t humiduty;
	uint32_t batCapacity;  // battery capacity
}T_DEVICE_STATUS;  // 31 B
typedef struct
{
   uint32_t hour;
   uint32_t min;
   uint32_t sec;
}T_TimeHSM;
typedef struct
{
   T_MSG_HEADER header;
   uint8_t action;
   T_DEVICE_STATUS tDevStatus;
   T_MSG_TAILER tailer;
}T_ReadDeviceCurStatus_Resp, T_ReadDeviceCurStatus_Report; // ��ȡ�豸��ǰ״̬��Ӧ, �����ϱ�
typedef T_General_Cmd T_ReadDeviceCurStatus_Report_Resp;  

typedef enum
{
    E_HOUR_BIT = 0x01,
	E_MIN_BIT  = 0x02,
	E_SEC_BIT  = 0x04,
}E_ATTR_FLAGS_BIT;
typedef struct
{
   T_MSG_HEADER header; 
   uint8_t action;
   uint8_t attr_flags;  // ���Ա�־λ
   T_TimeHSM timeSet;   
   T_MSG_TAILER tailer;
}T_DeviceTimeSet_Req;   // ����ʱ��
typedef T_General_Cmd T_DeviceTimeSet_Resp;

typedef struct
{
   uint8_t cmd;
   uint8_t (*ParseCmdFunc)(void *pMsg);  // ���������
   uint8_t (*MakeCmdFunc)(void *pMsg);   // �����������
}T_CMD_MAP;  // ����ӳ�䴦���
#pragma upack (1)


void FLASH_SAVE ATCMD_Init(void);
void FLASH_SAVE ATUART_RxIntServer(uint8_t *data, int32_t len);
uint8_t FLASH_SAVE CMDFN_SendConfigMode_Req(E_CONFIG_MODE configMode);
uint8_t FLASH_SAVE CMDFN_SendRestartWifi_Req(void);
uint8_t FLASH_SAVE  CMDFN_SendWifiStatusChangeNotify_Req(uint16_t wifi_status);
uint8_t FLASH_SAVE CMDFN_SendReadDeviceCurStatus_Req(void);
uint8_t FLASH_SAVE CMDFN_SendReadDeviceCurStatusReport(void);
uint8_t FLASH_SAVE CMDFN_SendDeviceTimeSet_Req(void);

void FLASH_SAVE DeviceCurBatteryCapacityUpdate(uint32_t batCap);
void FLASH_SAVE DeviceCurPM25ValUpdate(uint16_t pm25Val);
void FLASH_SAVE DeviceCurFormaValUpdate(uint32_t formaVal);
void FLASH_SAVE DeviceCurHumiValUpdate(uint32_t humiVal);
void FLASH_SAVE DeviceCurTempValUpdate(uint32_t tempVal);
void FLASH_SAVE DevceCurSysStatusUpdate(uint8_t sys_status);
void FLASH_SAVE DeviceCurTimeUpdate(uint32_t hour, uint32_t min, uint32_t sec);
#if GIZ_CMD_TEST_EN
E_BOOL FLASH_SAVE GetRxCfgModeReqFlag(void);
void FLASH_SAVE SetRxCfgModeReqFlag(E_BOOL flag);
E_BOOL FLASH_SAVE GetRxRestartWifiReqFlag(void);
void FLASH_SAVE SetRxRestartWifiReqFlag(E_BOOL flag);
E_BOOL FLASH_SAVE GetRxTimeSetReqFlag(void);
void FLASH_SAVE SetRxTimeSetReqFlag(E_BOOL flag);
#endif

E_BOOL FLASH_SAVE Sensor_HasTemperatureData(uint32_t *temperatureVal);

#endif
