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
#include <nrf_drv_pwm.h>
#include "nrf_delay.h"
#include "nrf_drv_pwm.h"
#include "app_util_platform.h"
#include "app_error.h"
#include "boards.h"
#include "bsp.h"
#include "app_timer.h"
#include "nrf_drv_clock.h"
#include "nrf_gpiote.h"
#include "nrf_drv_gpiote.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "SEGGER_RTT.h"


#define GPS_POWER_PIN NRF_GPIO_PIN_MAP(0, 30)
#define GPS_RESET_PIN NRF_GPIO_PIN_MAP(0, 5)
#define GPS_RX NRF_GPIO_PIN_MAP(0,06)
#define GPS_TX NRF_GPIO_PIN_MAP(0,26)

#define UART_TX_BUF_SIZE 256
#define UART_RX_BUF_SIZE 1024

uint8_t G_UART_input_buffer[UART_RX_BUF_SIZE];
char G_UART_new_data[256];  // single UART input command max length
uint16_t G_UART_buffer_pos;
uint8_t G_GPS_module_state;
uint8_t G_UART_wiring;
bool G_receiving_UBX;
uint8_t G_received_UBX_packet_len;

typedef enum
{
    UART_OFF,        /**< app_uart state OFF, indicating CTS is low. */
    UART_READY,      /**< app_uart state ON, indicating CTS is high. */
    UART_ON,         /**< app_uart state TX, indicating UART is ongoing transmitting data. */
    UART_WAIT_CLOSE, /**< app_uart state WAIT CLOSE, indicating that CTS is low, but a byte is currently being transmitted on the line. */
} app_uart_states_t;

/** @brief State transition events for the app_uart state machine. */
typedef enum
{
    ON_CTS_HIGH,   /**< Event: CTS gone high. */
    ON_CTS_LOW,    /**< Event: CTS gone low. */
    ON_UART_PUT,   /**< Event: Application wants to transmit data. */
    ON_TX_READY,   /**< Event: Data has been transmitted on the uart and line is available. */
    ON_UART_CLOSE, /**< Event: The UART module are being stopped. */
} app_uart_state_event_t;

static uint8_t  m_tx_byte;                /**< TX Byte placeholder for next byte to transmit. */

static volatile app_uart_states_t m_current_state = UART_OFF; /**< State of the state machine. */

void parse_UART_input(char *line_data)
 {


  if(line_data[0] != 0xB5)
    SEGGER_RTT_printf(0, "RTT DEBUG: UART input: %s\n",line_data);  // print it if we can assume that it's a text line 
  //uint16_t i = 0;
  //while(line_data[i] != 0x00)
  // SEGGER_RTT_printf(0,"%0X ",line_data[i++]);

  //SEGGER_RTT_printf(0,"\n\n");
 }

 void UART_config(  uint8_t rts_pin_number,
                           uint8_t txd_pin_number,
                           uint8_t cts_pin_number,
                           uint8_t rxd_pin_number,
                           uint32_t speed,
                           bool hwfc)
 {
   nrf_gpio_cfg_output(txd_pin_number);
   nrf_gpio_cfg_input(rxd_pin_number, NRF_GPIO_PIN_PULLUP);

   NRF_UART0->PSELTXD = txd_pin_number;
   NRF_UART0->PSELRXD = rxd_pin_number;

   if (hwfc)
   {
     nrf_gpio_cfg_output(rts_pin_number);
     nrf_gpio_cfg_input(cts_pin_number, NRF_GPIO_PIN_NOPULL);
     NRF_UART0->PSELCTS = cts_pin_number;
     NRF_UART0->PSELRTS = rts_pin_number;
     NRF_UART0->CONFIG  = (UART_CONFIG_HWFC_Enabled << UART_CONFIG_HWFC_Pos);
   }

   NRF_UART0->BAUDRATE         = (speed << UART_BAUDRATE_BAUDRATE_Pos);
   NRF_UART0->CONFIG           = (UART_CONFIG_PARITY_Excluded << UART_CONFIG_PARITY_Pos);
   NRF_UART0->ENABLE           = (UART_ENABLE_ENABLE_Enabled << UART_ENABLE_ENABLE_Pos);
   NRF_UART0->TASKS_STARTTX    = 1;
   NRF_UART0->TASKS_STARTRX    = 1;
   NRF_UART0->EVENTS_RXDRDY    = 0;

   m_current_state         = UART_READY; 

   NRF_UART0->INTENCLR = 0xffffffffUL;
   NRF_UART0->INTENSET = (UART_INTENSET_RXDRDY_Set << UART_INTENSET_RXDRDY_Pos) |
                         (UART_INTENSET_TXDRDY_Set << UART_INTENSET_TXDRDY_Pos) |
                         (UART_INTENSET_ERROR_Set << UART_INTENSET_ERROR_Pos);

   NVIC_ClearPendingIRQ(UART0_IRQn);
   NVIC_SetPriority(UART0_IRQn, 1);
   NVIC_EnableIRQ(UART0_IRQn);

 }

static void action_tx_send()
{
    if (m_current_state != UART_ON)
    {
        // Start the UART.
        NRF_UART0->TASKS_STARTTX = 1;
    }

    //SEGGER_RTT_printf(0, "RTT DEBUG: serial out: %X.\n",m_tx_byte);

    NRF_UART0->TXD  = m_tx_byte;
    m_current_state = UART_ON;
}

void action_tx_stop()
{
    //app_uart_evt_t app_uart_event;

    // No more bytes in FIFO, terminate transmission.
    NRF_UART0->TASKS_STOPTX = 1;
    m_current_state         = UART_READY;
    // Last byte from FIFO transmitted, notify the application.
    // Notify that new data is available if this was first byte put in the buffer.
    //app_uart_event.evt_type = APP_UART_TX_EMPTY;
}

static void action_tx_ready()
 {
     action_tx_stop();
 }

 static void on_uart_put(void)
 {
     if (m_current_state == UART_READY)
     {
         action_tx_send();
     }
 }

static void on_tx_ready(void)
 {
     switch (m_current_state)
     {
         case UART_ON:
         case UART_READY:
             action_tx_ready();
             break;

         default:
             // Nothing to do.
             break;
     }
 }


static void on_uart_event(app_uart_state_event_t event)
 {
     switch (event)
     {

         case ON_TX_READY:
             on_tx_ready();
             break;

         case ON_UART_PUT:
             on_uart_put();
             break;

         default:
             // All valid events are handled above.
             break;
     }
 }

uint32_t app_uart_put(uint8_t byte)
  {
     uint32_t err_code = NRF_SUCCESS;
     uint16_t delay_counter = 0;

     while(m_current_state != UART_READY)
      {
       nrf_delay_ms(1);
       delay_counter++;
       if(delay_counter > 300)
        return 4;
      }

     m_tx_byte = byte;
     on_uart_event(ON_UART_PUT);

     //SEGGER_RTT_printf(0, "RTT DEBUG: app_uart_put: %X, err: err_code: %d\n", byte, err_code);

     return err_code;
  }

 void UART0_IRQHandler(void)
   {
     uint8_t rx_byte;
 
    // Handle reception
     if ((NRF_UART0->EVENTS_RXDRDY != 0) && (NRF_UART0->INTENSET & UART_INTENSET_RXDRDY_Msk))
      {
    
      
       //app_uart_evt_t app_uart_event;

       // Clear UART RX event flag
       NRF_UART0->EVENTS_RXDRDY  = 0;
       rx_byte                 = (uint8_t)NRF_UART0->RXD;

       //app_uart_event.evt_type   = APP_UART_DATA;
       //app_uart_event.data.value = m_rx_byte;
       //m_event_handler(&app_uart_event);

       G_UART_input_buffer[G_UART_buffer_pos] = rx_byte;

       if((G_UART_buffer_pos == 0) && (rx_byte == 0xB5)) // receiving UBX packet
        {
         G_receiving_UBX = true;
         G_received_UBX_packet_len = 0;
        }
       else if((G_UART_buffer_pos == 0) && (rx_byte != 0xB5))
        {
         G_receiving_UBX = false;
         G_received_UBX_packet_len = 0;
        }

       if(G_receiving_UBX && (G_UART_buffer_pos == 4))
        G_received_UBX_packet_len = G_UART_input_buffer[G_UART_buffer_pos];
       if(G_receiving_UBX && (G_UART_buffer_pos == 5))
        G_received_UBX_packet_len |= G_UART_input_buffer[G_UART_buffer_pos] << 8;

       G_UART_buffer_pos++;

       if(G_UART_buffer_pos > (UART_RX_BUF_SIZE - 1))
         G_UART_buffer_pos = 0;  // overflow, wrap buffer

       if((G_UART_input_buffer[G_UART_buffer_pos - 1] == '\n') && (!G_receiving_UBX))  // new line of data from UART
        {
          memcpy(&G_UART_new_data,&G_UART_input_buffer,G_UART_buffer_pos);
          G_UART_new_data[G_UART_buffer_pos - 1] = 0x0;
          parse_UART_input((char *)&G_UART_new_data);
          G_UART_buffer_pos = 0;
        }
       else if(G_receiving_UBX && (G_UART_buffer_pos == (G_received_UBX_packet_len + 8)))
       {
         memcpy(&G_UART_new_data,&G_UART_input_buffer,G_UART_buffer_pos);
         parse_UART_input((char *)&G_UART_new_data);
         G_UART_buffer_pos = 0;
       }

      }

     if ((NRF_UART0->EVENTS_ERROR != 0) && (NRF_UART0->INTENSET & UART_INTENSET_ERROR_Msk))
      {
        uint32_t       error_source;
        //app_uart_evt_t app_uart_event;

        
        // Clear UART ERROR event flag.
        NRF_UART0->EVENTS_ERROR = 0;

        // Clear error source.
        error_source        = NRF_UART0->ERRORSRC;
        NRF_UART0->ERRORSRC = error_source;
        
        SEGGER_RTT_printf(0, "RTT DEBUG: serial receive error from source : %d\n",error_source);
       //app_uart_event.evt_type                 = APP_UART_COMMUNICATION_ERROR;
       //app_uart_event.data.error_communication = error_source;
       //m_event_handler(&app_uart_event);
       
      }

     // Handle transmission.
     if ((NRF_UART0->EVENTS_TXDRDY != 0) && (NRF_UART0->INTENSET & UART_INTENSET_TXDRDY_Msk))
     {
         // Clear UART TX event flag.
         NRF_UART0->EVENTS_TXDRDY = 0;
         on_uart_event(ON_TX_READY);
     }


  }


void GPS_send_stream(char *stream, uint16_t length)
 {
   uint16_t i;
   for(i = 0; i < length; i++)
    {
     app_uart_put(stream[i]);
    }
 }

static void nosd_power_manage(void)
 {
  __WFE();  
  __SEV();
  __WFE();
 }


/**
 * @brief Function for application main entry.
 */
int main(void)
{

 char gps_command[50];

 nrf_gpio_cfg_output(GPS_POWER_PIN);
 nrf_gpio_cfg_output(GPS_RESET_PIN);
 nrf_gpio_pin_set(GPS_POWER_PIN); 

 nrf_gpio_pin_set(GPS_RESET_PIN);
 nrf_delay_ms(1000);
 nrf_gpio_pin_clear(GPS_RESET_PIN);
 nrf_delay_ms(1000);
 nrf_gpio_pin_set(GPS_RESET_PIN);

 NRF_CLOCK->TASKS_HFCLKSTART = 1;
 
  while(0 == NRF_CLOCK ->EVENTS_HFCLKSTARTED)
  {
  }

 NRF_CLOCK->TASKS_LFCLKSTOP = 1;
   while(NRF_CLOCK->LFCLKSTAT);
 
 NRF_CLOCK->LFCLKSRC = CLOCK_LFCLKSRC_SRC_RC;  
 NRF_CLOCK->TASKS_LFCLKSTART = 1;
   
 UART_config(0,GPS_TX,0,GPS_RX,UART_BAUDRATE_BAUDRATE_Baud38400,false);

 nrf_delay_ms(10000);

 sprintf(gps_command,"$PMTK101*32\r\n");
 GPS_send_stream(gps_command,strlen(gps_command)); 

 SEGGER_RTT_printf(0, "RTT DEBUG: UART configured.\n");

 while(true)
  {
    nosd_power_manage();
  }



}

/**
 *@}
 **/
