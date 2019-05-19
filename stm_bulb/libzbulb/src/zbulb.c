#include "../include/zbulb.h"

bulb_handlers_t bulb_handlers = {0};

void init_zbulb(bulb_handlers_t *handlers) {
    bulb_handlers = *handlers;
}

void bulb_parse_packet(zb_uint8_t param) ZB_CALLBACK {
    zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
    bulb_payload_t *payload;

    /* Remove APS header from the packet */
    ZB_APS_HDR_CUT_P(buf, payload);

    if (ZB_BUF_LEN(buf) < sizeof(zb_uint8_t)) {
        TRACE_MSG(TRACE_APS2, "received too small packet", (FMT__0));
        zb_free_buf(buf);
        return;
    }

    if ((payload->command == BRIGHTNESS_COMMAND) && (ZB_BUF_LEN(buf) < sizeof(bulb_payload_t))) {
        TRACE_MSG(TRACE_APS2, "received brightness command without brightness payloaded", (FMT__0));
        zb_free_buf(buf);
        return;
    }

    TRACE_MSG(TRACE_APS2, "apsde_data_indication: packet %p len %d handle 0x%x command: %x", (FMT__P_D_D_D, param, (int)ZB_BUF_LEN(asdu), asdu->u.hdr.status, payload->command));

    switch (payload->command) {
        case ON_COMMAND:
            TRACE_MSG(TRACE_APS2, "received on command", (FMT__0));
            if (bulb_handlers.bulb_receive_on_command != NULL) {
                bulb_handlers.bulb_receive_on_command(param);
            } else {
                zb_free_buf(buf);
            }
            break;
        case OFF_COMMAND:
            TRACE_MSG(TRACE_APS2, "received off command", (FMT__0));
            if (bulb_handlers.bulb_receive_off_command != NULL) {
                bulb_handlers.bulb_receive_off_command(param);
            } else {
                zb_free_buf(buf);
            }
            break;
        case TOGGLE_COMMAND:
            TRACE_MSG(TRACE_APS2, "received toggle command", (FMT__0));
            if (bulb_handlers.bulb_receive_toggle_command != NULL) {
                bulb_handlers.bulb_receive_toggle_command(param);
            } else {
                zb_free_buf(buf);
            }
            break;
        case BRIGHTNESS_UP_COMMAND:
            TRACE_MSG(TRACE_APS2, "received brightness_up command", (FMT__0));
            if (bulb_handlers.bulb_receive_brightness_up_command != NULL) {
                bulb_handlers.bulb_receive_brightness_up_command(param);
            } else {
                zb_free_buf(buf);
            }
            break;
        case BRIGHTNESS_DOWN_COMMAND:
            TRACE_MSG(TRACE_APS2, "received brightness_down command", (FMT__0));
            if (bulb_handlers.bulb_receive_brightness_down_command != NULL) {
                bulb_handlers.bulb_receive_brightness_down_command(param);
            } else {
                zb_free_buf(buf);
            }
            break;
        case BRIGHTNESS_COMMAND:
            TRACE_MSG(TRACE_APS2, "received brightness command. brightness: %x", (FMT__D, payload->brightness));
            if (bulb_handlers.bulb_receive_brightness_command != NULL) {
                bulb_handlers.bulb_receive_brightness_command(param);
            } else {
                zb_free_buf(buf);
            }
            break;
        case TOGGLE_COLOR_COMMAND:
            TRACE_MSG(TRACE_APS2, "received color command.", (FMT__0));
            if (bulb_handlers.bulb_receive_toggle_color_command != NULL) {
                bulb_handlers.bulb_receive_toggle_color_command(param);
            } else {
                zb_free_buf(buf);
            }
            break;
        default:
            TRACE_MSG(TRACE_APS2, "received unknown command", (FMT__0));
            zb_free_buf(buf);
            break;
    }
}

static void send_payloaded_command(zb_uint8_t param, zb_uint8_t payload_length, zb_uint8_t *payload) {
    zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
    zb_uint8_t *ptr;
    zb_uint16_t address = *((zb_uint16_t *)ZB_GET_BUF_TAIL(buf, sizeof(zb_uint16_t)));
    zb_apsde_data_req_t *req = ZB_GET_BUF_TAIL(buf, sizeof(zb_apsde_data_req_t));

    ZB_BUF_INITIAL_ALLOC(buf, ((sizeof *payload) * payload_length), ptr);

    req->dst_addr.addr_short = address;
    req->addr_mode = ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
    req->tx_options = ZB_APSDE_TX_OPT_ACK_TX;
    req->radius = 1;
    req->profileid = 0x1234;
    req->clusterid = 0x1234;
    req->src_endpoint = 10;
    req->dst_endpoint = 10;

    buf->u.hdr.handle = 0x11;

    memcpy(ptr, payload, payload_length);

    if (payload_length > 1) {
        TRACE_MSG(TRACE_APS3, "Sending apsde_data.request, command: %x, payload: %s", (FMT__D_D, *payload, payload + 1));
    } else {
        TRACE_MSG(TRACE_APS3, "Sending apsde_data.request, command: %x ", (FMT__D, *payload));
    }

    ZB_SCHEDULE_CALLBACK(zb_apsde_data_request, ZB_REF_FROM_BUF(buf));
}

void bulb_send_on_command(zb_uint8_t param) ZB_CALLBACK {
    zb_uint8_t command = ON_COMMAND;
    send_payloaded_command(param, 1, &command);
}

void bulb_send_off_command(zb_uint8_t param) ZB_CALLBACK {
    zb_uint8_t command = OFF_COMMAND;
    send_payloaded_command(param, 1, &command);
}

void bulb_send_toggle_command(zb_uint8_t param) ZB_CALLBACK {
    zb_uint8_t command = TOGGLE_COMMAND;
    send_payloaded_command(param, 1, &command);
}

void bulb_send_brightness_up_command(zb_uint8_t param) ZB_CALLBACK {
    zb_uint8_t command = BRIGHTNESS_UP_COMMAND;
    send_payloaded_command(param, 1, &command);
}

void bulb_send_brightness_down_command(zb_uint8_t param) ZB_CALLBACK {
    zb_uint8_t command = BRIGHTNESS_DOWN_COMMAND;
    send_payloaded_command(param, 1, &command);
}

void bulb_send_brightness_command(zb_uint8_t param) ZB_CALLBACK {
    bulb_tail_t *tail = ZB_GET_BUF_TAIL((zb_buf_t *)ZB_BUF_FROM_REF(param), sizeof(bulb_tail_t));
    zb_uint8_t payload[2];
    payload[0] = BRIGHTNESS_COMMAND;
    payload[1] = tail->brightness;
    send_payloaded_command(param, 2, payload);
}

void bulb_send_toggle_color_command(zb_uint8_t param) ZB_CALLBACK {
    zb_uint8_t command = TOGGLE_COLOR_COMMAND;
    send_payloaded_command(param, 1, &command);
}
