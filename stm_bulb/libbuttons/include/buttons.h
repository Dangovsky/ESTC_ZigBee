#ifndef BUTTONS_H
#define BUTTONS_H

#include <stm32f4xx.h>
#include <stm32f4xx_tim.h>
#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_bufpool.h"

typedef struct button_handlers_s
{
    zb_callback_t button_first_click;
    zb_callback_t button_second_click;
    zb_callback_t button_both_click;
}ZB_PACKED_STRUCT button_handlers_t;

void init_buttons(button_handlers_t* handlers);

#endif /* BUTTONS_H */
