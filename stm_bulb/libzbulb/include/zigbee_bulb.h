#ifndef ZIGBEE_BULB_H 
#define ZIGBEE_BULB_H 

typedef enum commands_e
{
    ON_COMMAND = 0,
    OFF_COMMAND,
    TOGGLE_COMMAND,
    BRIGHTNESS_UP_COMMAND,
    BRIGHTNESS_DOWN_COMMAND,
    BRIGHTNESS_COMMAND,
    COLOR_COMMAND,
}commands_t;

typedef struct bulb_tail_s
{
    zb_uint8_t brightness;
    zb_uint16_t addr;
}ZB_PACKED_STRUCT bulb_tail_t;

void bulb_parce_packet(zb_uint8_t param) ZB_CALLBACK;
void __attribute__((weak)) bulb_receive_on_command(zb_uint8_t param) ZB_CALLBACK;
void __attribute__((weak)) bulb_receive_off_command(zb_uint8_t param) ZB_CALLBACK;
void __attribute__((weak)) bulb_receive_toggle_command(zb_uint8_t param) ZB_CALLBACK;
void __attribute__((weak)) bulb_receive_brightness_up_command(zb_uint8_t param) ZB_CALLBACK;
void __attribute__((weak)) bulb_receive_brightness_down_command(zb_uint8_t param) ZB_CALLBACK;
void __attribute__((weak)) bulb_receive_brightness_command(zb_uint8_t param) ZB_CALLBACK;
void __attribute__((weak)) bulb_receive_color_command(zb_uint8_t param) ZB_CALLBACK;

void send(zb_callback_t send_command);
void bulb_send_on_command(zb_uint8_t param) ZB_CALLBACK;
void bulb_send_off_command(zb_uint8_t param) ZB_CALLBACK;
void bulb_send_toggle_command(zb_uint8_t param) ZB_CALLBACK;
void bulb_send_brightness_up_command(zb_uint8_t param) ZB_CALLBACK;
void bulb_send_brightness_down_command(zb_uint8_t param) ZB_CALLBACK;
void bulb_send_brightness_command(zb_uint8_t param) ZB_CALLBACK;
void bulb_send_color_command(zb_uint8_t param) ZB_CALLBACK;

#endif  /* ZIGBEE_BULB_H */
