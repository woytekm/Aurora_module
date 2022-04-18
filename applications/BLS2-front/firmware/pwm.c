#include "global.h"
#include "nrf_drv_pwm.h"


static nrf_drv_pwm_t m_pwm0 = NRF_DRV_PWM_INSTANCE(0);

#define USED_PWM(idx) (1UL << idx)
static uint8_t m_used = 0;

nrf_pwm_values_individual_t seq_values[] = {{ 0x0,  0x0, 0x0, 0x0 }};

#define OUTPUT_PIN PIN_J33_08  // Cree T6 on Aurora extension board

nrf_pwm_sequence_t const seq =
{
    .values.p_individual = seq_values,
    .length          = NRF_PWM_VALUES_LENGTH(seq_values),
    .repeats         = 0,
    .end_delay       = 0
};

// Set duty cycle between 0 and 100%
void pwm_update_duty_cycle(uint8_t duty_cycle)
{

    // Check if value is outside of range. If so, set to 100%
    if(duty_cycle >= 100)
    {
        seq_values->channel_0 = 100 | 0x8000;
    }
    else
    {
        seq_values->channel_0 = duty_cycle | 0x8000;
    }

    nrf_drv_pwm_simple_playback(&m_pwm0, &seq, 1, NRF_DRV_PWM_FLAG_LOOP);
}


void pwm_init(void)
{
    nrf_drv_pwm_config_t const config0 =
    {
        .output_pins =
        {
            OUTPUT_PIN, // channel 0
            NRF_DRV_PWM_PIN_NOT_USED,             // channel 1
            NRF_DRV_PWM_PIN_NOT_USED,             // channel 2
            NRF_DRV_PWM_PIN_NOT_USED,             // channel 3
        },
        .irq_priority = APP_IRQ_PRIORITY_LOWEST,
        .base_clock   = NRF_PWM_CLK_1MHz,
        //.base_clock   = NRF_PWM_CLK_125kHz,
        .count_mode   = NRF_PWM_MODE_UP,
        .top_value    = 100,
        .load_mode    = NRF_PWM_LOAD_INDIVIDUAL,
        .step_mode    = NRF_PWM_STEP_AUTO
    };
    // Init PWM without error handler
    APP_ERROR_CHECK(nrf_drv_pwm_init(&m_pwm0, &config0, NULL));

}


void light_start()
 {
  pwm_update_duty_cycle(m_headlight_duty_cycle);
  m_light_on = true;
 }

void light_stop(void)
 {
  nrf_drv_pwm_stop(&m_pwm0,false);
  m_light_on = false;
 }

void switch_light_mode(uint8_t light_mode)
 {
   m_light_mode = light_mode;
 }

void headlight_flash_handler(void *p_context)
 {
  if(m_light_on)
   {
    if(m_light_mode == LIGHT_FLASHING)
     {
      nrf_gpio_pin_set(USER_LED_1);
      pwm_update_duty_cycle(m_headlight_duty_cycle);
      nrf_delay_ms(50);
      pwm_update_duty_cycle(0);
      nrf_gpio_pin_clear(USER_LED_1);
     }
    else
     pwm_update_duty_cycle(m_headlight_duty_cycle);
   }
 }



