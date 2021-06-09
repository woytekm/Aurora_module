#include "global.h"

#define UART_TX_BUF_SIZE 256
#define UART_RX_BUF_SIZE 1024

uint8_t m_UART_input_buffer[UART_RX_BUF_SIZE];
char m_UART_new_data[256];  // single UART input command max length
uint16_t m_UART_buffer_pos;
uint8_t m_GPS_module_state;
uint8_t m_UART_wiring;
bool m_receiving_UBX;
uint8_t m_received_UBX_packet_len;

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

    //SEGGER_RTT_printf(0, "RTT DEBUG: in UART IRQ handler\n");
 
    // Handle reception
     if ((NRF_UART0->EVENTS_RXDRDY != 0) && (NRF_UART0->INTENSET & UART_INTENSET_RXDRDY_Msk))
      {
    
     
       nrf_gpio_pin_write(PIN_BRD_LED, 1);  // flash onboard led on incoming UART data
       nrf_delay_us(90);
       nrf_gpio_pin_write(PIN_BRD_LED, 0);
 
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


void GPS_send_stream(char *stream, uint16_t length)
 {
   uint16_t i;
   for(i = 0; i < length; i++)
    {
     app_uart_put(stream[i]);
    }
 }

void GPS_enable(void)
 {
   nrf_gpio_pin_set(PIN_GPS_ENA); 
   nrf_gpio_pin_set(PIN_GPS_RST);
   nrf_delay_ms(1000);
   nrf_gpio_pin_clear(PIN_GPS_RST);
   nrf_delay_ms(1000);
   nrf_gpio_pin_set(PIN_GPS_RST);
   m_GPS_on = true;
 }


void GPS_disable(void)
 {
   nrf_gpio_pin_clear(PIN_GPS_ENA);
   m_GPS_on = false;
 }
