#include "global.h"

void touch_IRQ_init(void)
 {
    ret_code_t err_code;

    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(true);

    in_config.pull = NRF_GPIO_PIN_NOPULL;

    err_code = nrf_drv_gpiote_in_init(TOUCH_IRQ, &in_config, MPR121_check_pad_status);
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_event_enable(TOUCH_IRQ, true);

    SEGGER_RTT_printf(0, "Touch IRQ init complete.\n");
 }

void touch_event_timer_handler(void *p_context)
 {

  uint8_t i;
  
  SEGGER_RTT_printf(0, "Touch event summary:\n");

  for(i=0; i<m_touch_event_queue_idx; i++)
   SEGGER_RTT_printf(0, " event: %d\n",m_touch_event_queue[i]);  
  
  m_touch_event_in_progress = false;
  m_touch_event_queue_idx = 0;

 } 
