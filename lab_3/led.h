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

typedef struct bulb_tail_s
{
    zb_uint16_t addr;
    zb_uint8_t brightness;
}ZB_PACKED_STRUCT bulb_tail_t;

#endif  /* LIBLED_H */
