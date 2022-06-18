/** 
 *
 * BLS2 - Bike Lighting System 2 - rear light firmware: main.c
 *
 */

#include "global.h"
#include <stdbool.h>
#include <stdint.h>


static void fstorage_evt_handler(nrf_fstorage_evt_t * p_evt)
{
    if (p_evt->result != NRF_SUCCESS)
    {
        NRF_LOG_INFO("--> Event received: ERROR while executing an fstorage operation.");
        return;
    }

    switch (p_evt->id)
    {
        case NRF_FSTORAGE_EVT_WRITE_RESULT:
        {
            NRF_LOG_INFO("--> Event received: wrote %d bytes at address 0x%x.",
                         p_evt->len, p_evt->addr);
        } break;

        case NRF_FSTORAGE_EVT_ERASE_RESULT:
        {
            NRF_LOG_INFO("--> Event received: erased %d page from address 0x%x.",
                         p_evt->len, p_evt->addr);
        } break;

        default:
            break;
    }
}


NRF_FSTORAGE_DEF(nrf_fstorage_t fstorage) =
{
    /* Set a handler for fstorage events. */
    .evt_handler = fstorage_evt_handler,

    /* These below are the boundaries of the flash space assigned to this instance of fstorage.
     * You must set these manually, even at runtime, before nrf_fstorage_init() is called.
     * The function nrf5_flash_end_addr_get() can be used to retrieve the last address on the
     * last page of flash available to write data. */
    .start_addr = 0x70000,
    .end_addr   = 0x7ffff,
};


void wait_for_flash_ready(nrf_fstorage_t const * p_fstorage)
{
    /* While fstorage is busy, sleep and wait for an event. */
    while (nrf_fstorage_is_busy(p_fstorage))
    {
        __WFE();
        // Clear the event register.
        __SEV();
        __WFE();
    }
}



static uint32_t nrf5_flash_end_addr_get()
{
    uint32_t const bootloader_addr = NRF_UICR->NRFFW[0];
    uint32_t const page_sz         = NRF_FICR->CODEPAGESIZE;
    uint32_t const code_sz         = NRF_FICR->CODESIZE;

    return (bootloader_addr != 0xFFFFFFFF ?
            bootloader_addr : (code_sz * page_sz));
}


void save_state(uint8_t state)
 {
   ret_code_t ret;
   uint8_t data[4] = {0x0};
   data[0] = state;

   SEGGER_RTT_printf(0,"Erasing flash.\n");
   ret = nrf_fstorage_erase(&fstorage, 0x70000, 1, NULL);
   SEGGER_RTT_printf(0,"Erasing result: %d\n",ret);
   APP_ERROR_CHECK(ret);
   SEGGER_RTT_printf(0,"Writing %x%x%x%x to flash.", data[0],data[1],data[2],data[3]);
   ret = nrf_fstorage_write(&fstorage, 0x70000, &data, sizeof(data), NULL);
   APP_ERROR_CHECK(ret);
   wait_for_flash_ready(&fstorage);
 }


uint8_t read_state(void)
 {
  ret_code_t ret;
  uint8_t state[4];

  ret = nrf_fstorage_read(&fstorage, 0x70000, &state, sizeof(state));
  APP_ERROR_CHECK(ret);
  SEGGER_RTT_printf(0,"READ state %x%x%x%x from flash.\n", state[0],state[1],state[2],state[3]);
  return state[0];
 }


void pstorage_init(void)
 {
   ret_code_t ret;
#ifdef SOFTDEVICE_PRESENT
   p_fs_api = &nrf_fstorage_sd;
#else
   p_fs_api = &nrf_fstorage_nvmc;
#endif
   ret = nrf_fstorage_init(&fstorage, p_fs_api, NULL);
   APP_ERROR_CHECK(ret);

 }


int main(void)
{

 ret_code_t ret;
 uint8_t rest_state = 0;

 ret = NRF_LOG_INIT(NULL);
 APP_ERROR_CHECK(ret);
 NRF_LOG_DEFAULT_BACKENDS_INIT();

 SEGGER_RTT_printf(0, "Firmware start\n");

 SEGGER_RTT_printf(0,"clk_init()\n");
 clk_init();

 SEGGER_RTT_printf(0,"app_timer_init()\n");
 app_timer_init();

 SEGGER_RTT_printf(0, "Pstorage init.\n");
 pstorage_init();

 nrf_gpio_cfg_output(PIN_J33_08);
 nrf_gpio_cfg_output(PIN_J33_10);
 nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(0,29));
 nrf_gpio_pin_clear(PIN_J33_08);
 nrf_gpio_pin_clear(PIN_J33_10);
 nrf_gpio_pin_clear(NRF_GPIO_PIN_MAP(0,29));

 nrf_gpio_cfg_output(USER_LED_2);

 system_init();

rest_state = read_state();

 if(rest_state > 0)
  {
   m_light_mode = rest_state;
   light_start();
  }


 for (;;)
   {   
        // Wait for an event.
        __WFE();

        // Clear the event register.
        __SEV();
        __WFE();

        //app_sched_execute();
    }
 

}

