#include "zigbee_bulb_output.h"

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
