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

/* TWI instance ID. */
#define TWI_INSTANCE_ID_0     0
#define TWI_INSTANCE_ID_1     1

 /* Number of possible TWI addresses. */
#define TWI_ADDRESSES      127

/* TWI instance. */
static const nrf_drv_twi_t m_twi_0 = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID_0);
static const nrf_drv_twi_t m_twi_1 = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID_1);

#define TOUCH_IRQ NRF_GPIO_PIN_MAP(0,20)

#define MAX_TOUCH_EVENTS 6

APP_TIMER_DEF(m_touch_event_timer);

uint16_t m_touch_event_queue[MAX_TOUCH_EVENTS];
uint8_t m_touch_event_queue_idx;
bool m_touch_event_in_progress;

// prototypes

void MPR121_check_pad_status(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);
void touch_IRQ_init(void);
void twi_init(void);
void touch_event_timer_handler(void *p_context);
uint8_t MPR121_init(void);
uint8_t system_init(void);

#endif
