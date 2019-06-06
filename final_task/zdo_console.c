#include <stdio.h>
#include <string.h>
#include "zb_ringbuffer.h"

#include "./cmd_handlers.c"
#include "console.h"

/** Delay before rx_buffer_flush call in milliseconds */
#define RX_DELAY 100

#define RING_BUFFER_LENGTH 16
ZB_RING_BUFFER_DECLARE(ring_buffer, zb_uint8_t, RING_BUFFER_LENGTH);

/**
 * warning: actual baudrate will be in 1.5 times bigger. see BAUD_RATE_CORRECTION.
 */
#define BAUD_RATE 38200
/** Because of zb clock baudrate must be in 1.5 times bigger then on pc */
#define BAUD_RATE_CORRECTION(b) (zb_uint16_t)((b) + (b) / 2)

#define DISABLE_SERIAL_INTER() NVIC_DisableIRQ(USART2_IRQn)
#define ENABLE_SERIAL_INTER() NVIC_EnableIRQ(USART2_IRQn)

#define ENABLE_SERIAL_TR_INTER() USART_ITConfig(USART2, USART_IT_TXE, ENABLE)
#define DISABLE_SERIAL_TR_INTER() USART_ITConfig(USART2, USART_IT_TXE, DISABLE)

#define _CMD_HELP "help"
#define _CMD_CLEAR "clear"
#define _CMD_IEEE_ADDR "ieee"
#define _CMD_ACTIVE_EP "ep"
#define _CMD_SIMPLE_DISC "simple"
#define _CMD_NEIGHBORS "neighbors"
#define _CMD_NWK_ADDR "nwk"
#define _CMD_LEAVE "leave"
#define _CMD_PERMIT_JOIN "permit_join"

#define _NUM_OF_CMD 9

/** available  commands */
static const void *commands[_NUM_OF_CMD][2] = {
    {_CMD_HELP, help_cmd_handler},
    {_CMD_CLEAR, clear_cmd_handler},
    {_CMD_IEEE_ADDR, ieee_cmd_handler},
    {_CMD_ACTIVE_EP, active_ep_cmd_handler},
    {_CMD_SIMPLE_DISC, simple_desk_cmd_handler},
    {_CMD_NEIGHBORS, neighbors_cmd_handler},
    {_CMD_NWK_ADDR, nwk_addr_cmd_handler},
    {_CMD_LEAVE, leave_cmd_handler},
    {_CMD_PERMIT_JOIN, permit_joining_cmd_handler},
};

/** ring buffer for writing to usart */
static ring_buffer_t ring_buffer_tx;

/** ring buffer for sending char to microrl */
static ring_buffer_t ring_buffer_rx;

/** "tx is busy" flag */
static volatile zb_uint8_t tx_in_progress;

/** "rx_buffer_flush is scheduled" flag */
static volatile zb_uint8_t rx_in_progress;

void rx_buffer_flush(zb_uint8_t param) ZB_CALLBACK {
    DISABLE_SERIAL_INTER();
    volatile zb_uint8_t *p = ZB_RING_BUFFER_PEEK(&ring_buffer_rx);
    while (p) {
        ENABLE_SERIAL_INTER();
        microrl_insert_char(&microrl, *p);
        DISABLE_SERIAL_INTER();

        ZB_RING_BUFFER_FLUSH_GET(&ring_buffer_rx);
        /* ((&SER_CTX().tx_buf) -> written--, ((&SER_CTX().tx_buf) -> read_i = ((&SER_CTX().tx_buf)->read_i + 1) % ZB_RING_BUFFER_CAPACITY(&SER_CTX().tx_buf))); 
         */
        p = ZB_RING_BUFFER_PEEK(&ring_buffer_rx);
    }
    ENABLE_SERIAL_INTER();
    rx_in_progress = 0;
}

void USART2_IRQHandler() {
    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
        if (!ZB_RING_BUFFER_IS_FULL(&ring_buffer_rx)) {
            ZB_RING_BUFFER_PUT(&ring_buffer_rx, USART_ReceiveData(USART2));
        }
        if (!rx_in_progress) {
            rx_in_progress = 1;
            ZB_SCHEDULE_ALARM(rx_buffer_flush, 0, ZB_MILLISECONDS_TO_BEACON_INTERVAL(RX_DELAY)); /* Start sending char to microrl */
        }

        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    }

    if (USART_GetITStatus(USART2, USART_IT_TXE) != RESET) {
        volatile zb_uint8_t *p = ZB_RING_BUFFER_PEEK(&ring_buffer_tx);
        if (p) {
            USART_SendData(USART2, *p);
            tx_in_progress = 1;
            ZB_RING_BUFFER_FLUSH_GET(&ring_buffer_tx);
            /* ((&SER_CTX().tx_buf) -> written--, ((&SER_CTX().tx_buf) -> read_i = ((&SER_CTX().tx_buf)->read_i + 1) % ZB_RING_BUFFER_CAPACITY(&SER_CTX().tx_buf))); 
             */
        } else { /* No more data */
            tx_in_progress = 0;
            DISABLE_SERIAL_TR_INTER();
        }

        USART_ClearITPendingBit(USART2, USART_IT_TXE);
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
        if (!ZB_RING_BUFFER_IS_FULL(&ring_buffer_tx)) {
            ZB_RING_BUFFER_PUT(&ring_buffer_tx, *str);
            str++;
            len--;
        }
        ENABLE_SERIAL_INTER();
        if (!tx_in_progress) {
            ENABLE_SERIAL_TR_INTER(); /* Start transmit */
        }
    }
}

/* Actual executing of the command.
 *
 * it is allow to use delayed get_out_buf at the cost of memory. See "execute" function.
 */
void delayed_execute(zb_uint8_t param) ZB_CALLBACK {
    zb_uint8_t i;

    for (i = 0; i < _NUM_OF_CMD; i++) {
        if (!strcmp(argv_g[0], (char *)(commands[i][0]))) {
            ZB_SCHEDULE_CALLBACK((zb_callback_t)commands[i][1], param);
            return;
        }
    }
    print(CLEAR_LINE
          "Unknown command.\n\r"
          "print 'help' to see available commands.");
    interrupt_new_line_handler(&microrl);
}

/* execute callback for microrl library
 * warning: don't write to argv; read only
 *
 * allow to use delayed get_out_buf at the cost of memory
 */
int execute(int argc, const char *const *argv) {
    if (argc == 0) {
        return 1;
    }
    /* copy is needed because argv not be the same after end of this function */
    for (argc_g = 0; argc_g < argc; ++argc_g) {
        strcpy(argv_g[argc_g], argv[argc_g]);
    }

    ZB_GET_OUT_BUF_DELAYED(delayed_execute);
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
    usart.USART_BaudRate = BAUD_RATE_CORRECTION(BAUD_RATE); /* for 38400 on PC */
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
    rx_in_progress = 0;
    ZB_RING_BUFFER_INIT(&ring_buffer_tx);
    ZB_RING_BUFFER_INIT(&ring_buffer_rx);
    init_usart();
    microrl_init(&microrl, print);
    microrl_set_execute_callback(&microrl, execute);
    microrl_set_sigint_callback(&microrl, sigint);
}
