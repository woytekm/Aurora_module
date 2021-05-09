/**
 * Copyright (c) 2016 - 2019, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/** @file
 * @defgroup tw_scanner main.c
 * @{
 * @ingroup nrf_twi_example
 * @brief TWI Sensor Example main file.
 *
 * This file contains the source code for a sample application using TWI.
 *
 */

#include <stdio.h>
#include "boards.h"
#include "app_util_platform.h"
#include "app_error.h"
#include "nrf_drv_twi.h"


#include "SEGGER_RTT.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_delay.h"

#include "nrf_drv_gpiote.h"
#include "nrfx_gpiote.h"

#include "aurora_board.h"

/* TWI instance ID. */
#if TWI0_ENABLED
#define TWI_INSTANCE_ID     0
#elif TWI1_ENABLED
#define TWI_INSTANCE_ID     1
#endif

 /* Number of possible TWI addresses. */
 #define TWI_ADDRESSES      127

/* TWI instance. */
static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);


/**
 * @brief TWI initialization.
 */
void twi_init (void)
{
    ret_code_t err_code;

    const nrf_drv_twi_config_t twi_config = {
       .scl                = NRF_GPIO_PIN_MAP(0,8),  //NRF_GPIO_PIN_MAP(1,0) - for accelerometer (@0x1D) + pressure/temp sensor (@0x77), NRF_GPIO_PIN_MAP(0,8) - for touch sensor, when present (@0x5A)
       .sda                = NRF_GPIO_PIN_MAP(0,10), //NRF_GPIO_PIN_MAP(0,24) - for accelerometer (@0x1D) + pressure/temp sensor (@0x77), NRF_GPIO_PIN_MAP(0,10) - for touch sensor, when present (@0x5A)
       .frequency          = NRF_DRV_TWI_FREQ_100K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
       .clear_bus_init     = false
    };

    NRF_LOG_INFO("TWI init.\n");
    NRF_LOG_FLUSH();
    err_code = nrf_drv_twi_init(&m_twi, &twi_config, NULL, NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_INFO("TWI enable.\n");
    NRF_LOG_FLUSH();
    nrf_drv_twi_enable(&m_twi);
    APP_ERROR_CHECK(err_code);
}


#define TOUCH_IRQ NRF_GPIO_PIN_MAP(0,20)

/**
 * @brief Function for main application entry.
 */

#define MPR121_I2C_ADDR 0x5A
#define MPR121_REG_TOUCH_CTRL 0x5E
#define MPR121_REG_TOUCH_THRESHOLD_BASE 0x41
#define MPR121_REG_RELEASE_THRESHOLD_BASE 0x42
#define MPR121_REG_TOUCH_STATUS_BANK_0 0x0
#define MPR121_REG_TOUCH_STATUS_BANK_1 0x1

#define MPR121_CTRL_ALL_PADS_ON_FULL_BASELINE 207


void MPR121_check_pad_status(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
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

     APP_ERROR_CHECK(err_code);

 }

static void IRQ_hook_init(void)
 {
    ret_code_t err_code;

    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(true);;

    in_config.pull = NRF_GPIO_PIN_NOPULL;

    err_code = nrf_drv_gpiote_in_init(TOUCH_IRQ, &in_config, MPR121_check_pad_status);
    APP_ERROR_CHECK(err_code);
    
    nrf_drv_gpiote_in_event_enable(TOUCH_IRQ, true);
 }


int main(void)
{
    ret_code_t err_code;
    uint8_t address;
    uint8_t reg_data; 
    bool detected_device = false;

    APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
    NRF_LOG_DEFAULT_BACKENDS_INIT();

    SEGGER_RTT_printf(0, "RTT DEBUG: TWI scanner started.\n");
    NRF_LOG_FLUSH();
    
    twi_init();

    SEGGER_RTT_printf(0,"I2C detection is running ... \n");

    address = MPR121_I2C_ADDR;

    err_code = nrf_drv_twi_rx(&m_twi, address, &reg_data, sizeof(reg_data)); 

    if (err_code == NRF_SUCCESS)
      {
         detected_device = true;
         SEGGER_RTT_printf(0,"TWI device detected at address 0x%x.\n", address);
      }

    if (!detected_device)
    {
        SEGGER_RTT_printf(0,"No device was found.\n");
        NRF_LOG_FLUSH();
        while(1) {}
    }

   SEGGER_RTT_printf(0,"I2C detection successful.\n");

   uint8_t packet[2] = {MPR121_REG_TOUCH_CTRL, MPR121_CTRL_ALL_PADS_ON_FULL_BASELINE};

   err_code = nrf_drv_twi_tx(&m_twi, address, packet, sizeof(packet),false);

   if (err_code == NRF_SUCCESS)
    {
     SEGGER_RTT_printf(0,"MPR121 turned on.\n", address);
    }

   uint8_t i;
  
   // set touch threshold to 40 for all electrodes 
   for(i = 0; i < 25; i += 2)
    {
      packet[0] = MPR121_REG_TOUCH_THRESHOLD_BASE+i; packet[1] = 15;
      err_code = nrf_drv_twi_tx(&m_twi, address, packet, sizeof(packet),false);
    }

   // set release threshold to 20 for all electrodes
   for(i = 0; i < 25; i += 2)
    {
      packet[0] = MPR121_REG_RELEASE_THRESHOLD_BASE+i; packet[1] = 8;
      err_code = nrf_drv_twi_tx(&m_twi, address, packet, sizeof(packet),false);
    }

   IRQ_hook_init();

   SEGGER_RTT_printf(0,"Ready.\n");
 
   nrf_gpio_cfg_input(TOUCH_IRQ,NRF_GPIO_PIN_NOPULL);

   while(1) {
     __WFE();
   } 

}
/** @} */
