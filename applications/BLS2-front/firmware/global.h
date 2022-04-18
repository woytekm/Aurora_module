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

#include "nrf_drv_ppi.h"
#include "nrf_drv_timer.h"
#include "nrf_drv_saadc.h"

#include "nrf_drv_rng.h"

#include "app_util_platform.h"
#include "bsp.h"

#include <stdlib.h>
#include <time.h>
#include <strings.h>

#include "buttons.h"
#include "aurora_board.h"
#include "lis3dh_driver.h"
#include "lis3dh_defines.h"
#include "init.h"
#include "hwconfig.h"
#include "touch.h"
#include "adc.h"

#include "app_error.h"
#include "app_timer.h"


/* TWI instance ID. */
#define TWI_INSTANCE_ID_0     0
#define TWI_INSTANCE_ID_1     1

 /* Number of possible TWI addresses. */
#define TWI_ADDRESSES      127

/* TWI instance. */
static const nrf_drv_twi_t m_twi_0 = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID_0);
static const nrf_drv_twi_t m_twi_1 = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID_1);

#define USB_INIT_WAIT 20

#define TOUCH_IRQ_PIN NRF_GPIO_PIN_MAP(1,10)

#define USER_LED_3 NRF_GPIO_PIN_MAP(0,3)
#define USER_LED_2 NRF_GPIO_PIN_MAP(0,28)
#define USER_LED_1 NRF_GPIO_PIN_MAP(1,13)


app_timer_t *m_touch_event_timer;
app_timer_t *m_touch_reset_timer;
app_timer_t *m_button_debounce_timer;
app_timer_t *m_headlight_flash_timer;

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

volatile uint8_t m_battery_level;

volatile uint8_t m_SPI_mutex;

uint8_t m_led_step;
uint8_t m_led_steps[3];
bool led_1_blink;
bool led_2_blink;
bool led_3_blink;
uint8_t m_blink_delay;

enum LIGHT_MODE { LIGHT_CONSTANT, LIGHT_FLASHING };

uint8_t m_headlight_duty_cycle;
uint8_t m_light_mode;

uint16_t m_shock_val;
int16_t m_X_prev,m_Y_prev,m_Z_prev;
int16_t m_X_factor,m_Y_factor,m_Z_factor;

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
void light_start();
void light_stop(void);
void pwm_init(void);
void pwm_update_duty_cycle(uint8_t duty_cycle);
void switch_light_mode(uint8_t light_mode);
void headlight_flash_handler(void *p_context);

#endif
