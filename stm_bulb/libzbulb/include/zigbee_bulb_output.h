#ifndef ZIGBEE_BULB_OUTPUT_H
#define ZIGBEE_BULB_OUTPUT_H

#include "zigbee_bulb.h"

#define TIM2_PERIOD 10
#define TIM2_PRESCALER 0xffff
#define TIME_COMP 100

/* timer for buttons*/
static uint32_t timer_count;
static uint32_t timer_firstb;
static uint32_t timer_secb;

void init_perif(void);

void send(zb_callback_t send_command);

void bulb_send_on_command(zb_uint8_t param) ZB_CALLBACK;
void bulb_send_off_command(zb_uint8_t param) ZB_CALLBACK;
void bulb_send_toggle_command(zb_uint8_t param) ZB_CALLBACK;
void bulb_send_brightness_up_command(zb_uint8_t param) ZB_CALLBACK;
void bulb_send_brightness_down_command(zb_uint8_t param) ZB_CALLBACK;
void bulb_send_brightness_command(zb_uint8_t param) ZB_CALLBACK;
void bulb_send_color_command(zb_uint8_t param) ZB_CALLBACK;

#endif /*ZIGBEE_BULB_H*/
