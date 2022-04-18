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

 ret_code_t ret;

 ret = NRF_LOG_INIT(NULL);
 APP_ERROR_CHECK(ret);
 NRF_LOG_DEFAULT_BACKENDS_INIT();

 SEGGER_RTT_printf(0, "Firmware start\n");

 SEGGER_RTT_printf(0,"clk_init()\n");
 clk_init();

 SEGGER_RTT_printf(0,"app_timer_init()\n");
 app_timer_init();

 nrf_gpio_cfg_output(PIN_J33_08);
 nrf_gpio_cfg_output(PIN_J33_10);
 nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(0,29));
 nrf_gpio_pin_clear(PIN_J33_08);
 nrf_gpio_pin_clear(PIN_J33_10);
 nrf_gpio_pin_clear(NRF_GPIO_PIN_MAP(0,29));

 nrf_gpio_cfg_output(USER_LED_2);

 system_init();


 for (;;)
   {   
        // Wait for an event.
        __WFE();

        // Clear the event register.
        __SEV();
        __WFE();

        //app_sched_execute();
    }
 

}

