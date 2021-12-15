#include "global.h"

void timer_init(void)
 {

  ret_code_t err_code;

  err_code = app_timer_create(&m_touch_event_timer,
                               APP_TIMER_MODE_SINGLE_SHOT,
                               touch_event_timer_handler);
  APP_ERROR_CHECK(err_code);

  err_code = app_timer_create(&m_touch_reset_timer,
                               APP_TIMER_MODE_REPEATED,
                               touch_reset_timer_handler);
  APP_ERROR_CHECK(err_code);
  // subsequent app timers created here
  
  #define TOUCH_RST_TIMER_INTERVAL APP_TIMER_TICKS(3000)

  //err_code = app_timer_start(m_touch_reset_timer, TOUCH_RST_TIMER_INTERVAL, NULL);
  APP_ERROR_CHECK(err_code);

 }

static void lfclk_request(void)
{
    //ret_code_t err_code = nrf_drv_clock_init();
    //SEGGER_RTT_printf(0,"nrf_drv_clock_init(): %d\n",err_code);
    //APP_ERROR_CHECK(err_code);
    //nrf_drv_clock_lfclk_request(NULL);

  NRF_CLOCK->TASKS_HFCLKSTART = 1;

  while(0 == NRF_CLOCK ->EVENTS_HFCLKSTARTED)
   {
   }

  NRF_CLOCK->TASKS_LFCLKSTOP = 1;
   while(NRF_CLOCK->LFCLKSTAT);

  NRF_CLOCK->LFCLKSRC = CLOCK_LFCLKSRC_SRC_RC;
  NRF_CLOCK->TASKS_LFCLKSTART = 1;

}

uint8_t system_init(void)
 {

   m_touch_event_in_progress = false;

   twi_init();

   if(MPR121_init() == 0)
    {
     SEGGER_RTT_printf(0,"touch_IRQ_init()\n");
     touch_IRQ_init();
    } 
   
   nrf_delay_us(5000);

   m_touch_event_timer = (app_timer_t *) malloc(sizeof(app_timer_t));
   memset(m_touch_event_timer, 0, sizeof(app_timer_t));

   m_touch_reset_timer = (app_timer_t *) malloc(sizeof(app_timer_t));
   memset(m_touch_reset_timer, 0, sizeof(app_timer_t));

   m_led_program = 1;
   m_led_program_duty = 10000;   // step duration
   m_led_program_speed = 3;
   m_led_program_brightness = 1; // not used yet

   nrf_gpio_cfg_output(PIN_GPS_ENA);
   nrf_gpio_cfg_output(PIN_GPS_RST);
   nrf_gpio_cfg_output(PIN_BRD_BUZZER);
   nrf_gpio_cfg_output(USER_LED_1);
   nrf_gpio_cfg_output(USER_LED_2);
   nrf_gpio_cfg_output(USER_LED_3);
   nrf_gpio_cfg_output(PIN_BRD_LED);
  
   nrf_gpio_cfg_output(PIN_J33_08);
   nrf_gpio_cfg_output(PIN_J33_10);                    
   nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(0,29));

   nrf_gpio_pin_clear(PIN_J33_08);
   nrf_gpio_pin_clear(PIN_J33_10);                   
   nrf_gpio_pin_clear(NRF_GPIO_PIN_MAP(0,29));

   nrf_gpio_pin_clear(PIN_GPS_ENA);

   UART_config(0,PIN_GPS_TXD,0,PIN_GPS_RXD,UART_BAUDRATE_BAUDRATE_Baud38400,false);

   SEGGER_RTT_printf(0,"lfclk_request()\n");
   lfclk_request();
   SEGGER_RTT_printf(0,"app_timer_init()\n");
   app_timer_init();
   SEGGER_RTT_printf(0,"timer_init()\n");
   timer_init();
   SEGGER_RTT_printf(0,"init done.\n");
   return 0;

 } 
