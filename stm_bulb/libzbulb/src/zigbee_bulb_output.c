#include "../include/zigbee_bulb_output.h"

void send_payloaded_command(zb_uint8_t param, zb_uint8_t payload_length, zb_uint8_t* payload)
{
    zb_buf_t *buf = (zb_buf_t*)ZB_BUF_FROM_REF(param);
    zb_uint8_t* ptr;
    zb_uint16_t address = *((zb_uint16_t *)ZB_GET_BUF_TAIL(buf, sizeof(zb_uint16_t)));
    zb_apsde_data_req_t *req = ZB_GET_BUF_TAIL(buf, sizeof(zb_apsde_data_req_t));

    ZB_BUF_INITIAL_ALLOC(buf, ((sizeof *payload) * payload_length), ptr);

    req->dst_addr.addr_short = address; 
    req->addr_mode = ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
    req->tx_options = ZB_APSDE_TX_OPT_ACK_TX;
    req->radius = 1;
    req->profileid = 0x2232;
    req->clusterid = 0x1234;
    req->src_endpoint = 10;
    req->dst_endpoint = 10;

    buf->u.hdr.handle = 0x11;

    memcpy(ptr, payload, payload_length);
    
    if (payload_length > 1)
    {
        TRACE_MSG(TRACE_APS3, "Sending apsde_data.request, command: %x, payload: %x", (FMT__D_D, *payload, *(payload+1)));
    }
    else
    {
        TRACE_MSG(TRACE_APS3, "Sending apsde_data.request, command: %x ", (FMT__D, *payload));
    }

    ZB_SCHEDULE_CALLBACK(zb_apsde_data_request, ZB_REF_FROM_BUF(buf));
}

void bulb_send_on_command(zb_uint8_t param) ZB_CALLBACK
{
    zb_uint8_t command = ON_COMMAND;
    send_payloaded_command(param, 1, &command);
}

void bulb_send_off_command(zb_uint8_t param) ZB_CALLBACK
{
    zb_uint8_t command = OFF_COMMAND;
    send_payloaded_command(param, 1, &command);
}

void bulb_send_toggle_command(zb_uint8_t param) ZB_CALLBACK
{
    zb_uint8_t command = TOGGLE_COMMAND;
    send_payloaded_command(param, 1, &command);
}

void bulb_send_brightness_up_command(zb_uint8_t param) ZB_CALLBACK
{
    zb_uint8_t command = BRIGHTNESS_UP_COMMAND;
    send_payloaded_command(param, 1, &command);
}

void bulb_send_brightness_down_command(zb_uint8_t param) ZB_CALLBACK
{
    zb_uint8_t command = BRIGHTNESS_DOWN_COMMAND;
    send_payloaded_command(param, 1, &command);
}

void bulb_send_brightness_command(zb_uint8_t param) ZB_CALLBACK
{
    bulb_tail_t* tail = ZB_GET_BUF_TAIL((zb_buf_t *)ZB_BUF_FROM_REF(param), sizeof(bulb_tail_t));
    zb_uint8_t payload [2];
    payload[0] = BRIGHTNESS_COMMAND;
    payload[1] = tail->brightness;
    send_payloaded_command(param, 2, payload);
}

void bulb_send_color_command(zb_uint8_t param) ZB_CALLBACK
{
    zb_uint8_t command = COLOR_COMMAND;
    send_payloaded_command(param, 1, &command);
}

void send(zb_callback_t send_command)
{
    zb_buf_t *buf = ZB_GET_OUT_BUF();
    if (brightness)
    {
        bulb_tail_t* tail = ZB_GET_BUF_TAIL(buf, sizeof(bulb_tail_t));
        tail->brightness = brightness;
        tail->addr = addr;
    }
    else
    {
        zb_uint16_t* addr = ZB_GET_BUF_TAIL(buf, sizeof(zb_uint16_t));
        *addr = addr;
    }
    ZB_SCHEDULE_CALLBACK(send_command, ZB_REF_FROM_BUF(buf));
}

void TIM2_IRQHandler(void)
{
   if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
   {
      timer_count++;
      TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
   }
}

void EXTI0_IRQHandler(void)
{  
   if (EXTI_GetITStatus(EXTI_Line0) != RESET)
   {
      if (timer_firstb - timer_secb < TIME_COMP)
      {
          send(bulb_send_color_command);
      }
      else if (timer_count - timer_firstb > TIME_COMP)
      {
          send(bulb_send_toggle_command);
      }
      timer_firstb = timer_count;
      EXTI_ClearITPendingBit(EXTI_Line0);         
   }
}

void EXTI1_IRQHandler(void)
{   
   if (EXTI_GetITStatus(EXTI_Line1) != RESET)
   {
      if (timer_firstb - timer_secb < TIME_COMP)
      {
          send(bulb_send_color_command);
      }
      else if (timer_count - timer_firstb > TIME_COMP)
      {
          send(bulb_send_brightness_up_command);
      }
      timer_secb = timer_count
      EXTI_ClearITPendingBit(EXTI_Line1);      
   }
}

void init_perif()
{
  timer_count = 0;
  timer_firstb = 0;
  timer_secb = 0;

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
}
