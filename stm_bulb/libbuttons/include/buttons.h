#ifndef BUTTONS_H
#define BUTTONS_H

#include <stm32f4xx.h>
#include <stm32f4xx_exti.h>
#include <stm32f4xx_tim.h>
#include "zb_common.h"
#include "zb_bufpool.h"
#include "zb_scheduler.h"

/*! \file buttons.h
 *  \brief API for libbuttons
 *
 *  Library simplify buttons usage by calling functions from button_handlers_t,
 *  which passed to init_buttons.
 *  Use defines BUTTONS_TIMER or BUTTONS_ZB_ALARMS for choosing way of debouncing. 
 */

/*! 
 *  Structure to represent available actions with buttons.
 */
typedef struct button_handlers_s {
    zb_callback_t button_left_click;  /*!< Callback to schedule when left button pressed */
    zb_callback_t button_right_click; /*!< Callback to schedule when right button pressed */
    zb_callback_t button_both_click;  /*!< Callback to schedule when both button pressed at the same time*/
} button_handlers_t;

/*! \brief Initialise function
 * 
 *  Initialise timer(if BUTTONS_TIMER defined), pins for buttins, EXTI and NVIC
 *  * Timer - TIM2 
 *  * Pins - GPIO3 pins 0 and 1 
 *  * EXTI - line 0 and 1
 *  * NVIC - corresponding to EXTI and TIM2
 *  \param handlers - callbacks for buttons actions.
 */
void init_buttons(button_handlers_t *handlers);

#endif /* BUTTONS_H */
