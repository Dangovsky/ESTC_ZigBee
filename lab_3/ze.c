/***************************************************************************
*                      ZBOSS ZigBee Pro 2007 stack                         *
*                                                                          *
*          Copyright (c) 2012 DSR Corporation Denver CO, USA.              *
*                       http://www.dsr-wireless.com                        *
*                                                                          *
*                            All rights reserved.                          *
*          Copyright (c) 2011 ClarIDy Solutions, Inc., Taipei, Taiwan.     *
*                       http://www.claridy.com/                            *
*                                                                          *
*          Copyright (c) 2011 Uniband Electronic Corporation (UBEC),       *
*                             Hsinchu, Taiwan.                             *
*                       http://www.ubec.com.tw/                            *
*                                                                          *
*          Copyright (c) 2011 DSR Corporation Denver CO, USA.              *
*                       http://www.dsr-wireless.com                        *
*                                                                          *
*                            All rights reserved.                          *
*                                                                          *
*                                                                          *
* ZigBee Pro 2007 stack, also known as ZBOSS (R) ZB stack is available     *
* under either the terms of the Commercial License or the GNU General      *
* Public License version 2.0.  As a recipient of ZigBee Pro 2007 stack, you*
* may choose which license to receive this code under (except as noted in  *
* per-module LICENSE files).                                               *
*                                                                          *
* ZBOSS is a registered trademark of DSR Corporation AKA Data Storage      *
* Research LLC.                                                            *
*                                                                          *
* GNU General Public License Usage                                         *
* This file may be used under the terms of the GNU General Public License  *
* version 2.0 as published by the Free Software Foundation and appearing   *
* in the file LICENSE.GPL included in the packaging of this file.  Please  *
* review the following information to ensure the GNU General Public        *
* License version 2.0 requirements will be met:                            *
* http://www.gnu.org/licenses/old-licenses/gpl-2.0.html.                   *
*                                                                          *
* Commercial Usage                                                         *
* Licensees holding valid ClarIDy/UBEC/DSR Commercial licenses may use     *
* this file in accordance with the ClarIDy/UBEC/DSR Commercial License     *
* Agreement provided with the Software or, alternatively, in accordance    *
* with the terms contained in a written agreement between you and          *
* ClarIDy/UBEC/DSR.                                                        *
*                                                                          *
****************************************************************************
PURPOSE: Test for ZC application written using ZDO.
*/


#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_bufpool.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "zb_zdo.h"
#include "zb_secur.h"
#include "zb_secur_api.h"

#include "led.h"

#ifndef ZB_ED_ROLE
#error define ZB_ED_ROLE to compile ze tests
#endif
/*! \addtogroup ZB_TESTS */
/*! @{ */
#ifndef ZB_SECURITY
#error Define ZB_SECURITY
#endif

commands_t command_cntr = ON_COMMAND;

void test_send_data(zb_uint8_t param) ZB_CALLBACK;
void test_get_buffer(zb_uint8_t param) ZB_CALLBACK;

void send_payloaded_command(zb_uint8_t param, zb_uint8_t payload_length, zb_uint8_t* payload);

void bulb_send_on_command(zb_uint8_t param) ZB_CALLBACK;
void bulb_send_off_command(zb_uint8_t param) ZB_CALLBACK;
void bulb_send_toggle_command(zb_uint8_t param) ZB_CALLBACK;
void bulb_send_brightness_up_command(zb_uint8_t param) ZB_CALLBACK;
void bulb_send_brightness_down_command(zb_uint8_t param) ZB_CALLBACK;
void bulb_send_brightness_command(zb_uint8_t param) ZB_CALLBACK;
void bulb_send_color_command(zb_uint8_t param) ZB_CALLBACK;

zb_ieee_addr_t g_ieee_addr = {0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02};

MAIN()
{
  ARGV_UNUSED;

#ifndef KEIL
  if ( argc < 3 )
  {
    printf("%s <read pipe path> <write pipe path>\n", argv[0]);
    return 0;
  }
#endif

  /* Init device, load IB values from nvram or set it to default */
#ifndef ZB8051
  ZB_INIT("zdo_ze", argv[1], argv[2]);
#else
  ZB_INIT("zdo_ze", "3", "3");
#endif

  ZB_IEEE_ADDR_COPY(ZB_PIB_EXTENDED_ADDRESS(), g_ieee_addr);

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


void zb_zdo_startup_complete(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  if (buf->u.hdr.status == 0)
  {
    TRACE_MSG(TRACE_APS1, "Device STARTED OK", (FMT__0));
    command_cntr = ON_COMMAND;
    ZB_SCHEDULE_ALARM(test_get_buffer, param, ZB_TIME_ONE_SECOND);
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "Device started FAILED status %d", (FMT__D, (int)buf->u.hdr.status));
    zb_free_buf(buf);
  }
}

void send_payloaded_command(zb_uint8_t param, zb_uint8_t payload_length, zb_uint8_t* payload)
{
    zb_buf_t *buf = (zb_buf_t*)ZB_BUF_FROM_REF(param);
    zb_uint8_t* ptr;
    zb_uint16_t tail = *((zb_uint16_t *)ZB_GET_BUF_TAIL(buf, sizeof(zb_uint16_t)));
    zb_apsde_data_req_t *req = ZB_GET_BUF_TAIL(buf, sizeof(zb_apsde_data_req_t));

    ZB_BUF_INITIAL_ALLOC(buf, ((sizeof *payload) * payload_length), ptr);

    req->dst_addr.addr_short = tail; 
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

void test_get_buffer(zb_uint8_t param) ZB_CALLBACK
{
    if (param)
    {
        test_send_data(param);
    }
    else
    {
        ZB_GET_OUT_BUF_DELAYED(test_send_data);
    }
    ZB_SCHEDULE_ALARM(test_get_buffer, 0, ZB_TIME_ONE_SECOND);
}

void test_send_data(zb_uint8_t param) ZB_CALLBACK
{          
    zb_buf_t *buf = (zb_buf_t*)ZB_BUF_FROM_REF(param);
    if (command_cntr != BRIGHTNESS_COMMAND)
    {
        zb_uint16_t* tail = ZB_GET_BUF_TAIL(buf, sizeof(zb_uint16_t));
        *tail = 0;
    }
    else 
    {
        bulb_tail_t* tail = ZB_GET_BUF_TAIL(buf, sizeof(bulb_tail_t));
        tail->addr = 0;
        tail->brightness = 0xda;
    }

    switch(command_cntr)
    {
        case ON_COMMAND:
            ZB_SCHEDULE_CALLBACK(bulb_send_on_command, param);
            break;
        case OFF_COMMAND:
            ZB_SCHEDULE_CALLBACK(bulb_send_off_command, param);
            break;
        case TOGGLE_COMMAND:
            ZB_SCHEDULE_CALLBACK(bulb_send_toggle_command, param);
            break;
        case BRIGHTNESS_DOWN_COMMAND:
            ZB_SCHEDULE_CALLBACK(bulb_send_brightness_down_command, param);
            break;
        case BRIGHTNESS_UP_COMMAND:
            ZB_SCHEDULE_CALLBACK(bulb_send_brightness_up_command, param);
            break;
        case BRIGHTNESS_COMMAND:
            ZB_SCHEDULE_CALLBACK(bulb_send_brightness_command, param);
            break;
        case COLOR_COMMAND:
            ZB_SCHEDULE_CALLBACK(bulb_send_color_command, param);
            break;
    }

    if (++command_cntr > COLOR_COMMAND)
    {
        command_cntr = ON_COMMAND;
    }
}

/*! @} */
