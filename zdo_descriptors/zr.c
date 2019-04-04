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


/*! \addtogroup ZB_TESTS */
/*! @{ */

static zb_uint8_t cnt = 0;

void zdo_desc_ieee_addr(zb_uint8_t param) ZB_CALLBACK;
void ieee_addr_callback(zb_uint8_t param) ZB_CALLBACK;
void zdo_desc_active_ep(zb_uint8_t param) ZB_CALLBACK;
void active_ep_callback(zb_uint8_t param) ZB_CALLBACK;
void zdo_simple_desk(zb_uint8_t param) ZB_CALLBACK;
void simple_desc_callback(zb_uint8_t param) ZB_CALLBACK;

void send_data(zb_uint8_t param) ZB_CALLBACK;
void get_buffer(zb_uint8_t param) ZB_CALLBACK;

void send_command(zb_uint8_t param) ZB_CALLBACK;
/*
  ZR joins to ZC, then sends APS packet.
 */
 
zb_ieee_addr_t g_zr_addr = {0x01, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb};

MAIN()
{
  ARGV_UNUSED;

#if !(defined KEIL || defined SDCC|| defined ZB_IAR)
  if ( argc < 3 )
  {
    //printf("%s <read pipe path> <write pipe path>\n", argv[0]);
    return 0;
  }
#endif

  /* Init device, load IB values from nvram or set it to default */
#ifndef ZB8051
  ZB_INIT("zdo_zr", argv[1], argv[2]);
#else
  ZB_INIT("zdo_zr", "2", "2");
#endif
#ifdef ZB_SECURITY
  ZG->nwk.nib.security_level = 0;
#endif

  ZB_AIB().aps_channel_mask = (1l << 22); 
  ZB_IEEE_ADDR_COPY(ZB_PIB_EXTENDED_ADDRESS(), &g_zr_addr); 

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
    cnt = 0;
    get_buffer(param);
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "Device started FAILED status %d", (FMT__D, (int)buf->u.hdr.status));
    zb_free_buf(buf);
  }
}

void get_buffer(zb_uint8_t param) ZB_CALLBACK
{
  if (param != 0)
  {
      send_data(param);
  }
  ZB_GET_OUT_BUF_DELAYED(send_data);
  ZB_SCHEDULE_ALARM(get_buffer, 0, ZB_TIME_ONE_SECOND);
}

void send_data(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t* buf = (zb_buf_t*)ZB_BUF_FROM_REF(param);
  zb_uint16_t* tail;
  zb_zdo_simple_desc_req_t *req; 
  switch (cnt)
  {
    case 0:
        tail = ZB_GET_BUF_TAIL(buf, sizeof(zb_uint16_t));
        *tail = 0;
        zdo_desc_ieee_addr(param);
        break;
    case 1:
        tail = ZB_GET_BUF_TAIL(buf, sizeof(zb_uint16_t));
        *tail = 0;
        zdo_desc_active_ep(param);
        break;
    case 2:
        req = ZB_GET_BUF_TAIL(buf, sizeof(zb_zdo_simple_desc_req_t));
        req->nwk_addr = 0;
        req->endpoint = 11;
        zdo_simple_desk(param);
        break;
    default:
        tail = ZB_GET_BUF_TAIL(buf, sizeof(zb_uint16_t));
        *tail = 0;
        send_command(param);
        break;
  }
  ++cnt;
}

void send_command(zb_uint8_t param) ZB_CALLBACK
{
    zb_buf_t *buf = (zb_buf_t*)ZB_BUF_FROM_REF(param);
    zb_uint16_t *ptr;
    zb_uint16_t addr = *((zb_uint16_t *)ZB_GET_BUF_TAIL(buf, sizeof(zb_uint16_t)));
    zb_apsde_data_req_t *req = ZB_GET_BUF_TAIL(buf, sizeof(zb_apsde_data_req_t));

    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_uint16_t), ptr);
    req->dst_addr.addr_short = addr;
    req->addr_mode = ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
    req->tx_options = ZB_APSDE_TX_OPT_ACK_TX;
    req->radius = 1;
    req->profileid = 2;
    req->clusterid = 0xaaaa;
    req->src_endpoint = 10;
    req->dst_endpoint = 11;

    buf->u.hdr.handle = 0x11;

    *ptr = 0x0101;
  
    TRACE_MSG(TRACE_APS3, "Sending apsde_data.request", (FMT__0));
    ZB_SCHEDULE_CALLBACK(zb_apsde_data_request, ZB_REF_FROM_BUF(buf));
}

/* IEEE */ 
void zdo_desc_ieee_addr(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = (zb_buf_t*)ZB_BUF_FROM_REF(param);
  zb_zdo_ieee_addr_req_t *req = NULL;
  zb_uint16_t addr = *((zb_uint16_t *)ZB_GET_BUF_TAIL(buf, sizeof(zb_uint16_t)));
  
  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_ieee_addr_req_t), req);

  req->nwk_addr = addr;
  req->request_type = ZB_ZDO_SINGLE_DEV_RESPONSE;
  req->start_index = 0;
  zb_zdo_ieee_addr_req(ZB_REF_FROM_BUF(buf), ieee_addr_callback);
}

void ieee_addr_callback(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_zdo_nwk_addr_resp_head_t *resp;
  zb_ieee_addr_t ieee_addr;
  zb_uint16_t nwk_addr;
  
  TRACE_MSG(TRACE_ZDO2, "zb_get_peer_short_addr_cb param %hd", (FMT__H, param));
  
  resp = (zb_zdo_nwk_addr_resp_head_t*)ZB_BUF_BEGIN(buf);
  
  ZB_LETOH64(ieee_addr, resp->ieee_addr);
  ZB_LETOH16(&nwk_addr, &resp->nwk_addr);
  
  TRACE_MSG(TRACE_ZDO2, "resp status %hd, nwk addr %d", (FMT__H_D, resp->status, nwk_addr));
  ZB_DUMP_IEEE_ADDR(ieee_addr); 
  
  zb_free_buf(buf);
}

/* active ep */
void zdo_desc_active_ep(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = (zb_buf_t*)ZB_BUF_FROM_REF(param);
  zb_zdo_active_ep_req_t *req;
  zb_uint16_t addr = *((zb_uint16_t *)ZB_GET_BUF_TAIL(buf, sizeof(zb_uint16_t)));

  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_active_ep_req_t), req);
  req->nwk_addr = addr;
  zb_zdo_active_ep_req(ZB_REF_FROM_BUF(buf), active_ep_callback);
} 

void active_ep_callback(zb_uint8_t param) ZB_CALLBACK 
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_uint8_t *zdp_cmd = ZB_BUF_BEGIN(buf);
  zb_zdo_ep_resp_t *resp = (zb_zdo_ep_resp_t*)zdp_cmd;
  zb_uint8_t *ep_list = zdp_cmd + sizeof(zb_zdo_ep_resp_t);

  TRACE_MSG(TRACE_APS1, "active_ep_callback status %hd, addr 0x%x",
            (FMT__H, resp->status, resp->nwk_addr));

  if (resp->status != ZB_ZDP_STATUS_SUCCESS || resp->nwk_addr != 0x0)
  {
    TRACE_MSG(TRACE_APS1, "Error incorrect status/addr", (FMT__0));
  }

  TRACE_MSG(TRACE_APS1, " ep count %hd, ep %hd", (FMT__H_H, resp->ep_count, *ep_list));
  
  zb_free_buf(buf);
}

/* simple discriptor */
void zdo_simple_desk(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = (zb_buf_t*)ZB_BUF_FROM_REF(param);
  zb_zdo_simple_desc_req_t *req_tail = ZB_GET_BUF_TAIL(buf, sizeof(zb_zdo_simple_desc_req_t));
  zb_zdo_simple_desc_req_t *req;

  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_simple_desc_req_t), req);
  req->nwk_addr = req_tail->nwk_addr;
  req->endpoint = req_tail->endpoint;
  zb_zdo_simple_desc_req(ZB_REF_FROM_BUF(buf), simple_desc_callback);
}

void simple_desc_callback(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_uint8_t *zdp_cmd = ZB_BUF_BEGIN(buf);
  zb_zdo_simple_desc_resp_t *resp = (zb_zdo_simple_desc_resp_t*)(zdp_cmd);
  zb_uint_t i; 

  TRACE_MSG(TRACE_APS1, "simple_desc_callback status %hd, addr 0x%x",
            (FMT__H_D, resp->hdr.status, resp->hdr.nwk_addr));
  if (resp->hdr.status != ZB_ZDP_STATUS_SUCCESS || resp->hdr.nwk_addr != 0x0)
  {
    TRACE_MSG(TRACE_APS1, "Error incorrect status/addr", (FMT__0));
  }

  TRACE_MSG(TRACE_APS1, "ep %d, app prof %d, dev id %d, dev ver %d, input count 0x%x",
            (FMT__D_D_D_D_D, resp->simple_desc.endpoint, resp->simple_desc.app_profile_id,
            resp->simple_desc.app_device_id, resp->simple_desc.app_device_version,
           resp->simple_desc.app_input_cluster_count));

  zb_uint8_t *cnt_ptr = &(resp->simple_desc.app_input_cluster_count);
  zb_uint16_t *ptr = (zb_uint16_t*)(cnt_ptr + 1); 
  
  TRACE_MSG(TRACE_APS1, "input clusters:", (FMT__0));
  for(i = 0; i < *(cnt_ptr); i++)
  {
    TRACE_MSG(TRACE_APS1, " 0x%x", (FMT__D, *ptr));
    ++ptr;
  }
  
  cnt_ptr = (zb_uint8_t*)ptr;
  ptr = (zb_uint16_t*)(cnt_ptr + 1);
  
  TRACE_MSG(TRACE_APS1, "otput cluster count %x. output clusters:", (FMT__D, *(cnt_ptr)));
  for(i = 0; i < *(cnt_ptr); i++)
  {
    TRACE_MSG(TRACE_APS1, " 0x%x", (FMT__D, *ptr));
    ++ptr;
  }
  
  zb_free_buf(buf);
}

/*! @} */
