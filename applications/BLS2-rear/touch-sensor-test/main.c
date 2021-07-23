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


#define UART_TX_BUF_SIZE 256
#define UART_RX_BUF_SIZE 1024

uint8_t m_UART_input_buffer[UART_RX_BUF_SIZE];
char m_UART_new_data[256];  // single UART input command max length
uint16_t m_UART_buffer_pos;
uint8_t m_GPS_module_state;
uint8_t m_UART_wiring;
bool m_receiving_UBX;
uint8_t m_received_UBX_packet_len;

/** @brief State transition events for the app_uart state machine. */
typedef enum
{
    ON_CTS_HIGH,   /**< Event: CTS gone high. */
    ON_CTS_LOW,    /**< Event: CTS gone low. */
    ON_UART_PUT,   /**< Event: Application wants to transmit data. */
    ON_TX_READY,   /**< Event: Data has been transmitted on the uart and line is available. */
    ON_UART_CLOSE, /**< Event: The UART module are being stopped. */
} app_uart_state_event_t;

typedef enum
{
    UART_OFF,        /**< app_uart state OFF, indicating CTS is low. */
    UART_READY,      /**< app_uart state ON, indicating CTS is high. */
    UART_ON,         /**< app_uart state TX, indicating UART is ongoing transmitting data. */
    UART_WAIT_CLOSE, /**< app_uart state WAIT CLOSE, indicating that CTS is low, but a byte is currently being transmitted on the line. */
} app_uart_states_t;

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

   SEGGER_RTT_printf(0, "RTT DEBUG: UART inited\n");

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



void UART0_IRQHandler(void)
   {
     uint8_t rx_byte;

    SEGGER_RTT_printf(0, "RTT DEBUG: in UART IRQ handler\n");
    NRF_LOG_FLUSH();

    // Handle reception
     if ((NRF_UART0->EVENTS_RXDRDY != 0) && (NRF_UART0->INTENSET & UART_INTENSET_RXDRDY_Msk))
      {


       //nrf_gpio_pin_write(PIN_BRD_LED, 1);  // flash onboard led on incoming UART data
       //nrf_delay_us(90);
       //nrf_gpio_pin_write(PIN_BRD_LED, 0);

       //app_uart_evt_t app_uart_event;

       // Clear UART RX event flag
       NRF_UART0->EVENTS_RXDRDY  = 0;
       rx_byte                 = (uint8_t)NRF_UART0->RXD;

       //app_uart_event.evt_type   = APP_UART_DATA;
       //app_uart_event.data.value = m_rx_byte;
       //m_event_handler(&app_uart_event);

       m_UART_input_buffer[m_UART_buffer_pos] = rx_byte;

       if((m_UART_buffer_pos == 0) && (rx_byte == 0xB5)) // receiving UBX packet
        {
         m_receiving_UBX = true;
         m_received_UBX_packet_len = 0;
        }
       else if((m_UART_buffer_pos == 0) && (rx_byte != 0xB5))
        {
         m_receiving_UBX = false;
         m_received_UBX_packet_len = 0;
        }

       if(m_receiving_UBX && (m_UART_buffer_pos == 4))
        m_received_UBX_packet_len = m_UART_input_buffer[m_UART_buffer_pos];
       if(m_receiving_UBX && (m_UART_buffer_pos == 5))
        m_received_UBX_packet_len |= m_UART_input_buffer[m_UART_buffer_pos] << 8;

       m_UART_buffer_pos++;

       if(m_UART_buffer_pos > (UART_RX_BUF_SIZE - 1))
         m_UART_buffer_pos = 0;  // overflow, wrap buffer


       if((m_UART_input_buffer[m_UART_buffer_pos - 1] == '\n') && (!m_receiving_UBX))  // new line of data from UART
        {
          memcpy(&m_UART_new_data,&m_UART_input_buffer,m_UART_buffer_pos);
          m_UART_new_data[m_UART_buffer_pos - 1] = 0x0;
          parse_UART_input((char *)&m_UART_new_data);
          m_UART_buffer_pos = 0;
        }
       else if(m_receiving_UBX && (m_UART_buffer_pos == (m_received_UBX_packet_len + 8)))
       {
         memcpy(&m_UART_new_data,&m_UART_input_buffer,m_UART_buffer_pos);
         parse_UART_input((char *)&m_UART_new_data);
         m_UART_buffer_pos = 0;
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
#define MPR121_REG_SRST 0x80
#define MPR121_REG_DEBOUNCE 0x5B

#define MPR121_CTRL_ALL_PADS_ON_10BIT_BASELINE 207
#define MPR121_CTRL_ALL_PADS_ON_5BIT_BASELINE 143
#define MPR121_CTRL_ALL_PADS_ON_NO_BASELINE 79
#define MPR121_CTRL_SRST 0x63

#define MHD_R	0x2B
#define NHD_R	0x2C
#define	NCL_R 	0x2D
#define	FDL_R	0x2E
#define	MHD_F	0x2F
#define	NHD_F	0x30
#define	NCL_F	0x31
#define	FDL_F	0x32
#define	ELE0_T	0x41
#define	ELE0_R	0x42
#define	ELE1_T	0x43
#define	ELE1_R	0x44
#define	ELE2_T	0x45
#define	ELE2_R	0x46
#define	ELE3_T	0x47
#define	ELE3_R	0x48
#define	ELE4_T	0x49
#define	ELE4_R	0x4A
#define	ELE5_T	0x4B
#define	ELE5_R	0x4C
#define	ELE6_T	0x4D
#define	ELE6_R	0x4E
#define	ELE7_T	0x4F
#define	ELE7_R	0x50
#define	ELE8_T	0x51
#define	ELE8_R	0x52
#define	ELE9_T	0x53
#define	ELE9_R	0x54
#define	ELE10_T	0x55
#define	ELE10_R	0x56
#define	ELE11_T	0x57
#define	ELE11_R	0x58
#define	FIL_CFG	0x5D
#define	ELE_CFG	0x5E
#define GPIO_CTRL0	0x73
#define	GPIO_CTRL1	0x74
#define GPIO_DATA	0x75
#define	GPIO_DIR	0x76
#define	GPIO_EN		0x77
#define	GPIO_SET	0x78
#define	GPIO_CLEAR	0x79
#define	GPIO_TOGGLE	0x7A
#define	ATO_CFG0	0x7B
#define	ATO_CFGU	0x7D
#define	ATO_CFGL	0x7E
#define	ATO_CFGT	0x7F


// Global Constants
#define TOU_THRESH	0x0F
#define	REL_THRESH	0x0A


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

ret_code_t  mpr121Write(uint8_t reg, uint8_t val)
 {

    ret_code_t err_code;
    uint8_t address;

    address = MPR121_I2C_ADDR;

    uint8_t packet[2] = {reg, val};
    err_code = nrf_drv_twi_tx(&m_twi, address, packet, sizeof(packet),false);
    nrf_delay_ms(100);

    return err_code;

 }


int main(void)
{
    ret_code_t err_code;
    uint8_t address;
    uint8_t reg_data; 
    uint8_t reg_addr;

    address = MPR121_I2C_ADDR;

    APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
    NRF_LOG_DEFAULT_BACKENDS_INIT();


   twi_init();
   //UART_config(0,PIN_GPS_TXD,0,PIN_GPS_RXD,UART_BAUDRATE_BAUDRATE_Baud38400,false);
   
  
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


   mpr121Write(MHD_R, 0x01);
   mpr121Write(NHD_R, 0x01);
   mpr121Write(NCL_R, 0x00);
   mpr121Write(FDL_R, 0x00);
   
   mpr121Write(MHD_F, 0x01);
   mpr121Write(NHD_F, 0x01);
   mpr121Write(NCL_F, 0xFF);
   mpr121Write(FDL_F, 0x02);

   mpr121Write(FIL_CFG, 0x04);

   mpr121Write(ATO_CFGU, 0xC9);	
   mpr121Write(ATO_CFGL, 0x82);	
   mpr121Write(ATO_CFGT, 0xB5);
   
   //mpr121Write(MPR121_REG_DEBOUNCE,0xFF);

 
   uint8_t i;
  
   // set touch threshold to 40 for all electrodes 
   for(i = 0; i < 25; i += 2)
    {
      packet[0] = MPR121_REG_TOUCH_THRESHOLD_BASE+i; packet[1] = 0x30;
      err_code = nrf_drv_twi_tx(&m_twi, address, packet, sizeof(packet),false);
    }

   // set release threshold to 20 for all electrodes
   for(i = 0; i < 25; i += 2)
    {
      packet[0] = MPR121_REG_RELEASE_THRESHOLD_BASE+i; packet[1] = 0x0A;
      err_code = nrf_drv_twi_tx(&m_twi, address, packet, sizeof(packet),false);
    }

  // IRQ_hook_init();

   SEGGER_RTT_printf(0,"Ready.\n");
 
   nrf_gpio_cfg_input(TOUCH_IRQ,NRF_GPIO_PIN_NOPULL);


   nrf_gpio_pin_set(PIN_GPS_ENA);
   nrf_gpio_pin_set(PIN_GPS_RST);
   nrf_delay_ms(1000);
   nrf_gpio_pin_clear(PIN_GPS_RST);
   nrf_delay_ms(1000);
   nrf_gpio_pin_set(PIN_GPS_RST);

   while(1)
    {

     reg_addr = 0x00;
     err_code = nrf_drv_twi_tx(&m_twi, 0x5A, &reg_addr, sizeof(reg_addr),true);
     err_code = nrf_drv_twi_rx(&m_twi, 0x5A, &reg_data, sizeof(reg_data));

     SEGGER_RTT_printf(0,"MPR121: reg 0x%X: 0x%X (%d)\n",reg_addr,reg_data,err_code);

     reg_addr = 0x01;
     err_code = nrf_drv_twi_tx(&m_twi, 0x5A, &reg_addr, sizeof(reg_addr),true);
     err_code = nrf_drv_twi_rx(&m_twi, 0x5A, &reg_data, sizeof(reg_data));
     SEGGER_RTT_printf(0,"MPR121: reg 0x%X: 0x%X (%d)\n",reg_addr,reg_data,err_code);

     NRF_LOG_FLUSH();

     APP_ERROR_CHECK(err_code);
     nrf_delay_ms(1000);
   }

}
/** @} */
