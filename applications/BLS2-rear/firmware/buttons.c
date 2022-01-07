#include "buttons.h"

#include "nrf_drv_twi.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "SEGGER_RTT.h"

#include "nrf_drv_gpiote.h"

#include "nrf_delay.h"

#include "boards.h"

#include "app_timer.h"
#include "app_scheduler.h"
#include "nrf_drv_clock.h"
#include "nrf_gpiote.h"
#include "nrf_drv_gpiote.h"

#include "app_util_platform.h"
#include "bsp.h"

#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

void clear_buttons()
 {
   m_buttons[0].press = false;
   m_buttons[1].press = false;
   m_buttons[2].press = false;
 }

void button_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
 {

   SEGGER_RTT_printf(0, "button_handler triggered: %d\n",pin);

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
