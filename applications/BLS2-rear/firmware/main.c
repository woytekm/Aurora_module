/** 
 *
 * BLS2 - Bike Lighting System 2 - rear light firmware: main.c
 *
 */

#include "global.h"
#include <stdbool.h>
#include <stdint.h>

#define MYLED7 NRF_GPIO_PIN_MAP(1,02)

int main(void)
{

 SEGGER_RTT_printf(0, "Firmware start\n");
 nrf_gpio_cfg_output(MYLED7);

 system_init();

 nrf_gpio_cfg_output(PIN_GPS_ENA);
 nrf_gpio_cfg_output(PIN_GPS_RST);
 nrf_gpio_pin_set(PIN_GPS_ENA); 

 //nrf_gpio_pin_set(PIN_GPS_RST);
 //nrf_delay_ms(1000);
 //nrf_gpio_pin_clear(PIN_GPS_RST);
 //nrf_delay_ms(1000);
 //nrf_gpio_pin_set(PIN_GPS_RST);

 //uint8_t i;

  for (;;)
   {

        //i = 0;

        //while(i<100)
        // {
        //   i++;       
        //   nrf_gpio_pin_write(MYLED7, 1);
        //   nrf_delay_us(90000);
        //   nrf_gpio_pin_write(MYLED7, 0);
        //   nrf_delay_us(90000);
        // }

        nrf_delay_us(900000);
       
        // Wait for an event.
        __WFE();

        // Clear the event register.
        __SEV();
        __WFE();

        NRF_LOG_FLUSH();
    }
 

}

