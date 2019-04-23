#ifndef BUTTONS_H
#define BUTTONS_H

#define TIME_COMP 100

#ifdef TIMER
zb_uint8_t timer_count;
zb_uint8_t timer_firstb;
zb_uint8_t timer_secb;
#endif

#ifdef ZB_ALARMS
zb_uint8_t first_button;
zb_uint8_t second_button;
#endif

void init_buttons(void);

void __attribute__((weak)) button_first_click(zb_uint8_t param) ZB_CALLBACK;
void __attribute__((weak)) button_second_click(zb_uint8_t param) ZB_CALLBACK;
void __attribute__((weak)) button_both_click(zb_uint8_t param) ZB_CALLBACK;

#endif /* BUTTONS_H */
