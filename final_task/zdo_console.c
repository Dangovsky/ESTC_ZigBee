#include <stdio.h>
#include <string.h>
#include "zb_ringbuffer.h"

#include "./cmd_handlers.c"
#include "console.h"

/** Delay before rx_buffer_flush call in milliseconds */
#define RX_DELAY 100

/** String size used to store one word from console */
#define WORD_LEN 20

/** Ring buffer length of zb_uint8_t */
#define RING_BUFFER_LENGTH 16
ZB_RING_BUFFER_DECLARE(ring_buffer, zb_uint8_t, RING_BUFFER_LENGTH);

/**
 * warning: actual baudrate will be in 1.5 times bigger. see BAUD_RATE_CORRECTION.
 */
#define BAUD_RATE 38200
/** Because of zb clock, baudrate must be in 1.5 times bigger then on pc */
#define BAUD_RATE_CORRECTION(b) (zb_uint16_t)((b) + (b) / 2)

/** duplicate ZB macro, but with diffirent USART */
#define DISABLE_SERIAL_INTER() NVIC_DisableIRQ(USART2_IRQn)
/** duplicate ZB macro, but with diffirent USART */
#define ENABLE_SERIAL_INTER() NVIC_EnableIRQ(USART2_IRQn)

/** duplicate ZB macro, but with diffirent USART */
#define ENABLE_SERIAL_TR_INTER() USART_ITConfig(USART2, USART_IT_TXE, ENABLE)
/** duplicate ZB macro, but with diffirent USART */
#define DISABLE_SERIAL_TR_INTER() USART_ITConfig(USART2, USART_IT_TXE, DISABLE)

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
    {_CMD_BEACON_REQ, beacon_cmd_handler},
    {_CMD_DATA_REQ, data_req_handler},
};

static struct {
    char *variants[_NUM_OF_CMD + 1]; /*!< array for 'complet' function */

    ring_buffer_t ring_buffer_tx; /*!< ring buffer for writing to usart */

    ring_buffer_t ring_buffer_rx; /*!< ring buffer for sending char to microrl */

    microrl_t microrl; /*!< Instance of a "micro readline" library. */

    volatile int current_argc; /*!< "execute" function argument - num of words in command line. */

    char current_argv[_COMMAND_TOKEN_NMB][WORD_LEN + 1]; /*!< "execute" function argument - words in command line. */

    volatile zb_uint8_t tx_in_progress : 1; /*!< "tx is busy" flag */

    volatile zb_uint8_t rx_in_progress : 1; /*!< "rx_buffer_flush is scheduled" flag */

    volatile zb_uint8_t command_in_progress : 1; /*!< "command is executing or waiting for callback" flag */
} context;

microrl_t *get_current_microrl() {
    return &(context.microrl);
}

int get_current_argc() {
    return context.current_argc;
}

char *get_current_argv(zb_uint8_t i) {
    return context.current_argv[i];
}

void set_command_in_progress(zb_uint8_t flag) {
    context.command_in_progress = flag;
}

/** send all input values from ring_buffer_rx to microrl */
void rx_buffer_flush(zb_uint8_t param) ZB_CALLBACK {
    DISABLE_SERIAL_INTER();
    volatile zb_uint8_t *p = ZB_RING_BUFFER_PEEK(&(context.ring_buffer_rx));
    while (p) {
        ENABLE_SERIAL_INTER();
        microrl_insert_char(&(context.microrl), *p);
        DISABLE_SERIAL_INTER();

        ZB_RING_BUFFER_FLUSH_GET(&(context.ring_buffer_rx));
        /* ((&SER_CTX().tx_buf) -> written--, ((&SER_CTX().tx_buf) -> read_i = ((&SER_CTX().tx_buf)->read_i + 1) % ZB_RING_BUFFER_CAPACITY(&SER_CTX().tx_buf))); 
         */
        p = ZB_RING_BUFFER_PEEK(&(context.ring_buffer_rx));
    }
    ENABLE_SERIAL_INTER();
    context.rx_in_progress = 0;
}

void USART2_IRQHandler() {
    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
        if (!ZB_RING_BUFFER_IS_FULL(&(context.ring_buffer_rx))) {
            ZB_RING_BUFFER_PUT(&(context.ring_buffer_rx), USART_ReceiveData(USART2));
        }
        if (!context.rx_in_progress) {
            context.rx_in_progress = 1;
            ZB_SCHEDULE_ALARM(rx_buffer_flush, 0, ZB_MILLISECONDS_TO_BEACON_INTERVAL(RX_DELAY)); /* Start sending char to microrl */
        }

        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    }

    if (USART_GetITStatus(USART2, USART_IT_TXE) != RESET) {
        volatile zb_uint8_t *p = ZB_RING_BUFFER_PEEK(&(context.ring_buffer_tx));
        if (p) {
            USART_SendData(USART2, *p);
            context.tx_in_progress = 1;
            ZB_RING_BUFFER_FLUSH_GET(&(context.ring_buffer_tx));
            /* ((&SER_CTX().tx_buf) -> written--, ((&SER_CTX().tx_buf) -> read_i = ((&SER_CTX().tx_buf)->read_i + 1) % ZB_RING_BUFFER_CAPACITY(&SER_CTX().tx_buf))); 
             */
        } else { /* No more data */
            context.tx_in_progress = 0;
            DISABLE_SERIAL_TR_INTER();
        }

        USART_ClearITPendingBit(USART2, USART_IT_TXE);
    }
}

/** print callback for microrl library */
void print(const char *str) {
    while (context.tx_in_progress)
        ;
    while (*str) {
        DISABLE_SERIAL_INTER();
        if (!ZB_RING_BUFFER_IS_FULL(&(context.ring_buffer_tx))) {
            ZB_RING_BUFFER_PUT(&(context.ring_buffer_tx), *str);
            str++;
        }
        ENABLE_SERIAL_INTER();
        if (!context.tx_in_progress) {
            ENABLE_SERIAL_TR_INTER(); /* Start transmit */
        }
    }
}

/**
 * @brief Actual executing of the command.
 *
 * it is allow to use delayed get_out_buf at the cost of memory. See "execute" function.
 */
void delayed_execute(zb_uint8_t param) ZB_CALLBACK {
    zb_uint8_t i;

    for (i = 0; i < _NUM_OF_CMD; i++) {
        if (!strcmp(context.current_argv[0], (char *)(commands[i][0]))) {
            ZB_SCHEDULE_CALLBACK((zb_callback_t)commands[i][1], param);
            return;
        }
    }
    print(CLEAR_LINE
          "Unknown command.\n\r"
          "print 'help' to see available commands.");
    WRITE_PROMPT
    context.command_in_progress = 0;
}

/**
 * @brief execute callback for microrl library
 *
 * allow to use delayed get_out_buf at the cost of memory
 */
int execute(int argc, const char *const *argv) {
    if (0 == argc || context.command_in_progress) {
        return 1;
    }
    context.command_in_progress = 1;

    /* copy is needed because argv not be the same after end of this function */
    for (context.current_argc = 0; context.current_argc < argc; ++context.current_argc) {
        strcpy(context.current_argv[context.current_argc], argv[context.current_argc]);
    }

    ZB_GET_OUT_BUF_DELAYED(delayed_execute);
    return 0;
}

/* sigint callback for microrl library */
void sigint(void) {
    print("\n\r^C");
    WRITE_PROMPT
    set_command_in_progress(0);
}

/** completion callback for microrl library */
char **complet(int argc, const char *const *argv) {
    zb_uint8_t j = 0;
    char *bit;

    context.variants[0] = NULL;

    /* if there is token in cmdline */
    if (argc == 1) {
        /* get last entered token */
        bit = (char *)argv[argc - 1];
        /* iterate through our available token and match it */
        for (int i = 0; i < _NUM_OF_CMD; i++) {
            if (!strncmp(commands[i][0], bit, strlen(bit))) {
                context.variants[j] = (char *)(commands[i][0]);
                ++j;
            }
        }
    } else { /* if there is no token in cmdline, just print all available token */
        for (; j < _NUM_OF_CMD; j++) {
            context.variants[j] = (char *)(commands[j][0]);
        }
    }

    /* note: last ptr in array always must be NULL */
    context.variants[j] = NULL;

    return context.variants;
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
    context.command_in_progress = 0;
    context.tx_in_progress = 0;
    context.rx_in_progress = 0;
    ZB_RING_BUFFER_INIT(&(context.ring_buffer_tx));
    ZB_RING_BUFFER_INIT(&(context.ring_buffer_rx));

    init_usart();

    microrl_init(&(context.microrl), print);
    microrl_set_execute_callback(&(context.microrl), execute);
    microrl_set_sigint_callback(&(context.microrl), sigint);
    microrl_set_complete_callback(&(context.microrl), complet);
}
