#include "zb_common.h"
#include "zb_aps.h"
#include "zb_bufpool.h"
#include "zb_nwk.h"
#include "zb_mac.h"
#include "zb_scheduler.h"
#include "zb_zdo.h"
#include "stm32f4xx.h"

#include "./zdo_console.c"

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

void zb_zdo_startup_complete(zb_uint8_t param) ZB_CALLBACK {
    zb_buf_t *buf = ZB_BUF_FROM_REF(param);
    TRACE_MSG(TRACE_APS3, ">>zb_zdo_startup_complete status %d", (FMT__D, (int)buf->u.hdr.status));
    if (buf->u.hdr.status == 0) {
        TRACE_MSG(TRACE_APS1, "Device STARTED OK", (FMT__0));
        init_console();
    } else {
        TRACE_MSG(TRACE_ERROR, "Device start FAILED status %d", (FMT__D, (int)buf->u.hdr.status));
    }
    zb_free_buf(buf);
}
