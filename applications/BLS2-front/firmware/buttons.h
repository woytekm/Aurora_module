#ifndef BUTTONS_H
#define BUTTONS_H

#define HW_BUTTONS 3

#define PIN_BTN1 NRF_GPIO_PIN_MAP(0,8)
#define PIN_BTN2 NRF_GPIO_PIN_MAP(0,20)
#define PIN_BTN3 NRF_GPIO_PIN_MAP(0,10)

typedef struct _button_t
  {
    bool press;
  } button_t;

button_t m_buttons[HW_BUTTONS];

// prototypes
void init_buttons(void);
void zero_buttons(void);
void clear_buttons(void);
void button_debounce_timer_handler(void *p_context);
#endif

