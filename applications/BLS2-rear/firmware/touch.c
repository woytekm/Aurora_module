#include "global.h"
#include "touch.h"
#include "mpr121.h"

void blink_led(uint16_t GPIO,uint8_t times)
 {
   uint8_t i;

   for(i=0;i<=times;i++)
    { 
     nrf_gpio_pin_set(GPIO);
     nrf_delay_ms(100);
     nrf_gpio_pin_clear(GPIO);
    }
 }


void show_battery_level(void)
 {
   uint8_t battery_level = m_battery_level - 123; // this should yield around 115 max (battery fully loaded), and around 0 min (battery empty)

   blink_led(USER_LED_1,1);
   blink_led(USER_LED_2,1);
   blink_led(USER_LED_3,1);

   if(battery_level < 20)
     blink_led(USER_LED_3,15);
   else if((battery_level >= 20) && (battery_level < 40))
     {
      nrf_gpio_pin_set(USER_LED_3);
      blink_led(USER_LED_2,15);
     }
   else if((battery_level >= 40) && (battery_level < 60))
     {
      nrf_gpio_pin_set(USER_LED_3);
      nrf_gpio_pin_set(USER_LED_2);
      nrf_delay_ms(1500);
     }
   else if((battery_level >= 60) && (battery_level < 80))
     {
      nrf_gpio_pin_set(USER_LED_3);
      nrf_gpio_pin_set(USER_LED_2);
      blink_led(USER_LED_1,15);
     }
   else if(battery_level >= 80)
     {
      nrf_gpio_pin_set(USER_LED_3);
      nrf_gpio_pin_set(USER_LED_2);
      nrf_gpio_pin_set(USER_LED_1);
      nrf_delay_ms(1500);
     }

    nrf_gpio_pin_clear(USER_LED_3);
    nrf_gpio_pin_clear(USER_LED_2);
    nrf_gpio_pin_clear(USER_LED_1);

 }

void touch_event_timer_handler(void *p_context)
 {

  uint8_t i,event_code;

#ifdef USE_MPR121
  uint16_t touch_patterns[TOUCH_PATTERNS][MAX_TOUCH_EVENTS] =  // this should match with touch_events enum from touch.h  - touch controller events
                {{1,0,0,0,0,0},
                {2,0,0,0,0,0},
                {4,0,0,0,0,0},
                {1,0,1,0,0,0},
                {2,0,2,0,0,0},
                {4,0,4,0,0,0},
                {1,0,4,0,0,0},
                {1,0,2,0,0,0},
                {1,0,4,0,1,0},
                {2,0,4,0,2,0},
                {4,0,4,0,4,0},
                {2,0,2,0,2,0}};
#else
  uint16_t touch_patterns[TOUCH_PATTERNS][MAX_TOUCH_EVENTS] =  // this should match with touch_events enum from touch.h  - tact switch patterns
                {{8,0,0,0},
                {20,0,0,0},
                {10,0,0,0},
                {8,8,0,0},
                {20,20,0,0},
                {10,10,0,0},
                {8,8,8,0},
                {20,20,20,0},
                {10,10,10,0},
                {0,0,0,0},
                {0,0,0,0},
                {0,0,0,0}};
#endif

  SEGGER_RTT_printf(0, "Touch event timer end\n");

  for(i=0; i<=m_touch_event_queue_idx; i++)
   SEGGER_RTT_printf(0, " event: %d\n",m_touch_event_queue[i]);

  event_code = 254;

  for(i=0; i<TOUCH_PATTERNS; i++)
   if(memcmp(&m_touch_event_queue,&touch_patterns[i],MAX_TOUCH_EVENTS*2) == 0) // *2 cause this is uint16_t array
    event_code = i;

  switch(event_code)
   {
     case T_L:
      SEGGER_RTT_printf(0, "touch event: T_L\n");
      blink_led(USER_LED_2,1);
      break;

     case T_M:
      SEGGER_RTT_printf(0, "touch event: T_M\n");
      if(m_light_on && (m_led_program_duty < 60000))
       {
        light_stop();
        m_led_program_duty += 1000;
        light_start(m_led_program,m_led_program_speed,m_led_program_brightness);
       }
      blink_led(USER_LED_2,1);
      break;

     case T_R:
      SEGGER_RTT_printf(0, "touch event: T_R\n");
      blink_led(USER_LED_2,1);
      break;

     case T_L_DT:
      SEGGER_RTT_printf(0, "touch event: T_L_DT\n");
      if(!m_GPS_on)
       {
         GPS_enable();
         SEGGER_RTT_printf(0, "GPS logger on\n");
         m_GPS_logger_active = true;
         nrf_delay_ms(100);
#ifdef USE_MPR121
         nrf_delay_ms(3000);
         MPR121_init();
         MPR121_on_no_baseline();
#endif
       }
      else 
       {
        GPS_disable();
        nrf_delay_us(5000);
        SEGGER_RTT_printf(0, "GPS logger off\n");
        m_GPS_logger_active = false;
       }

      blink_led(USER_LED_2,1);
      break;

     case T_M_DT:
      SEGGER_RTT_printf(0, "touch event: T_M_DT\n");
      if(m_light_on && (m_led_program_duty < 60000))
       {
        light_stop();
        m_led_program_duty -= 1000;
        light_start(m_led_program,m_led_program_speed,m_led_program_brightness);
       }
      blink_led(USER_LED_2,1);
      break;

     case T_M_TT:
      SEGGER_RTT_printf(0, "touch event: T_M_TT\n");
      if(m_light_on)
       {
        if(m_led_program == LED_PGMS)
          m_led_program = 1;
        else m_led_program++;
        light_stop();
        light_start(m_led_program,m_led_program_speed,m_led_program_brightness);
        save_state(m_led_program);
       }
      blink_led(USER_LED_2,1);
      break;

     case T_R_DT:
      SEGGER_RTT_printf(0, "touch event: T_R_DT\n");
      if(!m_light_on)
       {
        light_start(m_led_program,m_led_program_speed,m_led_program_brightness);
        save_state(m_led_program);
       }
      else
       {
        light_stop();
        save_state(0);
       }
      blink_led(USER_LED_2,1);
      break;

     case T_R_TT:
      SEGGER_RTT_printf(0, "touch event: T_R_TT\n");
      show_battery_level();
      break;

   }

  m_touch_event_in_progress = false;
  m_touch_event_queue_idx = 0;

 }

#ifdef USE_MPR121

void touch_IRQ_init(void)
 {
    ret_code_t err_code;

    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(true);

    in_config.pull = NRF_GPIO_PIN_NOPULL;

    err_code = nrf_drv_gpiote_in_init(TOUCH_IRQ_PIN, &in_config, MPR121_check_pad_status);
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_event_enable(TOUCH_IRQ_PIN, true);

    SEGGER_RTT_printf(0, "Touch IRQ init complete.\n");
 }

#endif
