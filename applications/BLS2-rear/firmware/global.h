#ifndef GLOBAL_H
#define GLOBAL_H

#include "nrf_drv_twi.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "SEGGER_RTT.h"

#include "nrf_drv_gpiote.h"

#include "nrf_delay.h"

#include "boards.h"

#include "app_timer.h"
#include "app_scheduler.h"
#include "nrf_drv_clock.h"
#include "nrf_gpiote.h"
#include "nrf_drv_gpiote.h"

#include "nrf_atomic.h"
#include "nrf_drv_power.h"

#include "app_util_platform.h"
#include "bsp.h"

#include "ubxmessage.h"

#include <stdlib.h>
#include <time.h>

#include "buttons.h"
#include "aurora_board.h"
#include "lis3dh_driver.h"
#include "lis3dh_defines.h"

#include "ff.h"
#include "diskio_blkdev.h"

#include "app_usbd.h"
#include "app_usbd_core.h"
#include "app_usbd_string_desc.h"
#include "app_usbd_msc.h"
#include "app_error.h"
#include "app_timer.h"

#include "nrf_block_dev.h"
#include "nrf_block_dev_qspi.h"
#include "nrf_drv_usbd.h"



/* TWI instance ID. */
#define TWI_INSTANCE_ID_0     0
#define TWI_INSTANCE_ID_1     1

 /* Number of possible TWI addresses. */
#define TWI_ADDRESSES      127

/* TWI instance. */
static const nrf_drv_twi_t m_twi_0 = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID_0);
static const nrf_drv_twi_t m_twi_1 = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID_1);


#define TOUCH_IRQ_PIN NRF_GPIO_PIN_MAP(1,10)

#define USER_LED_3 NRF_GPIO_PIN_MAP(0,3)
#define USER_LED_2 NRF_GPIO_PIN_MAP(0,28)
#define USER_LED_1 NRF_GPIO_PIN_MAP(1,13)

#define MAX_TOUCH_EVENTS 4

app_timer_t *m_touch_event_timer;
app_timer_t *m_touch_reset_timer;
app_timer_t *m_button_debounce_timer;

uint16_t m_touch_event_queue[MAX_TOUCH_EVENTS];
uint8_t m_touch_event_queue_idx;
bool m_touch_event_in_progress;

bool m_button_debounce_active;
bool m_light_on;

#define LED_PGMS 3  // coded in pwm.c

uint8_t m_led_program;
uint8_t m_led_program_speed;
uint8_t m_led_program_brightness;
uint16_t m_led_program_duty;

uint8_t G_fixes;
uint8_t G_GPS_opMode;
uint8_t G_GPS_navMode;
uint8_t G_date_synced;
uint8_t G_time_synced;
uint8_t G_GPS_day;
uint8_t G_GPS_month;
uint8_t G_GPS_year;
uint8_t G_last_UBX_ACK_state;
bool G_GPS_cmd_sent;
UBXMsgBuffer G_last_received_UBX_msg;
enum UBX_ACK_STATE { UBX_ACK, UBX_NAK, UBX_UNKNOWN };

uint8_t G_current_speed;

#define NMEA_TIME_FORMAT        "%H%M%S"
#define NMEA_TIME_FORMAT_LEN    6

#define NMEA_DATE_FORMAT        "%d%m%y"
#define NMEA_DATE_FORMAT_LEN    6

 typedef struct {
           double minutes;
           int degrees;
  } degmin_position_t;

  typedef struct {
          struct tm time;
          degmin_position_t m_longitude;
          degmin_position_t m_latitude;
          char nmea_longitude[12];
          char nmea_latitude[12];
          double HDOP;
          int n_satellites;
          int altitude;
          char altitude_unit;
 } nmea_gpgga_t;

nmea_gpgga_t G_current_position;
nmea_gpgga_t G_prev_position;

uint8_t G_logger_buffer_idx;

char *G_current_gpx_filename;
bool G_gpx_wrote_header;
bool G_gpx_wrote_footer;

typedef struct GPS_settings{
  UBXU1_t lp_mode;
  UBXCFG_PM2Flags pm_flags;
  UBXU4_t pm_update_period;
  UBXU4_t pm_search_period;
  UBXU4_t pm_grid_offset;
  UBXU2_t pm_on_time; 
  UBXU2_t pm_min_acq_time;
 } GPS_settings_t;


bool m_GPS_on;
bool m_track_on;
bool m_track_pause;

// prototypes

void MPR121_check_pad_status(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);
void touch_IRQ_init(void);
void touch_event_timer_handler(void *p_context);
void touch_reset_timer_handler(void *p_context);
void twi_init(void);
uint8_t MPR121_init(void);
uint8_t MPR121_off(void);
uint8_t MPR121_on_no_baseline(void);
uint8_t system_init(void);
void light_start(uint8_t program, uint8_t speed, uint8_t brightness);
void light_stop(void);

void GPS_enable(void);
void GPS_disable(void);

void UART_config( uint8_t rts_pin_number,
                          uint8_t txd_pin_number,
                          uint8_t cts_pin_number,
                          uint8_t rxd_pin_number,
                          uint32_t speed,
                          bool hwfc);

void parse_GPS_input(char *input_string);
   
#endif
