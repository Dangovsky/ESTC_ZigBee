#include "../include/buttons.h"

#ifdef TIMER
void TIM2_IRQHandler(void)
{
   if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
   {
      timer_count++;
      TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
   }
}
#endif

void EXTI0_IRQHandler(void)
{  
   if (EXTI_GetITStatus(EXTI_Line0) != RESET)
   {

#ifdef TIMER
      if (timer_firstb - timer_secb < TIME_COMP)
      {
         zb_schedule_callback(button_both_click, 0);
      }
      else if (timer_count - timer_firstb > TIME_COMP)
      {
         zb_schedule_callback(button_first_click, 0);
      }
      timer_firstb = timer_count;
#endif
      
#ifdef ZB_ALARMS
      zb_schedule_alarm_cancel(buttons_action, ZB_ALARM_ANY_PARAM);
      first_button = 1;
      zb_schedule_alarm(buttons_action, 0, (zb_uint8_t)(TIME_COMP / ZB_MILLISECONDS_TO_BEACON_INTERVAL(1));
#endif

      EXTI_ClearITPendingBit(EXTI_Line0);
   }
}

void EXTI1_IRQHandler(void)
{   
   if (EXTI_GetITStatus(EXTI_Line1) != RESET)
   {

#ifdef TIMER
      if (timer_firstb - timer_secb < TIME_COMP)
      {
         zb_schedule_callback(button_both_click, 0);
      }
      else if (timer_count - timer_firstb > TIME_COMP)
      {
         zb_schedule_callback(button_second_click, 0);
      }
      timer_secb = timer_count
#endif

#ifdef ZB_ALARMS
      zb_schedule_alarm_cancel(buttons_action, ZB_ALARM_ANY_PARAM);
      second_button = 1;
      zb_schedule_alarm(buttons_action, 0, (zb_uint8_t)(TIME_COMP / ZB_MILLISECONDS_TO_BEACON_INTERVAL(1)));
#endif

      EXTI_ClearITPendingBit(EXTI_Line1);      
   }
}

#ifdef ZB_ALARMS
void buttons_action(zb_uint8_t param) ZB_CALLBACK
{
    if (first_button && second_button)
    {
        zb_schedule_callback(button_both_click, 0);
    }
    else if (first_button)
    {
        zb_schedule_callback(button_first_click, 0);
    }
    else if (second_button)
    {
        zb_schedule_callback(button_second_click, 0);
    }
}
#endif

void init_buttons(void)
{
#ifdef TIMER
  timer_count = 0;
  timer_firstb = 0;
  timer_secb = 0;
#endif

#ifdef ZB_ALARMS
  first_button = 0;
  second_button = 0;
#endif

  GPIO_InitTypeDef GPIO_InitStructure = {0};
  EXTI_InitTypeDef EXTI_InitStruct = {0};  
  NVIC_InitTypeDef nvic_struct = {0};
  TIM_TimeBaseInitTypeDef tim_struct = {0};

  /* Enable peripheral clock for timer 2*/
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
  /* Enable peripheral clock for buttons */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
  /* Enable peripheral clock for SYSCFG */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

  /* Init buttons */
  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0 | GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
  GPIO_Init(GPIOE, &GPIO_InitStructure);

  /* Using GPIOE0 as EXTI_Line0 */
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource0);
  /* Init EXTI_Line0 */
  EXTI_InitStruct.EXTI_Line = EXTI_Line0;
  EXTI_InitStruct.EXTI_LineCmd = ENABLE;
  EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
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
  EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
  EXTI_Init(&EXTI_InitStruct);
  /*  Configurate interrupts for button 1 */
  nvic_struct.NVIC_IRQChannel = EXTI1_IRQn;
  nvic_struct.NVIC_IRQChannelPreemptionPriority = 0;
  nvic_struct.NVIC_IRQChannelSubPriority = 1;
  nvic_struct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&nvic_struct);
#ifdef TIMER
  /* Init timer 2 */
  tim_struct.TIM_Period        = TIM2_PERIOD - 1;
  tim_struct.TIM_Prescaler     = TIM2_PRESCALER - 1;
  tim_struct.TIM_ClockDivision = 0;
  tim_struct.TIM_CounterMode   = TIM_CounterMode_Up;
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

void __attribute__((weak)) button_first_click(zb_uint8_t param) ZB_CALLBACK {}
void __attribute__((weak)) button_second_click(zb_uint8_t param) ZB_CALLBACK {}
void __attribute__((weak)) button_both_click(zb_uint8_t param) ZB_CALLBACK {}