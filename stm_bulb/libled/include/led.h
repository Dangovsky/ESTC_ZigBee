#ifndef LIBLED_H 
#define LIBLED_H 

#include <stm32f4xx.h>
#include <math.h>

#define TIM1_PERIOD 16400
#define TIM1_PRESCALER 1
#define TIM1_PULSE 16400
#define STEP 64

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
 *  \parem alfa - alfa chanel
 */
void SetColorARGB(uint8_t alfa, uint8_t red, uint8_t green, uint8_t blue);

/*! \brief Set choosen LEDs color
 *
 *  Split color on components and calls SetColorARGB.
 *  \param color - 32-bit HEX code, last 8 bit - alfa chanel.
 */
void SetColorHex(uint32_t color);

#endif  /* LIBLED_H */
