#include "zb_ringbuffer.h"
#include <string.h>
#include <stdio.h>

#include "./microrl/include/microrl.h"
#include "./cmd_handlers.c"

#define RING_BUFFER_LENGTH 16

ZB_RING_BUFFER_DECLARE(ring_buffer, zb_uint8_t, RING_BUFFER_LENGTH);

#define DISABLE_SERIAL_INTER() NVIC_DisableIRQ(USART2_IRQn)
#define ENABLE_SERIAL_INTER() NVIC_EnableIRQ(USART2_IRQn)

#define ENABLE_SERIAL_TR_INTER() USART_ITConfig(USART2, USART_IT_TXE, ENABLE)
#define DISABLE_SERIAL_TR_INTER() USART_ITConfig(USART2, USART_IT_TXE, DISABLE)

#define _CMD_HELP "help"
#define _CMD_CLEAR "clear"
#define _CMD_IEEE_ADDR "ieee"
#define _CMD_ACTIVE_EP "ep"
#define _CMD_SIMPLE_DISC "simple"

#define _NUM_OF_CMD 5

typedef int cmd_t(int, const char* const*);

/* available  commands */
static const void *commands[][2] = {
    {_CMD_HELP, help_cmd_handler},
    {_CMD_CLEAR, clear_cmd_handler},
    {_CMD_IEEE_ADDR, ieee_cmd_handler},
    {_CMD_ACTIVE_EP, active_ep_cmd_handler},
    {_CMD_SIMPLE_DISC, simple_disk_cmd_handler},
};
/* instance of a "micro readline" library */
microrl_t microrl = {0};
/* ring buffer for writing to usart */
static ring_buffer_t ring_buffer;
/* "tx is busy" flag */
static volatile zb_uint8_t tx_in_progress;


/* костыль, чтобы вызвать microrl_insert_char вне прирывания */
void microrl_insert(zb_uint8_t ch) ZB_CALLBACK {
    microrl_insert_char(&microrl, ch);
}

void USART2_IRQHandler() {
    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
        /* костыль, чтобы вызвать microrl_insert_char вне прирывания
         * microrl_insert_char(&microrl, USART_ReceiveData(USART2)); */
        ZB_SCHEDULE_CALLBACK(microrl_insert, USART_ReceiveData(USART2));
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    }

    if (USART_GetITStatus(USART2, USART_IT_TXE) != RESET) {
        volatile zb_uint8_t *p = ZB_RING_BUFFER_PEEK(&ring_buffer);
        if (p) {
            USART_SendData(USART2, *p);
            tx_in_progress = 1;
            ZB_RING_BUFFER_FLUSH_GET(&ring_buffer);
            /* ((&SER_CTX().tx_buf) -> written--, ((&SER_CTX().tx_buf) -> read_i = ((&SER_CTX().tx_buf)->read_i + 1) % ZB_RING_BUFFER_CAPACITY(&SER_CTX().tx_buf))); 
             */
        } else { /* No more data */
            tx_in_progress = 0;
            DISABLE_SERIAL_TR_INTER();
        }
    }
}

/* print callback for microrl library */
void print(const char *str) {
    zb_uint8_t len;

    while (tx_in_progress)
        ;
    len = strlen(str);
    while (len) {
        DISABLE_SERIAL_INTER();
        if (!ZB_RING_BUFFER_IS_FULL(&ring_buffer)) {
            ZB_RING_BUFFER_PUT(&ring_buffer, *str);
            str++;
            len--;
        }
        ENABLE_SERIAL_INTER();
        if (!tx_in_progress) {
            ENABLE_SERIAL_TR_INTER(); /* Start transmit */
        }
    }
}

/* execute callback for microrl library
 * warning: don't write to argv; read only
 */
int execute(int argc, const char *const *argv) {
    if (argc == 0) {
        return 1;
    }

    zb_uint_t i, err = 1;

    for(i = 0; i < _NUM_OF_CMD; i++) {
        if (!strcmp(argv[0], (char*)(commands[i][0]))) {
            err = ((cmd_t*)commands[i][1])(argc, argv);
            break;
        }
    }

    if (err)
    {
        print("Unknown command or argument missed\n\r"
              "print 'help' to see available commands.\n\r");
    }
    return 0;
}

/* sigint callback for microrl library */
void sigint(void) {
    print("^C catched!\n\r");
}

/* USART2
 * | TX  | RX  |
 * | PD5 | PD6 |
 */
void init_usart(void) {
    GPIO_InitTypeDef gpio = {0};
    USART_InitTypeDef usart = {0};

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

    GPIO_StructInit(&gpio);

    gpio.GPIO_Mode = GPIO_Mode_AF;
    gpio.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOD, &gpio);

    GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_USART2);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource6, GPIO_AF_USART2);

    USART_StructInit(&usart);
    usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    usart.USART_BaudRate = 57600; /* for 38400 on PC */
    usart.USART_Parity = USART_Parity_No;
    usart.USART_StopBits = USART_StopBits_1;
    usart.USART_WordLength = USART_WordLength_8b;
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;

    USART_Init(USART2, &usart);

    NVIC_EnableIRQ(USART2_IRQn);
    USART_Cmd(USART2, ENABLE);

    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
}

void init_console(void) {
    tx_in_progress = 0;
    ZB_RING_BUFFER_INIT(&ring_buffer);
    init_usart();
    microrl_init(&microrl, print);
    microrl_set_execute_callback(&microrl, execute);
    microrl_set_sigint_callback(&microrl, sigint);
}

