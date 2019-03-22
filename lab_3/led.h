#ifndef LIBLED_H 
#define LIBLED_H 

typedef enum commands_e
{
    ON = 0,
    OFF,
    TOGGLE,
    BRIGHTNESS_UP,
    BRIGHTNESS_DOWN,
    BRIGHTNESS,
    COLOR
}commands_t;

typedef struct bulb_addr_payload_s
{
    zb_uint16_t addr;
    zb_uint8_t payload;
}ZB_PACKED_STRUCT bulb_addr_payload_t;

typedef struct bulb_send_payload_s
{
    commands_t command;
    zb_uint8_t payload;
}ZB_PACKED_STRUCT bulb_send_payload_t;

#endif  /* LIBLED_H */
