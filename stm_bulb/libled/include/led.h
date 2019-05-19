#ifndef LIBLED_H
#define LIBLED_H

#include <stm32f4xx.h>
#include <stm32f4xx_tim.h>

/*! \file led.h
 *  \brief API for libled
 *  
 *  Library simplify initialise and control of 3-color led color and brightness.
 *  Gamma correction for nonlinear brightness change.
 */

/*! \brief Initialise function.
 *
 *  Initialise timer, PWM and Pins.
 *  * Timer - TIM1.
 *  * PWM - from TIM1 with pins AF.
 *  * GPOIA Pins - 8, 9, 10.
 */
void init_led(void);

/*! \brief Set choosen LEDs color
 *
 *  Calculate and set comparators to TIM1's PWM.
 *  Fix green color brightness.
 *  Gamma correction for nonlinear brightness change.
 *  \param red, green, blue - brightness of components
 *  \param alfa - alfa chanel
 */
void led_set_color_argb(uint8_t alfa, uint8_t red, uint8_t green, uint8_t blue);

/*! \brief Set choosen LEDs color
 *
 *  Split color on components and calls led_set_color_argb.
 *  \param color - 32-bit HEX code, last (little endian) 8 bit - alfa chanel.
 */
void led_set_color_hex(uint32_t color);

#endif /* LIBLED_H */
