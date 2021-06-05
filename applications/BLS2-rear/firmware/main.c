/** 
 *
 * BLS2 - Bike Lighting System 2 - rear light firmware: main.c
 *
 */

#include "global.h"
#include <stdbool.h>
#include <stdint.h>


int main(void)
{

 SEGGER_RTT_printf(0, "Firmware start\n");

 system_init();

  for (;;)
   {
        // Wait for an event.
        __WFE();

        // Clear the event register.
        __SEV();
        __WFE();

        NRF_LOG_FLUSH();
    }
 

}

