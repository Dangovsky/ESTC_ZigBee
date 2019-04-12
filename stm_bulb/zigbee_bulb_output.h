#ifndef ZIGBEE_BULB_OUTPUT_H
#define ZIGBEE_BULB_OUTPUT_H

#include "zigbee_bulb.h"

void send_payloaded_command(zb_uint8_t param, zb_uint8_t payload_length, zb_uint8_t* payload);

void bulb_send_on_command(zb_uint8_t param) ZB_CALLBACK;
void bulb_send_off_command(zb_uint8_t param) ZB_CALLBACK;
void bulb_send_toggle_command(zb_uint8_t param) ZB_CALLBACK;
void bulb_send_brightness_up_command(zb_uint8_t param) ZB_CALLBACK;
void bulb_send_brightness_down_command(zb_uint8_t param) ZB_CALLBACK;
void bulb_send_brightness_command(zb_uint8_t param) ZB_CALLBACK;
void bulb_send_color_command(zb_uint8_t param) ZB_CALLBACK;

#endif /*ZIGBEE_BULB_H*/
