#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_bufpool.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "zb_zdo.h"

#include "stm32f4xx.h"


#if !defined  (HSE_VALUE)
  #define HSE_VALUE    ((uint32_t)8000000) /*!< Value of the External oscillator in Hz */
#endif /* HSE_VALUE */

// Пусть нам надо передать 8 байт, создадим массив для данных
uint8_t sendData[] = "Hello, world!";
uint8_t recievedData = 0xff;
//uint8_t sendData[] = {0xff,0xaa,0x00,0xaa,0xff,0xaa,0x6c,0x92,0x00,0x00,'\0'};
uint8_t bytesToSend = sizeof sendData;
// Счетчик отправленных байт
volatile uint8_t sendDataCounter = 0;

#ifndef ZB_COORDINATOR_ROLE
#error Coordinator role is not compiled!
#endif

zb_ieee_addr_t g_zc_addr = {0x00, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};

MAIN()
{
  ARGV_UNUSED;

#if !(defined KEIL || defined SDCC || defined ZB_IAR)
  if ( argc < 3 )
  {
    //printf("%s <read pipe path> <write pipe path>\n", argv[0]);
    return 0;
  }
#endif

  /* Init device, load IB values from nvram or set it to default */
#ifndef ZB8051
  ZB_INIT("zdo_zc", argv[1], argv[2]);
#else
  ZB_INIT("zdo_zc", "1", "1");
#endif
#ifdef ZB_SECURITY
  ZG->nwk.nib.security_level = 0;
#endif
  ZB_IEEE_ADDR_COPY(ZB_PIB_EXTENDED_ADDRESS(), &g_zc_addr);
  MAC_PIB().mac_pan_id = 0x1aaa;

  /* let's always be coordinator */
  ZB_AIB().aps_designated_coordinator = 1;
  ZB_AIB().aps_channel_mask = (1l << 19);

  if (zdo_dev_start() != RET_OK)
  {
    TRACE_MSG(TRACE_ERROR, "zdo_dev_start failed", (FMT__0));
  }
  else
  {
    zdo_main_loop();
  }

  TRACE_DEINIT();

  MAIN_RETURN(0);
}
// Инициализация 
void init_usart()
{
    // Объявляем переменные
    GPIO_InitTypeDef gpio = {0};
    USART_InitTypeDef usart = {0};

    // Включаем прерывания
    __enable_irq();

    // Запускаем тактирование
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

    // Инициализация нужных пинов контроллера, для USART3 –
    // PC10 и PC11
    GPIO_StructInit(&gpio);

    gpio.GPIO_Mode = GPIO_Mode_AF;
    gpio.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOC, &gpio);

    GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_USART3);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_USART3);

    // А теперь настраиваем модуль USART
    USART_StructInit(&usart);
    usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    usart.USART_BaudRate = 9600;
    usart.USART_Parity = USART_Parity_No;
    usart.USART_StopBits = USART_StopBits_1;
    usart.USART_WordLength = USART_WordLength_8b;
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;

    USART_Init(USART3, &usart);

    // Включаем прерывания и запускаем USART
    NVIC_EnableIRQ(USART3_IRQn);
    USART_Cmd(USART3, ENABLE);

    // Включаем прерывание по окончанию передачи
    USART_ITConfig(USART3, USART_IT_TC, ENABLE);
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
}

// Обработчик прерывания
void USART3_IRQHandler()
{
    if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
        recievedData = USART_ReceiveData(USART3);
        USART_SendData(USART3, recievedData);
        USART_ClearITPendingBit(USART3, USART_IT_RXNE);
    }
    // Проверяем, действительно ли прерывание вызвано окончанием передачи
    if (USART_GetITStatus(USART3, USART_IT_TC) != RESET)
    {
        if  (sendDataCounter < bytesToSend)
        {
            // Отправляем байт данных
            USART_SendData(USART3, sendData[sendDataCounter]);

            // Увеличиваем счетчик отправленных байт
            sendDataCounter++;
        }
        // Очищаем флаг прерывания
        USART_ClearITPendingBit(USART3, USART_IT_TC);
    }
}

void zb_zdo_startup_complete(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  TRACE_MSG(TRACE_APS3, ">>zb_zdo_startup_complete status %d", (FMT__D, (int)buf->u.hdr.status));
  if (buf->u.hdr.status == 0)
  {
    TRACE_MSG(TRACE_APS1, "Device STARTED OK", (FMT__0));
    sendDataCounter = 0;
    recievedData = 0xff;
    init_usart();
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "Device start FAILED status %d", (FMT__D, (int)buf->u.hdr.status));
  }
  zb_free_buf(buf);
}

/*! @} */
