#include "mpr121.h"
#include <stdint.h>

#include "app_util_platform.h"
#include "app_error.h"
#include "app_timer.h"
#include "global.h"


uint8_t MPR121_read(uint8_t reg)
 {
     ret_code_t err_code;
     uint8_t reg_addr = reg;
     uint8_t reg_data_0 = 0x00;
     err_code = nrf_drv_twi_tx(&m_twi_0, MPR121_I2C_ADDR, &reg_addr, sizeof(reg_addr),true);
     err_code = nrf_drv_twi_rx(&m_twi_0, MPR121_I2C_ADDR, &reg_data_0, sizeof(reg_data_0));

     if (err_code != NRF_SUCCESS)
      {
         SEGGER_RTT_printf(0,"MPR121: cannot read from address 0x%x.\n", reg_addr);
      }

     SEGGER_RTT_printf(0,"MPR121: reg 0x%X: 0x%X\n",reg_addr,reg_data_0);
     return reg_data_0;
 }


ret_code_t  MPR121_write(uint8_t reg, uint8_t val)
 {

    ret_code_t err_code;

    uint8_t packet[2] = {reg, val};
    err_code = nrf_drv_twi_tx(&m_twi_0, MPR121_I2C_ADDR, packet, sizeof(packet),false);
    nrf_delay_ms(100);

    return err_code;

 }


uint8_t  MPR121_off(void)
 {
   uint8_t err_code;
   uint8_t packet[2];
   packet[0] = MPR121_REG_TOUCH_CTRL;
   packet[1] = MPR121_CTRL_OFF;

   err_code = nrf_drv_twi_tx(&m_twi_0, MPR121_I2C_ADDR, packet, sizeof(packet),false);
   
   return err_code;

 }


void touch_reset_timer_handler(void *p_context)
 {

   if(m_GPS_on)
    {
      SEGGER_RTT_printf(0, "touch reset: resettig touch controller...%d\n");
      MPR121_off();
      nrf_delay_us(5000);
      MPR121_on_no_baseline();
    }

 }


uint8_t  MPR121_on_no_baseline(void)
 {
   uint8_t err_code;
   uint8_t packet[2] = {MPR121_REG_SRST, MPR121_CTRL_SRST};

   //err_code = nrf_drv_twi_tx(&m_twi_0, MPR121_I2C_ADDR, packet, sizeof(packet),false);

   nrf_delay_ms(1000);

   packet[0] = MPR121_REG_TOUCH_CTRL;
   packet[1] = MPR121_CTRL_MY_PADS_ON_NO_BASELINE;

   err_code = nrf_drv_twi_tx(&m_twi_0, MPR121_I2C_ADDR, packet, sizeof(packet),false);
   
   return err_code;

 }



uint8_t MPR121_init(void)
 {
    uint8_t err_code,reg_data;
    bool detected_device;
    
    detected_device = false;
    
    err_code = nrf_drv_twi_rx(&m_twi_0, MPR121_I2C_ADDR, &reg_data, sizeof(reg_data));

    if (err_code == NRF_SUCCESS)
      {
         detected_device = true;
         SEGGER_RTT_printf(0,"MPR121 device detected at address 0x%x.\n", MPR121_I2C_ADDR);
      }

    if (!detected_device)
     {
        SEGGER_RTT_printf(0,"No MPR121 was found (%d).\n",err_code);
        NRF_LOG_FLUSH();
        return 1;
     }

   uint8_t packet[2] = {MPR121_REG_SRST, MPR121_CTRL_SRST};
   err_code = nrf_drv_twi_tx(&m_twi_0, MPR121_I2C_ADDR, packet, sizeof(packet),false);

   nrf_delay_ms(50);

//   packet[0] = MPR121_REG_TOUCH_CTRL;
//   packet[1] = MPR121_CTRL_MY_PADS_ON_5BIT_BASELINE;
//   err_code = nrf_drv_twi_tx(&m_twi_0, MPR121_I2C_ADDR, packet, sizeof(packet),false);

//   nrf_delay_ms(100);

//   if (err_code == NRF_SUCCESS)
//    {
//     SEGGER_RTT_printf(0,"MPR121 turned on.\n");
//    }

   //MPR121_write(ATO_CFG0, 0x0B);

   //MPR121_write(MHD_R, 0x01);
   //MPR121_write(NHD_R, 0x01);
   //MPR121_write(NCL_R, 0x00);
   //MPR121_write(FDL_R, 0x00);

   //MPR121_write(MHD_F, 0x01);
   //MPR121_write(NHD_F, 0x01);
   //MPR121_write(NCL_F, 0xFF);
   //MPR121_write(FDL_F, 0x02);

   //MPR121_write(FIL_CFG, 0x04);  // 0x04

   //MPR121_write(ATO_CFGU, 0xC9);
   //MPR121_write(ATO_CFGU, 0xC9); // 0xC9
   //MPR121_write(ATO_CFGL, 0x82);
   //MPR121_write(ATO_CFGT, 0xB5);

   //MPR121_write(MPR121_REG_DEBOUNCE,0xFF);

   uint8_t i;

   // set touch threshold 
   for(i = 0; i < 25; i += 2)
    {
      packet[0] = MPR121_REG_TOUCH_THRESHOLD_BASE+i; packet[1] = 0x01;  // 0x0A
      err_code = nrf_drv_twi_tx(&m_twi_0, MPR121_I2C_ADDR, packet, sizeof(packet),false);
      
      if (err_code != NRF_SUCCESS)
        SEGGER_RTT_printf(0,"MPR121: error setting touch threshold for address 0x%X.\n",MPR121_REG_TOUCH_THRESHOLD_BASE+i);
      else
        SEGGER_RTT_printf(0,"MPR121: set touch threshold 0x%X for address 0x%X.\n",packet[1],MPR121_REG_TOUCH_THRESHOLD_BASE+i);
    
    }


   // set release threshold 
   //for(i = 0; i < 25; i += 2)
   // {
   //   packet[0] = MPR121_REG_RELEASE_THRESHOLD_BASE+i; packet[1] = 0x05;  // 0x05
   //   err_code = nrf_drv_twi_tx(&m_twi_0, MPR121_I2C_ADDR, packet, sizeof(packet),false);
   //   if (err_code != NRF_SUCCESS)
   //     SEGGER_RTT_printf(0,"MPR121: error setting release threshold for address 0x%X\n",MPR121_REG_RELEASE_THRESHOLD_BASE+i);
   //   else
   //     SEGGER_RTT_printf(0,"MPR121: set release threshold 0x%X for address 0x%X.\n",packet[1],MPR121_REG_RELEASE_THRESHOLD_BASE+i);
   // }


   packet[0] = MPR121_REG_TOUCH_CTRL;
   packet[1] = MPR121_CTRL_MY_PADS_ON_5BIT_BASELINE;
   err_code = nrf_drv_twi_tx(&m_twi_0, MPR121_I2C_ADDR, packet, sizeof(packet),false);

   nrf_delay_ms(50);

   if (err_code == NRF_SUCCESS)
    {
     SEGGER_RTT_printf(0,"MPR121 turned on.\n");
    }
 
  return 0;

 }


void MPR121_check_pad_status(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
 {

     uint8_t reg_data_0;
     uint8_t reg_data_1;
     uint8_t reg_addr;
     ret_code_t err_code;
     uint16_t touch_event;

     //SEGGER_RTT_printf(0, "RTT DEBUG: in touch IRQ handler\n");

     reg_addr = 0x00;
     reg_data_0 = 0x00;
     err_code = nrf_drv_twi_tx(&m_twi_0, MPR121_I2C_ADDR, &reg_addr, sizeof(reg_addr),true);
     err_code = nrf_drv_twi_rx(&m_twi_0, MPR121_I2C_ADDR, &reg_data_0, sizeof(reg_data_0));

     SEGGER_RTT_printf(0,"MPR121: reg 0x%X: 0x%X\n",reg_addr,reg_data_0);

     reg_addr = 0x01;
     reg_data_1 = 0x00;
     err_code = nrf_drv_twi_tx(&m_twi_0, MPR121_I2C_ADDR, &reg_addr, sizeof(reg_addr),true);
     err_code = nrf_drv_twi_rx(&m_twi_0, MPR121_I2C_ADDR, &reg_data_1, sizeof(reg_data_1));
     SEGGER_RTT_printf(0,"MPR121: reg 0x%X: 0x%X (%d)\n",reg_addr,reg_data_1,err_code);

     touch_event =  ((uint16_t)reg_data_1 << 8) | reg_data_0;
 
     if(!m_touch_event_in_progress)
      {
        //SEGGER_RTT_printf(0,"MPR121: new touch event start\n");
        m_touch_event_in_progress = true;
        memset(&m_touch_event_queue,0,MAX_TOUCH_EVENTS*2); // *2 cause this is uint16_t array
        err_code = app_timer_start(m_touch_event_timer, APP_TIMER_TICKS(1000), NULL);
        app_timer_start(m_touch_event_timer, APP_TIMER_TICKS(1000), NULL);
        if(err_code > 0)
         SEGGER_RTT_printf(0,"MPR121: touch event timer error (%d)(%X)(%X)\n",err_code,m_touch_event_timer,touch_event_timer_handler);
        APP_ERROR_CHECK(err_code); 
      }
     
     if(m_touch_event_queue_idx < MAX_TOUCH_EVENTS)
        m_touch_event_queue[m_touch_event_queue_idx++] = touch_event;

     NRF_LOG_FLUSH();

 }
 
 
 
