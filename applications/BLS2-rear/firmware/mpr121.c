
uint8_t MPR121_init(void)
 {
    address = MPR121_I2C_ADDR;

    err_code = nrf_drv_twi_rx(&m_twi_0, address, &reg_data, sizeof(reg_data));

    if (err_code == NRF_SUCCESS)
      {
         detected_device = true;
         SEGGER_RTT_printf(0,"MPR121 device detected at address 0x%x.\n", address);
      }

    if (!detected_device)
     {
        SEGGER_RTT_printf(0,"No MPR121 was found.\n");
        NRF_LOG_FLUSH();
        return 0;
     }

   uint8_t packet[2] = {MPR121_REG_SRST, MPR121_CTRL_SRST};

   err_code = nrf_drv_twi_tx(&m_twi, address, packet, sizeof(packet),false);

   nrf_delay_ms(1000);

   packet[0] = MPR121_REG_TOUCH_CTRL;
   packet[1] = MPR121_CTRL_ALL_PADS_ON_5BIT_BASELINE;

   err_code = nrf_drv_twi_tx(&m_twi, address, packet, sizeof(packet),false);

   if (err_code == NRF_SUCCESS)
    {
     SEGGER_RTT_printf(0,"MPR121 turned on.\n", address);
    }


   MPR121_write(MHD_R, 0x01);
   MPR121_write(NHD_R, 0x01);
   MPR121_write(NCL_R, 0x00);
   MPR121_write(FDL_R, 0x00);

   MPR121_write(MHD_F, 0x01);
   MPR121_write(NHD_F, 0x01);
   MPR121_write(NCL_F, 0xFF);
   MPR121_write(FDL_F, 0x02);

   MPR121_write(FIL_CFG, 0x04);

   MPR121_write(ATO_CFGU, 0xC9);
   MPR121_write(ATO_CFGL, 0x82);
   MPR121_write(ATO_CFGT, 0xB5);

   MPR121_write(MPR121_REG_DEBOUNCE,0xFF);

   uint8_t i;

   // set touch threshold to 40 for all electrodes
   for(i = 0; i < 25; i += 2)
    {
      packet[0] = MPR121_REG_TOUCH_THRESHOLD_BASE+i; packet[1] = 0x0F;
      err_code = nrf_drv_twi_tx(&m_twi, address, packet, sizeof(packet),false);
    }

   // set release threshold to 20 for all electrodes
   for(i = 0; i < 25; i += 2)
    {
      packet[0] = MPR121_REG_RELEASE_THRESHOLD_BASE+i; packet[1] = 0x0A;
      err_code = nrf_drv_twi_tx(&m_twi, address, packet, sizeof(packet),false);
    }


 }



ret_code_t MPR121_check_pad_status(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
 {

     uint8_t reg_data;
     uint8_t reg_addr;
     ret_code_t err_code;

     reg_addr = 0x00;
     err_code = nrf_drv_twi_tx(&m_twi, 0x5A, &reg_addr, sizeof(reg_addr),true);
     err_code = nrf_drv_twi_rx(&m_twi, 0x5A, &reg_data, sizeof(reg_data));

     SEGGER_RTT_printf(0,"MPR121: reg 0x%X: 0x%X\n",reg_addr,reg_data);

     reg_addr = 0x01;
     err_code = nrf_drv_twi_tx(&m_twi, 0x5A, &reg_addr, sizeof(reg_addr),true);
     err_code = nrf_drv_twi_rx(&m_twi, 0x5A, &reg_data, sizeof(reg_data));
     SEGGER_RTT_printf(0,"MPR121: reg 0x%X: 0x%X\n",reg_addr,reg_data);

     NRF_LOG_FLUSH();

     return err_code;

 }
 
 
ret_code_t  MPR121_write(uint8_t reg, uint8_t val)
 {

    ret_code_t err_code;
    uint8_t address;

    address = MPR121_I2C_ADDR;

    uint8_t packet[2] = {reg, val};
    err_code = nrf_drv_twi_tx(&m_twi, address, packet, sizeof(packet),false);
    nrf_delay_ms(100);

    return err_code;

 }
 
 