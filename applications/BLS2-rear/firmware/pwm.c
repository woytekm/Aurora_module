#include "global.h"
#include "nrf_drv_pwm.h"


static nrf_drv_pwm_t m_pwm0 = NRF_DRV_PWM_INSTANCE(0);

#define USED_PWM(idx) (1UL << idx)
static uint8_t m_used = 0;


void light_start(uint8_t program, uint8_t speed, uint8_t brightness)
 {

  // period (in Hz) = clock / top

  if(m_light_on == false)
   {
         nrf_drv_pwm_config_t const config0 =
    {
        .output_pins =
        {
            PIN_J33_08,                           // channel 0 = LED1
            PIN_J33_10,                           // channel 1 = LED3
            NRF_GPIO_PIN_MAP(0,29),               // channel 2 = LED2 - there is an error in aurora_board.h 
            NRF_DRV_PWM_PIN_NOT_USED,             // channel 3
        },
        .irq_priority = APP_IRQ_PRIORITY_LOWEST,
        .base_clock   = NRF_PWM_CLK_125kHz,
        .count_mode   = NRF_PWM_MODE_UP,
        .top_value    = m_led_program_duty,
        .load_mode    = NRF_PWM_LOAD_COMMON,
        .step_mode    = NRF_PWM_STEP_AUTO
    };

    APP_ERROR_CHECK(nrf_drv_pwm_init(&m_pwm0, &config0, NULL));
    m_used |= USED_PWM(0);

    // This array cannot be allocated on stack (hence "static") and it must
    // be in RAM (hence no "const", though its content is not changed).
    static uint16_t /*const*/ seq_values[] =
                 {
                  0,
                  0x8000,
                 };

    nrf_pwm_sequence_t const seq =
    {
        .values.p_common = seq_values,
        .length          = NRF_PWM_VALUES_LENGTH(seq_values),
        .repeats         = 0,
        .end_delay       = 0
    };

    m_light_on = true;

    (void)nrf_drv_pwm_simple_playback(&m_pwm0, &seq, 3, NRF_DRV_PWM_FLAG_LOOP);     

   }
 }

void light_stop(void)
 {
  if(m_light_on == true)
   {

    if (m_used & USED_PWM(0))
      {
        nrf_drv_pwm_uninit(&m_pwm0);
      }
    m_light_on = false;
   }
 }



