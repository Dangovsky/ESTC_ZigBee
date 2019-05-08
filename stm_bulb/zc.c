#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_bufpool.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "zb_zdo.h"

#include "./libzbulb/include/zbulb.h"
#include "./libled/include/led.h"

#define COLORS_CNT 10
#define BRIGHTNESS_STEP 25

/* current brightness */
volatile uint8_t brightness = 255;
/* bulb state flag */
volatile uint8_t is_on = 0;
/* index in colors array */
volatile uint8_t current_color = 0;
/* colors from https://simpledits.com/top-12-pantone-colors-for-spring-2018-with-hex-cmyk-and-rgb-values/ */
static uint32_t colors[COLORS_CNT] = {0xecdb54, 0xe34132, 0x6ca0dc, 0x944743, 0xdbb2d1, 
                                     0xec9787, 0x00a68c, 0x645394, 0x6c4f3d, 0xebe1df};

zb_ieee_addr_t g_zc_addr = {0x00, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};


#ifndef ZB_COORDINATOR_ROLE
#error Coordinator role is not compiled!
#endif

MAIN()
{
  ARGV_UNUSED;

#if !(defined KEIL || defined SDCC || defined ZB_IAR)
  if ( argc < 3 )
  {
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

void bulb_receive_toggle_command(zb_uint8_t param) ZB_CALLBACK
{
    is_on = !is_on;
    if (is_on)
    {
        led_set_color_hex(((zb_uint32_t)brightness << 24) | colors[current_color]);
    }
    else
    {
        led_set_color_hex(0);
    }
}

void bulb_receive_brightness_up_command(zb_uint8_t param) ZB_CALLBACK
{
    brightness += BRIGHTNESS_STEP;
    led_set_color_hex(((zb_uint32_t)brightness << 24) | colors[current_color]);
}

void bulb_receive_toggle_color_command(zb_uint8_t param) ZB_CALLBACK
{
    ++current_color;
    if (current_color> COLORS_CNT)
    {
        current_color = 0;
    }
    led_set_color_hex(((zb_uint32_t)brightness << 24) | colors[current_color]);
}

void zb_zdo_startup_complete(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  bulb_handlers_t handlers = {0};

  TRACE_MSG(TRACE_APS3, ">>zb_zdo_startup_complete status %d", (FMT__D, (int)buf->u.hdr.status));
  if (buf->u.hdr.status == 0)
  {
    TRACE_MSG(TRACE_APS1, "Device STARTED OK", (FMT__0));

    init_led();
    handlers.bulb_receive_toggle_command = bulb_receive_toggle_command;
    handlers.bulb_receive_brightness_up_command = bulb_receive_brightness_up_command;
    handlers.bulb_receive_toggle_color_command = bulb_receive_toggle_color_command;
    init_zbulb(&handlers);
    zb_af_set_data_indication(bulb_parse_packet);
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "Device start FAILED status %d", (FMT__D, (int)buf->u.hdr.status));
  }
  zb_free_buf(buf);
}
