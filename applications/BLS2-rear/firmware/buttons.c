#include "global.h"
#include <strings.h>

void map_buttons(char *b1, char *b2, char *b3, char *b4, char *b5, char *b6)
 {
   strncpy(m_buttons[0].btn_text,b1,DSCR_LEN);
   strncpy(m_buttons[1].btn_text,b2,DSCR_LEN);
   strncpy(m_buttons[2].btn_text,b3,DSCR_LEN);
   strncpy(m_buttons[3].btn_text,b4,DSCR_LEN);
   strncpy(m_buttons[4].btn_text,b5,DSCR_LEN);
   strncpy(m_buttons[5].btn_text,b6,DSCR_LEN);
 }

void zero_buttons()
 {
   bzero(m_buttons[0].btn_text,DSCR_LEN);
   bzero(m_buttons[1].btn_text,DSCR_LEN);
   bzero(m_buttons[2].btn_text,DSCR_LEN);
   bzero(m_buttons[3].btn_text,DSCR_LEN);
   bzero(m_buttons[4].btn_text,DSCR_LEN);
   bzero(m_buttons[5].btn_text,DSCR_LEN);
 }

void clear_buttons()
 {
   m_buttons[0].press = false;
   m_buttons[1].press = false;
   m_buttons[2].press = false;
   m_buttons[3].press = false;
   m_buttons[4].press = false;
   m_buttons[5].press = false;
 }

void button_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
 {

   SEGGER_RTT_printf(0, "button_handler triggered: %d, (%d,%d).\n",pin,PIN_BTN1,PIN_BTN6);

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

  if((pin == PIN_BTN4) && (action ==  NRF_GPIOTE_POLARITY_HITOLO))
    {
     m_buttons[3].press = true;
    }

  if((pin == PIN_BTN5) && (action ==  NRF_GPIOTE_POLARITY_HITOLO))
    {
     m_buttons[4].press = true;
    }

   if((pin == PIN_BTN6) && (action ==  NRF_GPIOTE_POLARITY_HITOLO))
    {
     m_buttons[5].press = true;
    }
 }

void init_buttons(void)
 {
    ret_code_t err_code;

    SEGGER_RTT_printf(0, "buttons init.\n");
    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(false);
    in_config.pull = NRF_GPIO_PIN_NOPULL;

    err_code = nrf_drv_gpiote_in_init(PIN_BTN1, &in_config, button_handler);
    err_code = nrf_drv_gpiote_in_init(PIN_BTN2, &in_config, button_handler);
    err_code = nrf_drv_gpiote_in_init(PIN_BTN3, &in_config, button_handler);
    err_code = nrf_drv_gpiote_in_init(PIN_BTN4, &in_config, button_handler);
    err_code = nrf_drv_gpiote_in_init(PIN_BTN5, &in_config, button_handler);
    err_code = nrf_drv_gpiote_in_init(PIN_BTN6, &in_config, button_handler);

    SEGGER_RTT_printf(0, "GPIOTE init done (%d)\n",err_code);
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_event_enable(PIN_BTN1, true);
    nrf_drv_gpiote_in_event_enable(PIN_BTN2, true);
    nrf_drv_gpiote_in_event_enable(PIN_BTN3, true);
    nrf_drv_gpiote_in_event_enable(PIN_BTN4, true);
    nrf_drv_gpiote_in_event_enable(PIN_BTN5, true);
    nrf_drv_gpiote_in_event_enable(PIN_BTN6, true);


 }
