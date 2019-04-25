#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_bufpool.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "zb_zdo.h"

#include "./libzbulb/include/zbulb.h"
#define ZB_ALARMS
#include "./libbuttons/include/buttons.h"

zb_uint16_t addr = 0;
zb_uint8_t brightness = 255;

#ifndef ZB_ED_ROLE
#error define ZB_ED_ROLE to compile ze tests
#endif
/*! \addtogroup ZB_TESTS */
/*! @{ */

zb_ieee_addr_t g_ze_addr = {0x02, 0xed, 0xed, 0xed, 0xed, 0xed, 0xed, 0xed};

MAIN()
{
  ARGV_UNUSED;

#if !(defined KEIL || defined SDCC || defined ZB_IAR )
  if ( argc < 3 )
  {
    //printf("%s <read pipe path> <write pipe path>\n", argv[0]);
    return 0;
  }
#endif

  /* Init device, load IB values from nvram or set it to default */
#ifndef ZB8051
  ZB_INIT("zdo_ze", argv[1], argv[2]);
#else
  ZB_INIT((char*)"zdo_ze", (char*)"3", (char*)"3");
#endif
#ifdef ZB_SECURITY
  ZG->nwk.nib.security_level = 0;
#endif
  ZB_IEEE_ADDR_COPY(ZB_PIB_EXTENDED_ADDRESS(), &g_ze_addr);
  ZB_PIB_RX_ON_WHEN_IDLE() = ZB_FALSE;
  ZB_AIB().aps_channel_mask = (1l << 22);
	
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

zb_uint8_t prepare_buf(void)
{
    zb_buf_t buf = zb_get_out_buf();
    bulb_tail_t* tail = ZB_GET_BUF_TAIL(buf, sizeof(bulb_tail_t));
    tail->brightness = 0;
    tail->addr = addr;
    return ZB_REF_FROM_BUF(buf);
}

void button_first_click(zb_uint8_t param) ZB_CALLBACK
{
    bulb_send_toggle_command(ZB_BUF_FROM_REF(prepare_buf()));
}

void button_second_click(zb_uint8_t param) ZB_CALLBACK
{
    bulb_send_brightness_up_command(ZB_BUF_FROM_REF(prepare_buf()));
}

void button_both_click(zb_uint8_t param) ZB_CALLBACK
{
    bulb_send_color_command(ZB_BUF_FROM_REF(prepare_buf()));
}

void zb_zdo_startup_complete(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  if (buf->u.hdr.status == 0)
  {
    TRACE_MSG(TRACE_APS1, "Device STARTED OK", (FMT__0));
    init_buttons();
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "Device started FAILED status %d", (FMT__D, (int)buf->u.hdr.status));
    zb_free_buf(buf);
  }
}

/*! @} */
