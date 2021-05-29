#include "global.h"

 
void timer_init(void)
 {

  ret_code_t err_code;

  err_code = app_timer_create(&m_touch_event_timer,
                               APP_TIMER_MODE_SINGLE_SHOT,
                               touch_event_timer_handler);
  // subsequent app timers created here
  
  APP_ERROR_CHECK(err_code);

 }

static void lfclk_request(void)
{
    ret_code_t err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);
    nrf_drv_clock_lfclk_request(NULL);
}

uint8_t system_init(void)
 {

   m_touch_event_in_progress = false;

   twi_init();
   MPR121_init();

   SEGGER_RTT_printf(0,"lfclk_request()\n");
   lfclk_request();
   SEGGER_RTT_printf(0,"app_timer_init()\n");
   app_timer_init();
   SEGGER_RTT_printf(0,"timer_init()\n");
   timer_init();

   SEGGER_RTT_printf(0,"touch_IRQ_init()\n");
   touch_IRQ_init();

   return 0;

 } 
