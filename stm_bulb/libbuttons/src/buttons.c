#include "../include/buttons.h"

#define TIME_COMP 100 /* time in milliseconds, used for debouncing */

#if defined(BUTTONS_TIMER) && defined(BUTTONS_ZB_ALARMS)
#error defined both timer and zb alarms, choose only one
#endif

#if !defined(BUTTONS_TIMER) && !defined(BUTTONS_ZB_ALARMS)
#error define timer or zb alarms
#endif

#ifdef BUTTONS_TIMER
#define TIM2_PERIOD 16400
#define TIM2_PRESCALER 6 /* this is 6 because strange zboss clocks 
                          * and to make TIME_COMP independet of type of debouncing, should be 1 */
volatile zb_uint8_t timer_count;
#endif

volatile zb_uint8_t first_button;
volatile zb_uint8_t second_button;
button_handlers_t button_handlers = {0};

void buttons_action(zb_uint8_t param) ZB_CALLBACK {
    if (first_button && second_button) {
        if (button_handlers.button_both_click != NULL) {
#ifdef BUTTONS_ZB_ALARMS
            zb_schedule_callback(button_handlers.button_both_click, 0);
#endif

#ifdef BUTTONS_TIMER
            button_handlers.button_both_click(0);
#endif
        }
    } else if (first_button) {
        if (button_handlers.button_left_click != NULL) {
#ifdef BUTTONS_ZB_ALARMS
            zb_schedule_callback(button_handlers.button_left_click, 0);
#endif

#ifdef BUTTONS_TIMER
            button_handlers.button_left_click(0);
#endif
        }
    } else if (second_button) {
        if (button_handlers.button_right_click != NULL) {
#ifdef BUTTONS_ZB_ALARMS
            zb_schedule_callback(button_handlers.button_right_click, 0);
#endif

#ifdef BUTTONS_TIMER
            button_handlers.button_right_click(0);
#endif
        }
    }
    first_button = second_button = 0;
}

#ifdef BUTTONS_TIMER
void TIM2_IRQHandler(void) {
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
        if (timer_count == TIME_COMP) {
            buttons_action(0);
            TIM_Cmd(TIM2, DISABLE);
        }
        timer_count++;
    }
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
}
#endif

void EXTI0_IRQHandler(void) {
    if (EXTI_GetITStatus(EXTI_Line0) != RESET) {
#ifdef BUTTONS_TIMER
        timer_count = 0;
        TIM_Cmd(TIM2, ENABLE);
        first_button = 1;
#endif

#ifdef BUTTONS_ZB_ALARMS
        zb_schedule_alarm_cancel(buttons_action, ZB_ALARM_ANY_PARAM);
        first_button = 1;
        zb_schedule_alarm(buttons_action, 0, ZB_MILLISECONDS_TO_BEACON_INTERVAL(TIME_COMP));
#endif

        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}

void EXTI1_IRQHandler(void) {
    if (EXTI_GetITStatus(EXTI_Line1) != RESET) {
#ifdef BUTTONS_TIMER
        timer_count = 0;
        TIM_Cmd(TIM2, ENABLE);
        second_button = 1;
#endif

#ifdef BUTTONS_ZB_ALARMS
        zb_schedule_alarm_cancel(buttons_action, ZB_ALARM_ANY_PARAM);
        second_button = 1;
        zb_schedule_alarm(buttons_action, 0, ZB_MILLISECONDS_TO_BEACON_INTERVAL(TIME_COMP));
#endif

        EXTI_ClearITPendingBit(EXTI_Line1);
    }
}

void init_buttons(button_handlers_t *handlers) {
#ifdef BUTTONS_ZB_ALARMS
    first_button = 0;
    second_button = 0;
#endif

    button_handlers = *handlers;

    GPIO_InitTypeDef GPIO_InitStructure = {0};
    EXTI_InitTypeDef EXTI_InitStruct = {0};
    NVIC_InitTypeDef nvic_struct = {0};

#ifdef BUTTONS_TIMER
    TIM_TimeBaseInitTypeDef tim_struct = {0};
#endif

    /* Enable peripheral clock for timer 2*/
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    /* Enable peripheral clock for buttons */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
    /* Enable peripheral clock for SYSCFG */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    /* Init buttons */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    /* Using GPIOE0 as EXTI_Line0 */
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource0);
    /* Init EXTI_Line0 */
    EXTI_InitStruct.EXTI_Line = EXTI_Line0;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_Init(&EXTI_InitStruct);
    /* Configurate interrupts for button 0 */
    nvic_struct.NVIC_IRQChannel = EXTI0_IRQn;
    nvic_struct.NVIC_IRQChannelPreemptionPriority = 0;
    nvic_struct.NVIC_IRQChannelSubPriority = 0;
    nvic_struct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic_struct);

    /* Using GPIOE0 as EXTI_Line1 */
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource1);
    /* Init EXTI_Line1 */
    EXTI_InitStruct.EXTI_Line = EXTI_Line1;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_Init(&EXTI_InitStruct);
    /*  Configurate interrupts for button 1 */
    nvic_struct.NVIC_IRQChannel = EXTI1_IRQn;
    nvic_struct.NVIC_IRQChannelPreemptionPriority = 0;
    nvic_struct.NVIC_IRQChannelSubPriority = 1;
    nvic_struct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic_struct);
#ifdef BUTTONS_TIMER
    /* Init timer 2 */
    tim_struct.TIM_Period = TIM2_PERIOD - 1;
    tim_struct.TIM_Prescaler = TIM2_PRESCALER - 1;
    tim_struct.TIM_ClockDivision = 0;
    tim_struct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &tim_struct);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM2, ENABLE);
    /* Configurate interrupts for timer 2 */
    nvic_struct.NVIC_IRQChannel = TIM2_IRQn;
    nvic_struct.NVIC_IRQChannelPreemptionPriority = 0;
    nvic_struct.NVIC_IRQChannelSubPriority = 1;
    nvic_struct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic_struct);
#endif
}
