#ifndef LIBLED_H 
#define LIBLED_H 

#include <stm32f4xx.h>
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
void InitLeds(void);

/*! \brief Set choosen LEDs color
 *
 *  Calculate and set comparators to TIM1's PWM.
 *  Fix green color brightnes.
 *  Logistic function for nonlinear brightness change.
 *  \param red, green, blue - brightness of components
 */
void SetColorRGB(uint8_t red, uint8_t green, uint8_t blue);

/*! \brief Set choosen LEDs color
 *
 *  Calculate and set comparators to TIM1's PWM.
 *  Fix green color brightnes.
 *  Logistic function for nonlinear brightness change.
 *  \param red, green, blue - brightness of components
 *  \parem alfa - overall brightness
 */
void SetColorARGB(uint8_t alfa, uint8_t red, uint8_t green, uint8_t blue);

/*! \brief Set choosen LEDs color
 *
 *  Split color on components and calls SetColorRGB.
 *  \param color - 24-bit HEX code.
 */
void SetColorHex(uint32_t color);

/*! \brief Set choosen LEDs color
 *
 *  Split color on components and calls SetColorARGB.
 *  \param color - 24-bit HEX code.
 */
void SetColorAHex(uint8_t alfa, uint32_t color);

#endif  /* LIBLED_H */
