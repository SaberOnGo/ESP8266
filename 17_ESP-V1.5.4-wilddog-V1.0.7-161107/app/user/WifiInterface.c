
#include "WifiInterface.h"


E_BOOL FLASH_SAVE my_wifi_is_config_mode_on(void)
{
   E_BOOL result = E_FALSE;

   if(E_TRUE == wifi_station_get_config(NULL))result = E_TRUE;
   if(E_TRUE == wifi_station_get_config_default(NULL))result = E_TRUE;

   return result;
}

E_BOOL FLASH_SAVE my_wifi_is_binding_on(void)
{
   return E_FALSE;
}

E_BOOL FLASH_SAVE my_wifi_is_connect_cloud(void)
{
   return E_FALSE;
}


