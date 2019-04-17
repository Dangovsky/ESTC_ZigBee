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

#endif  /* ZIGBEE_BULB_H */
