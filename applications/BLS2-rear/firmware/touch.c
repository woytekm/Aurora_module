#include "global.h"
#include "touch.h"

void touch_event_timer_handler(void *p_context)
 {

  uint8_t i,event_code;
  uint16_t touch_patterns[11][MAX_TOUCH_EVENTS] =  // this should match with touch_events enum from touch.h 
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
                {4,0,4,0,4,0}};

  SEGGER_RTT_printf(0, "Touch event timer end\n");

  //for(i=0; i<m_touch_event_queue_idx; i++)
  // SEGGER_RTT_printf(0, " event: %d\n",m_touch_event_queue[i]);

  event_code = 254;

  for(i=0; i<11; i++)
   if(memcmp(&m_touch_event_queue,&touch_patterns[i],MAX_TOUCH_EVENTS*2) == 0) // *2 cause this is uint16_t array
    event_code = i;

  switch(event_code)
   {
     case T_L:
      SEGGER_RTT_printf(0, "touch event: T_L\n");
      break;

     case T_M:
      SEGGER_RTT_printf(0, "touch event: T_M\n");
      if(m_light_on && (m_led_program_duty > 2000))
       {
        light_stop();
        m_led_program_duty -= 1000;
        light_start(1,m_led_program_speed,m_led_program_brightness);
       }
      break;

     case T_R:
      SEGGER_RTT_printf(0, "touch event: T_R\n");
      break;

     case T_L_DT:
      SEGGER_RTT_printf(0, "touch event: T_L_DT\n");
      if(!m_GPS_on)
       GPS_enable();
      else 
       GPS_disable();
      break;


     case T_M_DT:
      SEGGER_RTT_printf(0, "touch event: T_M_DT\n");
      if(m_light_on && (m_led_program_duty < 60000))
       {
        light_stop();
        m_led_program_duty += 1000;
        light_start(1,m_led_program_speed,m_led_program_brightness);
       }
      break;

     case T_R_DT:
      SEGGER_RTT_printf(0, "touch event: T_R_DT\n");
      light_start(1,m_led_program_speed,m_led_program_brightness);
      break;

     case T_R_TT:
      SEGGER_RTT_printf(0, "touch event: T_R_TT\n");
      light_stop();
      break;

   }

  m_touch_event_in_progress = false;
  m_touch_event_queue_idx = 0;

 }

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

