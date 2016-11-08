
#include "Util_Drv.h"


double Util_pow(double x, double y)
{
 register int i;
 double z , p=1;
 
 z = Util_fabs(y);
 
 for(i=0; i<z ; ++i)
 {
    p *= x;
 }
 if(y<0)
   return 1/p;
 else
   return p;
}

void FLASH_SAVE Util_PrintTime(void)
{
   uint32_t us;

   us = system_get_time();
   APP_DEBUG_PRINT("cur time = %ld ms\n", us / 1000);
}

