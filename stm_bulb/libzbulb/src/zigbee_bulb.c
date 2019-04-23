#include "../include/zigbee_bulb.h"

void bulb_parce_packet(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *asdu = (zb_buf_t *)ZB_BUF_FROM_REF(param);
  zb_uint8_t* payload;

  /* Remove APS header from the packet */
  ZB_APS_HDR_CUT_P(asdu, payload);

  if (ZB_BUF_LEN(asdu) < sizeof(zb_uint8_t))
  {
      TRACE_MSG(TRACE_APS2, "recieved too small packet", (FMT__0));
      zb_free_buf(asdu);
      return;
  }

  if ((*payload == BRIGHTNESS_COMMAND) && (ZB_BUF_LEN(asdu) < sizeof(zb_uint8_t) * 2))
  {
      TRACE_MSG(TRACE_APS2, "recieved brightness packet without payload", (FMT__0));
      zb_free_buf(asdu);
      return;
  }

  TRACE_MSG(TRACE_APS2, "apsde_data_indication: packet %p len %d handle 0x%x command: %x", (FMT__P_D_D_D,
                         asdu, (int)ZB_BUF_LEN(asdu), asdu->u.hdr.status, *payload));

  switch (*payload)
  {
      case ON_COMMAND:
          TRACE_MSG(TRACE_APS2, "recieved on command", (FMT__0));
          bulb_receive_on_command(0);
          break;
      case OFF_COMMAND:
          TRACE_MSG(TRACE_APS2, "recieved off command", (FMT__0));
          bulb_recieve_off_command(0);
          break;
      case TOGGLE_COMMAND:
          TRACE_MSG(TRACE_APS2, "recieved toggle command", (FMT__0));
          bulb_recieve_toggle_command(0);
          break;
      case BRIGHTNESS_UP_COMMAND:
          TRACE_MSG(TRACE_APS2, "recieved brightness_up command", (FMT__0));
          bulb_recieve_brigtness_up_command(0);
          break;
      case BRIGHTNESS_DOWN_COMMAND:
          TRACE_MSG(TRACE_APS2, "recieved brightness_down command", (FMT__0));
          bulb_recieve_brigtness_down_command(0);
          break;
      case BRIGHTNESS_COMMAND:
          TRACE_MSG(TRACE_APS2, "recieved brightness command. brightness: %x", (FMT__D, *(payload+1)));
          bulb_recieve_brigtness_command(*(payload+1));
          break;
      case COLOR_COMMAND:
          TRACE_MSG(TRACE_APS2, "recieved color command.", (FMT__0));
          bulb_recieve_color_command(0)
          break;
      default:
          TRACE_MSG(TRACE_APS2, "recieved unknown command", (FMT__0));
          break;
  }
  zb_free_buf(asdu);
}

void __attribute__((weak)) bulb_receive_on_command(zb_uint8_t param) ZB_CALLBACK {}
void __attribute__((weak)) bulb_receive_off_command(zb_uint8_t param) ZB_CALLBACK {}
void __attribute__((weak)) bulb_receive_toggle_command(zb_uint8_t param) ZB_CALLBACK {}
void __attribute__((weak)) bulb_receive_brightness_up_command(zb_uint8_t param) ZB_CALLBACK {}
void __attribute__((weak)) bulb_receive_brightness_down_command(zb_uint8_t param) ZB_CALLBACK {}
void __attribute__((weak)) bulb_receive_brightness_command(zb_uint8_t param) ZB_CALLBACK {}
void __attribute__((weak)) bulb_receive_color_command(zb_uint8_t param) ZB_CALLBACK {}

static void send_payloaded_command(zb_uint8_t param, zb_uint8_t payload_length, zb_uint8_t* payload)
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
        TRACE_MSG(TRACE_APS3, "Sending apsde_data.request, command: %x, payload: %s", (FMT__D_D, *payload, payload+1));
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