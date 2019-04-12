#include "zigbee_bulb_input.h"

void bulb_parce_package(zb_uint8_t param) ZB_CALLBACK
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
          break;
      case OFF_COMMAND:
          TRACE_MSG(TRACE_APS2, "recieved off command", (FMT__0));
          break;
      case TOGGLE_COMMAND:
          TRACE_MSG(TRACE_APS2, "recieved toggle command", (FMT__0));
          break;
      case BRIGHTNESS_UP_COMMAND:
          TRACE_MSG(TRACE_APS2, "recieved brightness_up command", (FMT__0));
          break;
      case BRIGHTNESS_DOWN_COMMAND:
          TRACE_MSG(TRACE_APS2, "recieved brightness_down command", (FMT__0));
          break;
      case BRIGHTNESS_COMMAND:
          TRACE_MSG(TRACE_APS2, "recieved brightness command. brightness: %x", (FMT__D, *(payload+1)));
          break;
      case COLOR_COMMAND:
          TRACE_MSG(TRACE_APS2, "recieved color command.", (FMT__0));
          break;
      default:
          TRACE_MSG(TRACE_APS2, "recieved unknown command", (FMT__0));
          break;
  }
  zb_free_buf(asdu);
}


