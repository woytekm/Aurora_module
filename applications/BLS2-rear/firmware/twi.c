/* TWI instance ID. */
#if TWI0_ENABLED
#define TWI_INSTANCE_ID_0     0
#elif TWI1_ENABLED
#define TWI_INSTANCE_ID_1     1
#endif

 /* Number of possible TWI addresses. */
#define TWI_ADDRESSES      127

/* TWI instance. */
static const nrf_drv_twi_t m_twi_0 = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID_0);
static const nrf_drv_twi_t m_twi_1 = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID_1);

void twi_init (void)
{
    ret_code_t err_code;

    const nrf_drv_twi_config_t twi_0_config = {
       .scl                = NRF_GPIO_PIN_MAP(0,8),  //touch sensor, when present (@0x5A)
       .sda                = NRF_GPIO_PIN_MAP(0,10), //touch sensor, when present (@0x5A)
       .frequency          = NRF_DRV_TWI_FREQ_100K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
       .clear_bus_init     = false
    };


    const nrf_drv_twi_config_t twi_1_config = {
       .scl                = NRF_GPIO_PIN_MAP(1,0),  //accelerometer (@0x1D) + pressure/temp sensor (@0x77)
       .sda                = NRF_GPIO_PIN_MAP(0,24), //accelerometer (@0x1D) + pressure/temp sensor (@0x77)
       .frequency          = NRF_DRV_TWI_FREQ_100K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
       .clear_bus_init     = false
    };

    NRF_LOG_INFO("TWI init.\n");
    NRF_LOG_FLUSH();
    
    err_code = nrf_drv_twi_init(&m_twi_0, &twi_0_config, NULL, NULL);
    APP_ERROR_CHECK(err_code);
    
    err_code = nrf_drv_twi_init(&m_twi_1, &twi_1_config, NULL, NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_INFO("TWI enable.\n");
    NRF_LOG_FLUSH();
    nrf_drv_twi_enable(&m_twi_0);
    nrf_drv_twi_enable(&m_twi_1);
    APP_ERROR_CHECK(err_code);
}

