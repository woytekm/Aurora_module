#include "global.h"
#include "buttons.h"

void clear_buttons()
 {
   m_buttons[0].press = false;
   m_buttons[1].press = false;
   m_buttons[2].press = false;
 }

void button_debounce_timer_handler(void *p_context)
 {
   m_button_debounce_active = false;
 }

void button_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
 {

  ret_code_t err_code;
  uint16_t touch_event;

  SEGGER_RTT_printf(0, "button_handler triggered: %d\n",pin);

  if(m_button_debounce_active)
   return;
  else
   {
    m_button_debounce_active = true;
    err_code = app_timer_start(m_button_debounce_timer, APP_TIMER_TICKS(90), NULL);

    if(err_code > 0)
      SEGGER_RTT_printf(0,"debounce timer error (%d)(%X)(%X)\n",err_code,m_touch_event_timer,touch_event_timer_handler);

   }

  if((pin == PIN_BTN1) && (action ==  NRF_GPIOTE_POLARITY_HITOLO))
    {
     m_buttons[0].press = true;
    }

  if((pin == PIN_BTN2) && (action ==  NRF_GPIOTE_POLARITY_HITOLO))
    {
     m_buttons[1].press = true;
    }

  if((pin == PIN_BTN3) && (action ==  NRF_GPIOTE_POLARITY_HITOLO))
    {
     m_buttons[2].press = true;
    }

  touch_event =  pin;

  if(!m_touch_event_in_progress)
      {
        SEGGER_RTT_printf(0,"new touch event start\n");
        m_touch_event_in_progress = true;
        memset(&m_touch_event_queue,0,MAX_TOUCH_EVENTS*2); // *2 cause this is uint16_t array
        err_code = app_timer_start(m_touch_event_timer, APP_TIMER_TICKS(1000), NULL);
        if(err_code > 0)
         SEGGER_RTT_printf(0,"touch event timer error (%d)(%X)(%X)\n",err_code,m_touch_event_timer,touch_event_timer_handler);
        APP_ERROR_CHECK(err_code);
      }

  if(m_touch_event_queue_idx < MAX_TOUCH_EVENTS)
       m_touch_event_queue[m_touch_event_queue_idx++] = touch_event;

  NRF_LOG_FLUSH();

 }

void init_buttons(void)
 {
    ret_code_t err_code;

    SEGGER_RTT_printf(0, "buttons init.\n");
    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);

    nrf_gpio_cfg_input(PIN_BTN1,NRF_GPIO_PIN_NOPULL);
    nrf_gpio_cfg_input(PIN_BTN2,NRF_GPIO_PIN_NOPULL);
    nrf_gpio_cfg_input(PIN_BTN3,NRF_GPIO_PIN_NOPULL);
    
    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(false);
    in_config.pull = NRF_GPIO_PIN_NOPULL;

    err_code = nrf_drv_gpiote_in_init(PIN_BTN1, &in_config, button_handler);
    err_code = nrf_drv_gpiote_in_init(PIN_BTN2, &in_config, button_handler);
    err_code = nrf_drv_gpiote_in_init(PIN_BTN3, &in_config, button_handler);

    SEGGER_RTT_printf(0, "GPIOTE init done (%d)\n",err_code);
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_event_enable(PIN_BTN1, true);
    nrf_drv_gpiote_in_event_enable(PIN_BTN2, true);
    nrf_drv_gpiote_in_event_enable(PIN_BTN3, true);

 }
