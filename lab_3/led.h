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

typedef struct bulb_addr_s
{
    zb_uint16_t addr;
}ZB_PACKED_STRUCT bulb_addr_t;

typedef struct bulb_addr_payload_s
{
    zb_uint16_t addr;
    zb_uint16_t payload;
}ZB_PACKED_STRUCT bulb_addr_payload_t;

#endif  /* LIBLED_H */
