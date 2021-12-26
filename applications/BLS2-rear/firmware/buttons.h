#ifndef BUTTONS_H
#define BUTTONS_H

#define HW_BUTTONS 6
#define DSCR_LEN 5
#define BUTTON_DESCR_LINE_LEN 60

typedef struct _button_t
  {
    char btn_text[DSCR_LEN];
    bool press;
  } button_t;

char m_button_description[BUTTON_DESCR_LINE_LEN];

button_t m_buttons[HW_BUTTONS];

// prototypes
void init_buttons(void);
void map_buttons(char *b1, char *b2, char *b3, char *b4, char *b5, char *b6);
void zero_buttons(void);
void clear_buttons(void);

#endif

