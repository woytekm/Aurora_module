#include "global.h"
#include "adc.h"


void  adc_init(void)
 {

  state = 1;
  saadc_init();
  saadc_sampling_event_init();
  saadc_sampling_event_enable();

  nrf_delay_ms(500);

  uint32_t ticks = nrf_drv_timer_ms_to_ticks(&m_timer, 2000);

  nrf_drv_timer_disable(&m_timer);

  nrf_drv_timer_extended_compare(&m_timer,
                                   NRF_TIMER_CC_CHANNEL0,
                                   ticks,
                                   NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK,
                                   false);
  nrf_drv_timer_enable(&m_timer);

 }


void timer_handler(nrf_timer_event_t event_type, void * p_context)
 {

 }


void saadc_sampling_event_init(void)
{
    ret_code_t err_code;

    err_code = nrf_drv_ppi_init();
    APP_ERROR_CHECK(err_code);

    nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;
    timer_cfg.bit_width = NRF_TIMER_BIT_WIDTH_32;
    err_code = nrf_drv_timer_init(&m_timer, &timer_cfg, timer_handler);
    APP_ERROR_CHECK(err_code);

    /* setup m_timer for compare event every 400ms */
    uint32_t ticks = nrf_drv_timer_ms_to_ticks(&m_timer, 50);

    nrf_drv_timer_extended_compare(&m_timer,
                                   NRF_TIMER_CC_CHANNEL0,
                                   ticks,
                                   NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK,
                                   false);
    nrf_drv_timer_enable(&m_timer);

    uint32_t timer_compare_event_addr = nrf_drv_timer_compare_event_address_get(&m_timer,
                                                                                NRF_TIMER_CC_CHANNEL0);
    uint32_t saadc_sample_task_addr   = nrf_drv_saadc_sample_task_get();

    /* setup ppi channel so that timer compare event is triggering sample task in SAADC */
    err_code = nrf_drv_ppi_channel_alloc(&m_ppi_channel);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_ppi_channel_assign(m_ppi_channel,
                                          timer_compare_event_addr,
                                          saadc_sample_task_addr);
    APP_ERROR_CHECK(err_code);
}


void saadc_sampling_event_enable(void)
{
    ret_code_t err_code = nrf_drv_ppi_channel_enable(m_ppi_channel);

    APP_ERROR_CHECK(err_code);
}


void saadc_callback(nrf_drv_saadc_evt_t const * p_event)
{
  uint16_t  batt_avg;

    if (p_event->type == NRF_DRV_SAADC_EVT_DONE)
    {
        ret_code_t err_code;

        err_code = nrf_drv_saadc_buffer_convert(p_event->data.done.p_buffer, SAMPLES_IN_BUFFER);
        APP_ERROR_CHECK(err_code);

        int i;
        SEGGER_RTT_printf(0, "ADC event number: %d\n", (int)m_adc_evt_counter);

        batt_avg = 0;

        for (i = 0; i < SAMPLES_IN_BUFFER; i++)
          batt_avg += p_event->data.done.p_buffer[i];

        m_battery_level = batt_avg / SAMPLES_IN_BUFFER;
        SEGGER_RTT_printf(0,"battery level: %d\n", m_battery_level);

        m_adc_evt_counter++;
    }
}


void saadc_init(void)
{

    ret_code_t err_code;
    nrf_saadc_channel_config_t channel_config =
        NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN1);  // AIN1 = P0.03, AIN2 = P0.04

    err_code = nrf_drv_saadc_init(NULL, saadc_callback);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_channel_init(0, &channel_config);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_buffer_convert(m_buffer_pool[0], SAMPLES_IN_BUFFER);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_buffer_convert(m_buffer_pool[1], SAMPLES_IN_BUFFER);
    APP_ERROR_CHECK(err_code);

}


/**
 * @brief Function for main application entry.
 */
//int main(void)
//{
//    uint32_t err_code = NRF_LOG_INIT(NULL);
//    APP_ERROR_CHECK(err_code);
//
//    NRF_LOG_DEFAULT_BACKENDS_INIT();
//
//   ret_code_t ret_code = nrf_pwr_mgmt_init();
//    APP_ERROR_CHECK(ret_code);
//
//    saadc_init();
//    saadc_sampling_event_init();
//    saadc_sampling_event_enable();
//    NRF_LOG_INFO("SAADC HAL simple example started.");
//
//    while (1)
//    {
//        nrf_pwr_mgmt_run();
//        NRF_LOG_FLUSH();
//    }
//}
//

/** @} */
