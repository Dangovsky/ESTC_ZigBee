#ifndef LIBLED_H 
#define LIBLED_H 

#include <stm32f4xx.h>
#include <stm32f4xx_tim.h>
#include <math.h>

/*! \file led.h
 *  \brief API for libled
 *  
 *  DSR ESTC course 5-th task
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
 *  Fix green color brightnes.
 *  Logistic function for nonlinear brightness change.
 *  \param red, green, blue - brightness of components
 *  \parem alfa - alfa chanel
 */
void led_set_color_ARGB(uint8_t alfa, uint8_t red, uint8_t green, uint8_t blue);

/*! \brief Set choosen LEDs color
 *
 *  Split color on components and calls led_set_color_ARGB.
 *  \param color - 32-bit HEX code, last 8 bit - alfa chanel.
 */
void led_set_color_Hex(uint32_t color);

#endif  /* LIBLED_H */
