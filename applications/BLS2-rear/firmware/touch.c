#include "global.h"
#include "touch.h"
#include "mpr121.h"

void touch_reset_timer_handler(void *p_context)
 {
 
   SEGGER_RTT_printf(0, "touch reset: invoked \n");
   uint8_t reg_addr_04 = 0x04;
   uint8_t reg_filt_data_lsb = 0x00;

   uint8_t reg_addr_05 = 0x05;
   uint8_t reg_filt_data_msb = 0x00;

   uint8_t reg_addr_1e = 0x1E;
   uint8_t reg_baseline = 0x00;

   nrf_drv_twi_tx(&m_twi_0, MPR121_I2C_ADDR, &reg_addr_04, sizeof(uint8_t),true);
   nrf_drv_twi_rx(&m_twi_0, MPR121_I2C_ADDR, &reg_filt_data_lsb, sizeof(uint8_t));

   nrf_drv_twi_tx(&m_twi_0, MPR121_I2C_ADDR, &reg_addr_05, sizeof(uint8_t),true);
   nrf_drv_twi_rx(&m_twi_0, MPR121_I2C_ADDR, &reg_filt_data_msb, sizeof(uint8_t));

   nrf_drv_twi_tx(&m_twi_0, MPR121_I2C_ADDR, &reg_addr_1e, sizeof(uint8_t),true);
   nrf_drv_twi_rx(&m_twi_0, MPR121_I2C_ADDR, &reg_baseline, sizeof(uint8_t));

   SEGGER_RTT_printf(0,"MPR121: reg 0x%X: 0x%X\n",reg_addr_04,reg_filt_data_lsb);
   SEGGER_RTT_printf(0,"MPR121: reg 0x%X: 0x%X\n",reg_addr_05,reg_filt_data_msb);
   SEGGER_RTT_printf(0,"MPR121: reg 0x%X: 0x%X\n",reg_addr_1e,reg_baseline);

   //if(m_GPS_on)
   // {
   //   SEGGER_RTT_printf(0, "touch reset: resettig touch controller...%d\n");
   //   MPR121_off();
   //   nrf_delay_us(5000);
   //   MPR121_on_no_baseline();
   // }

 }
void touch_event_timer_handler(void *p_context)
 {

  uint8_t i,event_code;
  uint16_t touch_patterns[TOUCH_PATTERNS][MAX_TOUCH_EVENTS] =  // this should match with touch_events enum from touch.h 
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

  SEGGER_RTT_printf(0, "Touch event timer end\n");

  //for(i=0; i<m_touch_event_queue_idx; i++)
  // SEGGER_RTT_printf(0, " event: %d\n",m_touch_event_queue[i]);

  event_code = 254;

  for(i=0; i<TOUCH_PATTERNS; i++)
   if(memcmp(&m_touch_event_queue,&touch_patterns[i],MAX_TOUCH_EVENTS*2) == 0) // *2 cause this is uint16_t array
    event_code = i;

  switch(event_code)
   {
     case T_L:
      SEGGER_RTT_printf(0, "touch event: T_L\n");
      break;

     case T_M:
      SEGGER_RTT_printf(0, "touch event: T_M\n");
      if(m_light_on && (m_led_program_duty < 60000))
       {
        light_stop();
        m_led_program_duty += 1000;
        light_start(m_led_program,m_led_program_speed,m_led_program_brightness);
       }
      break;

     case T_R:
      SEGGER_RTT_printf(0, "touch event: T_R\n");
      break;

     case T_L_DT:
      SEGGER_RTT_printf(0, "touch event: T_L_DT\n");
      if(!m_GPS_on)
       {
         MPR121_off();
         GPS_enable();
         nrf_delay_ms(3000);
         //MPR121_init();
         MPR121_on_no_baseline();
       }
      else 
       {
        MPR121_off();
        GPS_disable();
        nrf_delay_us(5000);
        MPR121_init();
        MPR121_on_no_baseline();
       }
      break;

     case T_M_DT:
      SEGGER_RTT_printf(0, "touch event: T_M_DT\n");
      if(m_light_on && (m_led_program_duty < 60000))
       {
        light_stop();
        m_led_program_duty -= 1000;
        light_start(m_led_program,m_led_program_speed,m_led_program_brightness);
       }
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
       }
      break;

     case T_R_DT:
      SEGGER_RTT_printf(0, "touch event: T_R_DT\n");
      light_start(m_led_program,m_led_program_speed,m_led_program_brightness);
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

