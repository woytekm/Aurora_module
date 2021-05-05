/**
 * Copyright (c) 2014 - 2019, Nordic Semiconductor ASA
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
 *
 * @defgroup blinky_example_main main.c
 * @{
 * @ingroup blinky_example
 * @brief Blinky Example Application main file.
 *
 * This file contains the source code for a sample application to blink LEDs.
 *
 */

#include <stdbool.h>
#include <stdint.h>
//#include <nrf_drv_pwm.h>
#include "nrf_delay.h"
//#include "nrf_drv_pwm.h"
//#include "app_util_platform.h"
//#include "app_error.h"
//#include "boards.h"
//#include "bsp.h"
//#include "app_timer.h"
//#include "nrf_drv_clock.h"
//#include "nrf_gpiote.h"
#include "nrf_drv_gpiote.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "SEGGER_RTT.h"


//#define BUZZER NRF_GPIO_PIN_MAP(1,06)
#define BUZZER 38

//#define BUTTON0 NRF_GPIO_PIN_MAP(1,11)

//#define GPIOTE_CHANNEL_0 0

volatile uint8_t blink;


/**
 * @brief Function for application main entry.
 */
int main(void)
{

 blink = 1;

 nrf_gpio_cfg_output(BUZZER);

// nrf_drv_gpiote_init();

 SEGGER_RTT_printf(0, "app start segger.\n");
  //NRF_LOG_INFO("app start.");
  //demo2();

//void in_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action){
//          blink ? (blink = 0) : (blink = 1);
//          SEGGER_RTT_printf(0, "GPIOTE pin handler\n");
//
//
//nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
//in_config.pull = NRF_GPIO_PIN_NOPULL;
//nrf_drv_gpiote_in_init(BUTTON0, &in_config, in_pin_handler);
//nrf_drv_gpiote_in_event_enable(BUTTON0, true);

//}

while(1)
 {

    //nrf_gpio_pin_set(BUZZER);     
    nrf_gpio_pin_write(BUZZER, 1);
    nrf_delay_us(50);
    //nrf_gpio_pin_clear(BUZZER);
    nrf_gpio_pin_write(BUZZER, 0);
    nrf_delay_us(50);

//  while(!blink)
//   {}

  }

}

/**
 *@}
 **/
