#include "zb_ringbuffer.h"
#include <string.h>
#include <stdio.h>

#include "./microrl/include/microrl.h"

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

/* available  commands */
static const char *key_words[] = {
    _CMD_HELP,
    _CMD_CLEAR,
    _CMD_IEEE_ADDR,
    _CMD_ACTIVE_EP,
    _CMD_SIMPLE_DISC,
};
/* instance of a "micro readline" library */
static microrl_t microrl = {0};
/* ring buffer for writing to usart */
static ring_buffer_t ring_buffer;
/* "tx is busy" flag */
static volatile zb_uint8_t tx_in_progress;

void zdo_desc_ieee_addr(zb_uint8_t param) ZB_CALLBACK;
void zdo_desc_active_ep(zb_uint8_t param) ZB_CALLBACK;
void zdo_simple_desk(zb_uint8_t param) ZB_CALLBACK;

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

void print_help(void) {
    print(
        "Use TAB key for completion\n\rCommands:\n\r"
        "\tclear                  - clear screen\n\r"
        "\tieee   [nwk addr]      - get ieee address discriptor\n\r"
        "\tep     [nwk addr]      - get active endpoints discriptor\n\r"
        "\tsimple [nwk addr] [ep] - get simple discriptor\n\r");
}

/* execute callback for microrl library
 * warning: don't write to argv; read only
 */
int execute(int argc, const char *const *argv) {
    if (argc == 0) {
        return 1;
    }

    int i = 0;
    zb_buf_t *buf;
    zb_uint16_t *tail;
    zb_zdo_simple_desc_req_t *req;

    if (strcmp(argv[i], _CMD_HELP) == 0) {
        print_help();
        return 0;
    } else if (strcmp(argv[i], _CMD_CLEAR) == 0) {
        print(
            "\033[2J"  /* ESC seq for clear entire screen            */
            "\033[H"); /* ESC seq for move cursor at left-top corner */
        return 0;
    } else if (strcmp(argv[i], _CMD_IEEE_ADDR) == 0) {
        buf = zb_get_out_buf();
        tail = ZB_GET_BUF_TAIL(buf, sizeof(zb_uint16_t));

        ++i;
        if (i >= argc) {
            print(_CMD_IEEE_ADDR " [nwk addr]\n\r");
            return 1;
        }
        *tail = atoi(argv[i]);

        ZB_SCHEDULE_CALLBACK(zdo_desc_ieee_addr, ZB_REF_FROM_BUF(buf));
        return 0;
    } else if (strcmp(argv[i], _CMD_ACTIVE_EP) == 0) {
        buf = zb_get_out_buf();
        tail = ZB_GET_BUF_TAIL(buf, sizeof(zb_uint16_t));

        ++i;
        if (i >= argc) {
            print(_CMD_ACTIVE_EP " [nwk addr]\n\r");
            return 1;
        }
        *tail = atoi(argv[i]);

        ZB_SCHEDULE_CALLBACK(zdo_desc_active_ep, ZB_REF_FROM_BUF(buf));
        return 0;
    } else if (strcmp(argv[i], _CMD_SIMPLE_DISC) == 0) {
        buf = zb_get_out_buf();
        req = ZB_GET_BUF_TAIL(buf, sizeof(zb_zdo_simple_desc_req_t));

        ++i;
        if (i >= argc) {
            print(_CMD_SIMPLE_DISC " [nwk addr] [endpoint]\n\r");
            return 1;
        }
        req->nwk_addr = atoi(argv[i]);

        ++i;
        if (i >= argc) {
            print("\n\r" _CMD_SIMPLE_DISC " [nwc addr] [endpoint]\n\r");
            return 1;
        }
        req->endpoint = atoi(argv[i]);

        ZB_SCHEDULE_CALLBACK(zdo_simple_desk, ZB_REF_FROM_BUF(buf));
        return 0;
    } else {
        print("command: '");
        print((char *)argv[i]);
        print("' Not found.\n\r");
    }
    ++i;
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

/* IEEE */
#ifdef IEEE_TEST
void ieee_addr_callback(zb_uint8_t param) ZB_CALLBACK {
    zb_uint8_t status = 15, nwk_addr = 10, ieee_addr[] = {12, 13, 14, 15, 16, 18, 19, 20};
    char str[100];
    sprintf(str, "\n\rGet IEEE address responce:\n\r\tstatus: %hd\n\r\tnwk address: %d\n\r",
            status, nwk_addr);            
    print(str);
    sprintf(str, "\tieee address: %hd %hd %hd %hd %hd %hd %hd %hd\n\r",
            ieee_addr[7], ieee_addr[6], ieee_addr[5], ieee_addr[4], ieee_addr[3], ieee_addr[2], ieee_addr[1], ieee_addr[0]);
    print(str);
    interrupt_new_line_handler(&microrl);
}
#else
void ieee_addr_callback(zb_uint8_t param) ZB_CALLBACK {
    zb_buf_t *buf = ZB_BUF_FROM_REF(param);
    zb_zdo_nwk_addr_resp_head_t *resp;
    zb_ieee_addr_t ieee_addr;
    zb_uint16_t nwk_addr;
    char *str = NULL;

    resp = (zb_zdo_nwk_addr_resp_head_t *)ZB_BUF_BEGIN(buf);

    ZB_LETOH64(ieee_addr, resp->ieee_addr);
    ZB_LETOH16(&nwk_addr, &resp->nwk_addr);

    sprintf(str, "\n\rGet IEEE address responce:\n\r\tstatus: %hd\n\r\tnwk address: %d\n\r\tieee address: %hd %hd %hd %hd %hd %hd %hd %hd\n\r",
            resp->status, nwk_addr,
            ieee_addr[7], ieee_addr[6], ieee_addr[5], ieee_addr[4], ieee_addr[3], ieee_addr[2], ieee_addr[1], ieee_addr[0]);
    print(str);
    interrupt_new_line_handler(&microrl);

    zb_free_buf(buf);
}
#endif /* IEEE_TEST */

#ifdef IEEE_TEST
void zdo_desc_ieee_addr(zb_uint8_t param) ZB_CALLBACK {
    ZB_SCHEDULE_ALARM(ieee_addr_callback,0,250);
    zb_free_buf(ZB_BUF_FROM_REF(param));
}
#else
void zdo_desc_ieee_addr(zb_uint8_t param) ZB_CALLBACK {
    zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
    zb_zdo_ieee_addr_req_t *req = NULL;
    zb_uint16_t addr = *((zb_uint16_t *)ZB_GET_BUF_TAIL(buf, sizeof(zb_uint16_t)));

    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_ieee_addr_req_t), req);

    req->nwk_addr = addr;
    req->request_type = ZB_ZDO_SINGLE_DEV_RESPONSE;
    req->start_index = 0;
    zb_zdo_ieee_addr_req(ZB_REF_FROM_BUF(buf), ieee_addr_callback);
}
#endif /* IEEE_TEST */

/* active ep */
void active_ep_callback(zb_uint8_t param) ZB_CALLBACK {
    zb_buf_t *buf = ZB_BUF_FROM_REF(param);
    zb_uint8_t *zdp_cmd = ZB_BUF_BEGIN(buf);
    zb_zdo_ep_resp_t *resp = (zb_zdo_ep_resp_t *)zdp_cmd;
    zb_uint8_t *ep_list = zdp_cmd + sizeof(zb_zdo_ep_resp_t);
    char *str = NULL;
    zb_uint8_t i;

    sprintf(str, "\n\rActive endpoint callback status: %hd, addr: 0x%x",
            resp->status, resp->nwk_addr);
    print(str);

    if (resp->status != ZB_ZDP_STATUS_SUCCESS || resp->nwk_addr != 0x0) {
        print("\n\rError incorrect status/addr");
    }

    sprintf(str, "\n\rep count: %hd\n\r", resp->ep_count);
    print(str);
    for (i = 0, print("\teps: "); i < resp->ep_count; ++i) {
        sprintf(str,"%d ",ep_list[i]);
        print(str);
    }
    interrupt_new_line_handler(&microrl);

    zb_free_buf(buf);
}
void zdo_desc_active_ep(zb_uint8_t param) ZB_CALLBACK {
    zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
    zb_zdo_active_ep_req_t *req;
    zb_uint16_t addr = *((zb_uint16_t *)ZB_GET_BUF_TAIL(buf, sizeof(zb_uint16_t)));

    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_active_ep_req_t), req);
    req->nwk_addr = addr;
    zb_zdo_active_ep_req(ZB_REF_FROM_BUF(buf), active_ep_callback);
}


/* simple discriptor */
void simple_desc_callback(zb_uint8_t param) ZB_CALLBACK {
    zb_buf_t *buf = ZB_BUF_FROM_REF(param);
    zb_uint8_t *zdp_cmd = ZB_BUF_BEGIN(buf);
    zb_zdo_simple_desc_resp_t *resp = (zb_zdo_simple_desc_resp_t *)(zdp_cmd);
    char *str = NULL;
    zb_uint_t i;

    sprintf(str, "\n\rSimple desciptor status %hd, addr 0x%x\n\r",
            resp->hdr.status, resp->hdr.nwk_addr);
    print(str);

    if (resp->hdr.status != ZB_ZDP_STATUS_SUCCESS || resp->hdr.nwk_addr != 0x0) {
        print("Error incorrect status/addr");
    }

    sprintf(str, "\tep: %d, app profile: %d, dev id: %d, dev ver: %d\n\r\tinput clusters count: 0x%x\n\r",
            resp->simple_desc.endpoint,
            resp->simple_desc.app_profile_id,
            resp->simple_desc.app_device_id,
            resp->simple_desc.app_device_version,
            resp->simple_desc.app_input_cluster_count);
    print(str);

    zb_uint8_t *cnt_ptr = &(resp->simple_desc.app_input_cluster_count);
    zb_uint16_t *ptr = (zb_uint16_t *)(cnt_ptr + 1);

    for (i = 0, print("\n\r\tinput clusters: "); i < *(cnt_ptr); i++) {
        sprintf(str,"%d ",*ptr);
        print(str);
        ++ptr;
    }

    cnt_ptr = (zb_uint8_t *)ptr;
    ptr = (zb_uint16_t *)(cnt_ptr + 1);

    sprintf(str, "\n\r\totput cluster count %x", *(cnt_ptr));
    for (i = 0, print("\n\r\tinput clusters: "); i < *(cnt_ptr); i++) {
        sprintf(str,"%d ",*ptr);
        print(str);
        ++ptr;
    }
    interrupt_new_line_handler(&microrl);

    zb_free_buf(buf);
}

void zdo_simple_desk(zb_uint8_t param) ZB_CALLBACK {
    zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
    zb_zdo_simple_desc_req_t *req_tail = ZB_GET_BUF_TAIL(buf, sizeof(zb_zdo_simple_desc_req_t));
    zb_zdo_simple_desc_req_t *req;

    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_simple_desc_req_t), req);
    req->nwk_addr = req_tail->nwk_addr;
    req->endpoint = req_tail->endpoint;
    zb_zdo_simple_desc_req(param, simple_desc_callback);
}