#include "global.h"

uint8_t system_init(void)
 {
 
   twi_init();
   MPR121_init();
   
   touch_IRQ_init();

   return 0;
      
 }
 
 
