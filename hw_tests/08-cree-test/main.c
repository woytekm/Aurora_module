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

#include "aurora_board.h"


/**
 * @brief Function for main application entry.
 */
int main(void)
{

 nrf_gpio_cfg_output(PIN_J33_08);
 nrf_gpio_cfg_output(PIN_J33_10);
 nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(0,29));
 nrf_gpio_pin_clear(PIN_J33_08);
 nrf_gpio_pin_clear(PIN_J33_10);
 nrf_gpio_pin_clear(NRF_GPIO_PIN_MAP(0,29));


 nrf_gpio_cfg_output(PIN_GPS_ENA);
 nrf_gpio_cfg_output(PIN_GPS_RST);
 nrf_gpio_cfg_output(PIN_BRD_BUZZER);
 nrf_gpio_cfg_output(PIN_BRD_LED);

 nrf_gpio_pin_clear(PIN_J33_08);
 nrf_gpio_pin_clear(PIN_J33_10);
 nrf_gpio_pin_clear(NRF_GPIO_PIN_MAP(0,29));

 nrf_gpio_pin_clear(PIN_GPS_ENA);


 for (;;)
   {

//     nrf_gpio_pin_clear(NRF_GPIO_PIN_MAP(0,29));
//     nrf_gpio_pin_set(PIN_J33_10);
//     nrf_delay_ms(5000);

     nrf_gpio_pin_clear(PIN_J33_10);
     nrf_gpio_pin_set(PIN_J33_08);
     nrf_delay_ms(5000);

//     nrf_gpio_pin_clear(PIN_J33_08);
//     nrf_gpio_pin_set(NRF_GPIO_PIN_MAP(0,29));
//     nrf_delay_ms(5000);

   }

}
/** @} */
