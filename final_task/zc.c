#include "zb_common.h"
#include "zb_aps.h"
#include "zb_bufpool.h"
#include "zb_nwk.h"
#include "zb_ringbuffer.h"
#include "zb_scheduler.h"
#include "zb_zdo.h"

#include "stm32f4xx.h"

#include "./microrl/include/microrl.h"

#define RING_BUFFER_LENGTH 16

ZB_RING_BUFFER_DECLARE(ring_buffer, zb_uint8_t, RING_BUFFER_LENGTH);

#define DISABLE_SERIAL_INTER() NVIC_DisableIRQ(USART2_IRQn)
#define ENABLE_SERIAL_INTER() NVIC_EnableIRQ(USART2_IRQn)

#define ENABLE_SERIAL_TR_INTER() USART_ITConfig(USART2, USART_IT_TXE, ENABLE)
#define DISABLE_SERIAL_TR_INTER() USART_ITConfig(USART2, USART_IT_TXE, DISABLE)

#define _CMD_HELP "help"
#define _CMD_CLEAR "clear"

#define _NUM_OF_CMD 2

/* available  commands */
static const char *keyworld[] = {
    _CMD_HELP,
    _CMD_CLEAR,
};
/* instance of a "micro readline" library */
static microrl_t microrl = {0};
/* ring buffer for writing to usart */
static ring_buffer_t ring_buffer;
/* "tx is busy" flag */
static volatile zb_uint8_t tx_in_progress;

#ifndef ZB_COORDINATOR_ROLE
#error Coordinator role is not compiled!
#endif

zb_ieee_addr_t g_zc_addr = {0x00, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};

MAIN() {
    ARGV_UNUSED;

#if !(defined KEIL || defined SDCC || defined ZB_IAR)
    if (argc < 3) {
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

    if (zdo_dev_start() != RET_OK) {
        TRACE_MSG(TRACE_ERROR, "zdo_dev_start failed", (FMT__0));
    } else {
        zdo_main_loop();
    }

    TRACE_DEINIT();

    MAIN_RETURN(0);
}

/* я использовал светодиоды для дебага */
void init_debug(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Enable peripheral clock for LEDs port */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

    /* Init LEDs */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    /* Turn all the leds off */
    GPIO_ResetBits(GPIOD, GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
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
            ENABLE_SERIAL_TR_INTER();  /* Start transmit */
        }
    }
}

void print_help(void) {
    print(
        "Use TAB key for completion\n\rCommands:\n"
        "\tclear               - clear screen\n");
}

/* execute callback for microrl library *
 * warning: don't write to argv; read only
 */
int execute(int argc, const char *const *argv) {
    int i = 0;
    while (i < argc) {
        if (strcmp(argv[i], _CMD_HELP) == 0) {
            print_help();
        } else if (strcmp(argv[i], _CMD_CLEAR) == 0) {
            print(
                "\033[2J"  /* ESC seq for clear entire screen            */
                "\033[H"); /* ESC seq for move cursor at left-top corner */
        } else {
            print("command: '");
            print((char *)argv[i]);
            print("' Not found.\n\r");
        }
        ++i;
    }
    return 0;
}

/* sigint callback for microrl library */
void sigint(void) {
    print("^C catched!\n\r");
}

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

void zb_zdo_startup_complete(zb_uint8_t param) ZB_CALLBACK {
    zb_buf_t *buf = ZB_BUF_FROM_REF(param);
    TRACE_MSG(TRACE_APS3, ">>zb_zdo_startup_complete status %d", (FMT__D, (int)buf->u.hdr.status));
    if (buf->u.hdr.status == 0) {
        TRACE_MSG(TRACE_APS1, "Device STARTED OK", (FMT__0));
        tx_in_progress = 0;
        ZB_RING_BUFFER_INIT(&ring_buffer);
        /* init_debug(); */
        init_usart();
        microrl_init(&microrl, print);
        microrl_set_execute_callback(&microrl, execute);
        microrl_set_sigint_callback(&microrl, sigint);
    } else {
        TRACE_MSG(TRACE_ERROR, "Device start FAILED status %d", (FMT__D, (int)buf->u.hdr.status));
    }
    zb_free_buf(buf);
}
