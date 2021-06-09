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
#include "nrf_drv_clock.h"
#include "nrf_gpiote.h"
#include "nrf_drv_gpiote.h"

#include "app_util_platform.h"
#include "bsp.h"

#include <stdlib.h>

#include "aurora_board.h"


/* TWI instance ID. */
#define TWI_INSTANCE_ID_0     0
#define TWI_INSTANCE_ID_1     1

 /* Number of possible TWI addresses. */
#define TWI_ADDRESSES      127

/* TWI instance. */
static const nrf_drv_twi_t m_twi_0 = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID_0);
static const nrf_drv_twi_t m_twi_1 = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID_1);

#define TOUCH_IRQ_PIN NRF_GPIO_PIN_MAP(0,20)

#define MAX_TOUCH_EVENTS 6

app_timer_t *m_touch_event_timer;

uint16_t m_touch_event_queue[MAX_TOUCH_EVENTS];
uint8_t m_touch_event_queue_idx;
bool m_touch_event_in_progress;

bool m_light_on;

uint8_t m_led_program_speed;
uint8_t m_led_program_brightness;
uint16_t m_led_program_duty;

bool m_GPS_on;
bool m_track_on;
bool m_track_pause;

// prototypes

void MPR121_check_pad_status(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);
void touch_IRQ_init(void);
void touch_event_timer_handler(void *p_context);
void twi_init(void);
uint8_t MPR121_init(void);
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

#endif
