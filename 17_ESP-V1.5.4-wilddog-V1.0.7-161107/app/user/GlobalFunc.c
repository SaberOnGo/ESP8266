
#include "GlobalFunc.h"

// get system running time since power on
// return: run time, unit: ms
uint32_t FLASH_SAVE  Sys_GetRunTime(void)
{
   return (uint32_t)(system_get_time() / 1000);
}

void FLASH_SAVE Sys_DelayMs(uint32_t ms)
{
    uint32_t i;

	for(i = 0; i < ms; i++)
	{
	   os_delay_us(1000);
	}
}

E_BOOL FLASH_SAVE Sys_IsBigEndian(void)
{
   union
   {
      int  w16;
	  char c8;
   }uNum;

   uNum.w16 = 0x1234;
   if(0x12 == uNum.c8)  // low addr, MSB : bigEnddian
   {
      return E_TRUE;
   }
   else
   {
      return E_FALSE;
   }
}

#if ALL_STRING_SAVE_IN_FLASH
static const char RO_DATA_IN_FLASH device_protocol_ver[]=  "COMV1.00";  // 8B
static const char RO_DATA_IN_FLASH device_private_ver[] =  "PV1.00R0";  // 8B
static const char RO_DATA_IN_FLASH  device_pcb_ver[]    =  "HV1.00R1";   // 8B
static const char RO_DATA_IN_FLASH device_soft_ver[]    =  "SV0.01T1";   // 8B
static const char RO_DATA_IN_FLASH device_product_key[] = "Arduino Nano V3.00";  // 32B
#else
#define DEVICE_PROTOCOL_VER   "COMV1.00"   // serial version: 8B,V0.01
#define DEVICE_PRIVATE_VER    "PV1.00R0"
#define DEVICE_PCB_VER        "HV1.00R1"   // PCB version: 8B, V1.00
#define DEVICE_SOFT_VER       "SV0.01T1"   // Software 8B
#define DEVICE_PRODUCT_KEY    "Arduino Nano V3.00"   // 32
#endif
E_RESULT Sys_GetVersion(uint8_t *outString, int16_t stringSize, E_VERION_TYPE index)
{
   #if ALL_STRING_SAVE_IN_FLASH
   uint16_t len = 0;
   uint8_t i;
   #endif
   if(NULL == outString || stringSize < 1)return APP_FAILED;
   switch(index)
   {
       case E_PROTOCOL_VER:
	   {   
	   	   #if ALL_STRING_SAVE_IN_FLASH
	   	   len = strlen_P(device_protocol_ver);
		   if(len > stringSize)len = stringSize;
		   for(i = 0; i < len; i++)
		   {
		      outString[i] = pgm_read_byte_near(device_protocol_ver + i);
		   }
		   #else
           os_strncpy(outString, DEVICE_PROTOCOL_VER, stringSize);
		   #endif
	   }break;
	   case E_PRIVATE_VER:
	   {
	   	   #if ALL_STRING_SAVE_IN_FLASH
	   	   len = strlen_P(device_private_ver);
		   if(len > stringSize)len = stringSize;
		   for(i = 0; i < len; i++)
		   {
		      outString[i] = pgm_read_byte_near(device_private_ver + i);
		   }
		   #else
           os_strncpy(outString, DEVICE_PRIVATE_VER, stringSize);
		   #endif
	   }break;
	   case E_HARD_VER:
	   {
	   	   #if ALL_STRING_SAVE_IN_FLASH
	   	   len = strlen_P(device_pcb_ver);
		   if(len > stringSize)len = stringSize;
		   for(i = 0; i < len; i++)
		   {
		      outString[i] = pgm_read_byte_near(device_pcb_ver + i);
		   }
		   #else
           os_strncpy(outString, DEVICE_PCB_VER, stringSize);
		   #endif
	   }break;
	   case E_SOFT_VER:
	   {
	   	   #if ALL_STRING_SAVE_IN_FLASH
	   	   len = strlen_P(device_soft_ver);
		   if(len > stringSize)len = stringSize;
		   for(i = 0; i < len; i++)
		   {
		      outString[i] = pgm_read_byte_near(device_soft_ver + i);
		   }
		   #else
           os_strncpy(outString, DEVICE_SOFT_VER, stringSize);
		   #endif
	   }break;
	   case E_PRODUCT_KEY:
	   {
	   	   #if ALL_STRING_SAVE_IN_FLASH
	   	   len = strlen_P(device_product_key);
		   if(len > stringSize)len = stringSize;
		   for(i = 0; i < len; i++)
		   {
		      outString[i] = pgm_read_byte_near(device_product_key + i);
		   }
		   #else
           os_strncpy(outString, DEVICE_PRODUCT_KEY, stringSize);
		   #endif
	   }break;
	   default:break;
   }
   return APP_SUCCESS;
}
