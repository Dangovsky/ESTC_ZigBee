#ifndef ZIGBEE_BULB_INPUT_H
#define ZIGBEE_BULB_INPUT_H

#include "zigbee_bulb.h"
#include "../../libled/include/led.h"

#define COLORS_CNT 10
#define BRIGHTNESS_STEP 0.1

/* current brightness */
static uint8_t brightness = 255;
/* bulb state flag */
static bool is_on = 0;
/* index in colors array */
static uint8_t current_color = 0;
/* colors from https://simpledits.com/top-12-pantone-colors-for-spring-2018-with-hex-cmyk-and-rgb-values/ */
static uint8_t colors[COLORS_CNT] = {0xecdb54, 0xe34132, 0x6ca0dc, 0x944743, 0xdbb2d1, 
                                     0xec9787, 0x00a68c, 0x645394, 0x6c4f3d, 0xebe1df};

void bulb_parce_package(zb_uint8_t param) ZB_CALLBACK;

void bulb_update_state(void);

#endif /*ZIGBEE_BULB_INPUT_H*/
