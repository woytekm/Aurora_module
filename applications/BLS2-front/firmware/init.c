#include "global.h"

void timer_init(void)
 {

  ret_code_t err_code;

  m_touch_event_timer = (app_timer_t *) malloc(sizeof(app_timer_t));
  memset(m_touch_event_timer, 0, sizeof(app_timer_t));

#ifdef USE_MPR121
  m_touch_reset_timer = (app_timer_t *) malloc(sizeof(app_timer_t));
  memset(m_touch_reset_timer, 0, sizeof(app_timer_t));
#else
  m_button_debounce_timer = (app_timer_t *) malloc(sizeof(app_timer_t));
  memset(m_button_debounce_timer, 0, sizeof(app_timer_t));
#endif

//  m_gpx_writer_timer = (app_timer_t *) malloc(sizeof(app_timer_t));
//  memset(m_gpx_writer_timer, 0, sizeof(app_timer_t));

//  m_shock_update_timer = (app_timer_t *) malloc(sizeof(app_timer_t));
//  memset(m_shock_update_timer, 0, sizeof(app_timer_t));

  err_code = app_timer_create(&m_touch_event_timer,
                               APP_TIMER_MODE_SINGLE_SHOT,
                               touch_event_timer_handler);
  SEGGER_RTT_printf(0,"timer create code %d\n",err_code);
  APP_ERROR_CHECK(err_code);

#ifndef USE_MPR121
  err_code = app_timer_create(&m_button_debounce_timer,
                               APP_TIMER_MODE_SINGLE_SHOT,
                               button_debounce_timer_handler);
  SEGGER_RTT_printf(0,"timer create code %d\n",err_code);
  APP_ERROR_CHECK(err_code);
#endif

//  err_code = app_timer_create(&m_gpx_writer_timer,
//                               APP_TIMER_MODE_REPEATED,
//                               gpx_writer_handler);
//  SEGGER_RTT_printf(0,"timer create code %d\n",err_code);
//  APP_ERROR_CHECK(err_code);

//  err_code = app_timer_create(&m_shock_update_timer,
//                               APP_TIMER_MODE_REPEATED,
//                               LIS3DH_update_shock_val2);
//  SEGGER_RTT_printf(0,"timer create code %d\n",err_code);
//  APP_ERROR_CHECK(err_code);

  // subsequent app timers created here

  #define GPX_WRITER_TIMER_INTERVAL APP_TIMER_TICKS(3000)
  #define SHOCK_UPDATE_TIMER_INTERVAL APP_TIMER_TICKS(500)

//  err_code = app_timer_start(m_gpx_writer_timer, GPX_WRITER_TIMER_INTERVAL, NULL);
//  err_code = app_timer_start(m_shock_update_timer, SHOCK_UPDATE_TIMER_INTERVAL, NULL);

#ifdef USE_MPR121

  err_code = app_timer_create(&m_touch_reset_timer,
                               APP_TIMER_MODE_REPEATED,
                               touch_reset_timer_handler);
  SEGGER_RTT_printf(0,"timer create code %d\n",err_code);
  APP_ERROR_CHECK(err_code);
  
  #define TOUCH_RST_TIMER_INTERVAL APP_TIMER_TICKS(3000)

  //err_code = app_timer_start(m_touch_reset_timer, TOUCH_RST_TIMER_INTERVAL, NULL);

#endif

 }

void clk_init(void)
{
  ret_code_t err_code = nrf_drv_clock_init();
  SEGGER_RTT_printf(0,"nrf_drv_clock_init(): %d\n",err_code);
  APP_ERROR_CHECK(err_code);
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

   // wake up on pending interrupts
   SCB->SCR |= SCB_SCR_SEVONPEND_Msk;

   twi_init();

   m_led_step = 1;
   m_button_debounce_active = false;
   m_touch_event_in_progress = false;
   m_led_program = 1;
   m_led_program_duty = 10000;   // step duration
   m_led_program_speed = 3;
   m_led_program_brightness = 1; // not used yet
   m_SPI_mutex = false;
   m_prev_GPS_state = false;
   m_led_steps[0] = USER_LED_1;
   m_led_steps[1] = USER_LED_2;
   m_led_steps[2] = USER_LED_3;

   G_pos_write_delay = 0;
 
   G_gpx_write_position = false;
   G_time_synced = false;

   m_shock_val = 0;

   m_X_prev=m_Y_prev=m_Z_prev=0;
   m_X_factor=m_Y_factor=m_Z_factor=0;

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

   //UART_config(0,PIN_GPS_TXD,0,PIN_GPS_RXD,UART_BAUDRATE_BAUDRATE_Baud38400,false);

   SEGGER_RTT_printf(0,"timer_init()\n");
   timer_init();


#ifdef  USE_MPR121

   if(MPR121_init() == 0)
    {
     SEGGER_RTT_printf(0,"touch_IRQ_init()\n");
     touch_IRQ_init();
    }

   nrf_delay_us(5000);

#else

   init_buttons();

#endif

   SEGGER_RTT_printf(0,"init done.\n");

   blink_led(USER_LED_1);
   blink_led(USER_LED_2);
   blink_led(USER_LED_3);

   return 0;
  
 } 
