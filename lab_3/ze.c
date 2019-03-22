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

commands_t command_cntr = ON;

void test_send_data(zb_uint8_t param) ZB_CALLBACK;
void test_get_buffer(zb_uint8_t param) ZB_CALLBACK;

void send_payloaded_command(zb_uint8_t param, commands_t command);
void send_command(zb_uint8_t param, commands_t command);

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
    command_cntr = ON;
    test_send_data(param);
    ZB_SCHEDULE_ALARM(test_get_buffer, 0, ZB_TIME_ONE_SECOND);
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "Device started FAILED status %d", (FMT__D, (int)buf->u.hdr.status));
    zb_free_buf(buf);
  }
}

void send_command(zb_uint8_t param, commands_t command)
{
    zb_buf_t *buf = (zb_buf_t*)ZB_BUF_FROM_REF(param);
    commands_t *ptr;
    zb_uint16_t addr = *((zb_uint16_t *)ZB_GET_BUF_TAIL(buf, sizeof(zb_uint16_t)));
    zb_apsde_data_req_t *req = ZB_GET_BUF_TAIL(buf, sizeof(zb_apsde_data_req_t));

    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_uint8_t), ptr); /* if use (sizeof command), we'll have 3 empty bytes*/
    req->dst_addr.addr_short = addr; 
    req->addr_mode = ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
    req->tx_options = ZB_APSDE_TX_OPT_ACK_TX;
    req->radius = 1;
    req->profileid = 2;
    req->src_endpoint = 10;
    req->dst_endpoint = 10;

    buf->u.hdr.handle = 0x11;

    *ptr = (zb_uint8_t)command; /* same purpose */
  
    TRACE_MSG(TRACE_APS3, "Sending apsde_data.request, command: %x", (FMT__D_D, command));
    ZB_SCHEDULE_CALLBACK(zb_apsde_data_request, ZB_REF_FROM_BUF(buf));
}

void send_payloaded_command(zb_uint8_t param,  commands_t command) ZB_CALLBACK
{
    zb_buf_t *buf = (zb_buf_t*)ZB_BUF_FROM_REF(param);
    bulb_send_payload_t* ptr;
    bulb_addr_payload_t tail = *((bulb_addr_payload_t *)ZB_GET_BUF_TAIL(buf, sizeof(bulb_addr_payload_t)));
    zb_apsde_data_req_t *req = ZB_GET_BUF_TAIL(buf, sizeof(zb_apsde_data_req_t));

    ZB_BUF_INITIAL_ALLOC(buf, sizeof(bulb_send_payload_t), ptr);
    req->dst_addr.addr_short = tail.addr; 
    req->addr_mode = ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
    req->tx_options = ZB_APSDE_TX_OPT_ACK_TX;
    req->radius = 1;
    req->profileid = 2;
    req->src_endpoint = 10;
    req->dst_endpoint = 10;

    buf->u.hdr.handle = 0x11;

    ptr->command = command;
    ptr->payload = tail.payload;
  
    TRACE_MSG(TRACE_APS3, "Sending apsde_data.request, command: %x, payload: %x", (FMT__D_D, command, tail.payload));
    ZB_SCHEDULE_CALLBACK(zb_apsde_data_request, ZB_REF_FROM_BUF(buf));
}

void bulb_send_on_command(zb_uint8_t param) ZB_CALLBACK
{
    send_command(param, ON);
}

void bulb_send_off_command(zb_uint8_t param) ZB_CALLBACK
{
    send_command(param, OFF);
}

void bulb_send_toggle_command(zb_uint8_t param) ZB_CALLBACK
{
    send_command(param, TOGGLE);
}

void bulb_send_brightness_up_command(zb_uint8_t param) ZB_CALLBACK
{
    send_command(param, BRIGHTNESS_UP);
}

void bulb_send_brightness_down_command(zb_uint8_t param) ZB_CALLBACK
{
    send_command(param, BRIGHTNESS_DOWN);
}

void bulb_send_brightness_command(zb_uint8_t param) ZB_CALLBACK
{
    send_payloaded_command(param, BRIGHTNESS);
}

void bulb_send_color_command(zb_uint8_t param) ZB_CALLBACK
{
    send_payloaded_command(param, COLOR);
}

void test_get_buffer(zb_uint8_t param) ZB_CALLBACK
{
    ZB_GET_OUT_BUF_DELAYED(test_send_data);
    ZB_SCHEDULE_ALARM(test_get_buffer, 0, ZB_TIME_ONE_SECOND);
}

void test_send_data(zb_uint8_t param) ZB_CALLBACK
{          
    zb_buf_t *buf = (zb_buf_t*)ZB_BUF_FROM_REF(param);
    if (command_cntr < 5)
    {
        zb_uint16_t* tail = ZB_GET_BUF_TAIL(buf, sizeof(zb_uint16_t));
        *tail = 0;
    }
    else 
    {
        bulb_addr_payload_t* tail = ZB_GET_BUF_TAIL(buf, sizeof(bulb_addr_payload_t));
        tail->addr = 0;
        tail->payload = 0xda;
    }

    switch(command_cntr)
    {
        case ON:
            ZB_SCHEDULE_CALLBACK(bulb_send_on_command, param);
            break;
        case OFF:
            ZB_SCHEDULE_CALLBACK(bulb_send_off_command, param);
            break;
        case TOGGLE:
            ZB_SCHEDULE_CALLBACK(bulb_send_toggle_command, param);
            break;
        case BRIGHTNESS_DOWN:
            ZB_SCHEDULE_CALLBACK(bulb_send_brightness_down_command, param);
            break;
        case BRIGHTNESS_UP:
            ZB_SCHEDULE_CALLBACK(bulb_send_brightness_up_command, param);
            break;
        case BRIGHTNESS:
            ZB_SCHEDULE_CALLBACK(bulb_send_brightness_command, param);
            break;
        case COLOR:
            ZB_SCHEDULE_CALLBACK(bulb_send_color_command, param);
            break;
    }

    if (++command_cntr > COLOR)
    {
        command_cntr = ON;
    }
}

/*! @} */
