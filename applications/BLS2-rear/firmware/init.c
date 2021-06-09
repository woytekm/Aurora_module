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

   m_touch_event_timer = (app_timer_t *) malloc(sizeof(app_timer_t));
   memset(m_touch_event_timer, 0, sizeof(app_timer_t));

   m_led_program_duty = 10000;   // step duration
   m_led_program_speed = 3;
   m_led_program_brightness = 1; // not used yet

   nrf_gpio_cfg_output(PIN_GPS_ENA);
   nrf_gpio_cfg_output(PIN_GPS_RST);
   nrf_gpio_cfg_output(PIN_BRD_BUZZER);
   nrf_gpio_cfg_output(PIN_BRD_LED);

   //UART_config(0,PIN_GPS_TXD,0,PIN_GPS_RXD,UART_BAUDRATE_BAUDRATE_Baud38400,false);

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
