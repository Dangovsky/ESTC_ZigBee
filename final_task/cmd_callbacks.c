#include "console.h"

/**
 * @brief ieee command callback
 *
 * Print IEEE address responce
 */
#ifdef IEEE_TEST
void ieee_addr_callback(zb_uint8_t param) ZB_CALLBACK {
    zb_uint8_t status = 15, nwk_addr = 10, ieee_addr[] = {12, 13, 14, 15, 16, 18, 19, 20};
    char str[100];
    sprintf(str, "\n\rget ieee address responce:\n\r\tstatus: %hd\n\r\tnwk address: %d\n\r",
            status, nwk_addr);
    print(str);
    sprintf(str, "\tieee address: " FORMAT_64,
            ARG_64(ieee_addr));
    print(str);

    on_return();
}
#else
void ieee_addr_callback(zb_uint8_t param) ZB_CALLBACK {
    zb_buf_t *buf = ZB_BUF_FROM_REF(param);
    zb_zdo_nwk_addr_resp_head_t *resp;
    zb_ieee_addr_t ieee_addr;
    zb_uint16_t nwk_addr;
    char str[100];

    resp = (zb_zdo_nwk_addr_resp_head_t *)ZB_BUF_BEGIN(buf);

    ZB_LETOH64(ieee_addr, resp->ieee_addr);
    ZB_LETOH16(&nwk_addr, &resp->nwk_addr);

    sprintf(str, "\n\rget ieee address responce:\n\r\tstatus: %hd\n\r\tnwk address: %d\n\r",
            resp->status, nwk_addr);
    print(str);
    sprintf(str, "\tieee address: " FORMAT_64,
            ARG_64(ieee_addr));
    print(str);

    zb_free_buf(buf);

    on_return();
}
#endif /* IEEE_TEST */

/**
 * @brief ep command callback
 *
 * Print active endpoints responce
 */
void active_ep_callback(zb_uint8_t param) ZB_CALLBACK {
    zb_buf_t *buf = ZB_BUF_FROM_REF(param);
    zb_uint8_t *zdp_cmd = ZB_BUF_BEGIN(buf);
    zb_zdo_ep_resp_t *resp = (zb_zdo_ep_resp_t *)zdp_cmd;
    zb_uint8_t *ep_list = zdp_cmd + sizeof(zb_zdo_ep_resp_t);
    char str[100];
    zb_uint8_t i;

    sprintf(str, "\n\rActive endpoint callback status: %hd, addr: 0x%x",
            resp->status, resp->nwk_addr);
    print(str);

    if (resp->status != ZB_ZDP_STATUS_SUCCESS) {
        print("\n\rError incorrect status");
    }

    sprintf(str, "\n\rep count: %hd\n\r", resp->ep_count);
    print(str);
    for (i = 0, print("\teps: "); i < resp->ep_count; ++i) {
        sprintf(str, "%d ", ep_list[i]);
        print(str);
    }

    zb_free_buf(buf);

    on_return();
}

/**
 * @brief simple command callback
 *
 * Print simple descriptor responce
 */
void simple_desc_callback(zb_uint8_t param) ZB_CALLBACK {
    zb_buf_t *buf = ZB_BUF_FROM_REF(param);
    zb_uint8_t *zdp_cmd = ZB_BUF_BEGIN(buf);
    zb_zdo_simple_desc_resp_t *resp = (zb_zdo_simple_desc_resp_t *)(zdp_cmd);
    char str[100];
    zb_uint_t i;

    sprintf(str, "\n\r> status %hd, addr 0x%x\n\r",
            resp->hdr.status, resp->hdr.nwk_addr);
    print(str);

    if (resp->hdr.status != ZB_ZDP_STATUS_SUCCESS) {
        print("\n\r> Error incorrect status\n\r");
    }

    sprintf(str, "\tep: %d, app profile: %d, dev id: %d, dev ver: %d\n\r",
            resp->simple_desc.endpoint,
            resp->simple_desc.app_profile_id,
            resp->simple_desc.app_device_id,
            resp->simple_desc.app_device_version);
    print(str);
    sprintf(str, "\tinput clusters count: 0x%x",
            resp->simple_desc.app_input_cluster_count);
    print(str);

    zb_uint8_t *cnt_ptr = &(resp->simple_desc.app_input_cluster_count);
    zb_uint16_t *ptr = (zb_uint16_t *)(cnt_ptr + 1);

    if (*cnt_ptr) {
        print("\n\r\t\tinput clusters: ");
    }
    for (i = 0; i < *(cnt_ptr); i++) {
        sprintf(str, "%d ", *ptr);
        print(str);
        ++ptr;
    }

    cnt_ptr = (zb_uint8_t *)ptr;
    ptr = (zb_uint16_t *)(cnt_ptr + 1);

    sprintf(str, "\n\r\toutput cluster count %x", *(cnt_ptr));
    if (*(cnt_ptr)) {
        print("\n\r\t\toutput clusters: ");
    }
    for (i = 0; i < *(cnt_ptr); i++) {
        sprintf(str, "%d ", *ptr);
        print(str);
        ++ptr;
    }

    zb_free_buf(buf);

    on_return();
}

/**
 * @brief neighbors command callback
 *
 * Print neighbor table from Mgmt_Lqi_res
 */
void neighbors_callback(zb_uint8_t param) ZB_CALLBACK {
    zb_buf_t *buf = ZB_BUF_FROM_REF(param);
    zb_uint8_t *zdp_cmd = ZB_BUF_BEGIN(buf);
    zb_zdo_mgmt_lqi_resp_t *resp = (zb_zdo_mgmt_lqi_resp_t *)(zdp_cmd);
    zb_zdo_neighbor_table_record_t *record = (zb_zdo_neighbor_table_record_t *)(resp + 1);
    zb_uint_t i;
    char str[100];

    sprintf(str, "\n\r> Mgmt_Lqi_req status %hd, neighbor_table_entries %hd",
            resp->status,
            resp->neighbor_table_entries);
    print(str);

    sprintf(str, "\n\r  start_index %hd, neighbor_table_list_count %d",
            resp->start_index,
            resp->neighbor_table_list_count);
    print(str);

    for (i = 0; i < resp->neighbor_table_list_count; i++) {
        sprintf(str, "\n\r  #%hd:   long addr " FORMAT_64 " pan id " FORMAT_64,
                i,
                ARG_64(record->ext_addr),
                ARG_64(record->ext_pan_id));
        print(str);

        sprintf(str, "\n\r\tnetwork_addr %d, dev_type %hd, rx_on_wen_idle %hd",
                record->network_addr,
                ZB_ZDO_RECORD_GET_DEVICE_TYPE(record->type_flags),
                ZB_ZDO_RECORD_GET_RX_ON_WHEN_IDLE(record->type_flags));
        print(str);

        sprintf(str, "\n\r\trelationship %hd, permit_join %hd, depth %hd, lqi %hd",
                ZB_ZDO_RECORD_GET_RELATIONSHIP(record->type_flags),
                record->permit_join,
                record->depth,
                record->lqi);
        print(str);

        record++;
    }

    zb_free_buf(buf);

    on_return();
}

/**
 * @brief nwk command callback
 *
 * Print NWK address responce
 */
void nwk_addr_callback(zb_uint8_t param) ZB_CALLBACK {
    zb_buf_t *buf = ZB_BUF_FROM_REF(param);
    zb_zdo_nwk_addr_resp_head_t *resp = (zb_zdo_nwk_addr_resp_head_t *)ZB_BUF_BEGIN(buf);
    char str[100];

    sprintf(str, "\n\rResponce status %hd, nwk addr %d\n\rieee addr " FORMAT_64,
            resp->status,
            resp->nwk_addr,
            ARG_64(resp->ieee_addr));
    print(str);

    zb_free_buf(buf);

    on_return();
}

/**
 * @brief leave command callback
 *
 * Print Mgmt_Leave_res.
 */
void leave_callback(zb_uint8_t param) ZB_CALLBACK {
    zb_uint8_t *ret = (zb_uint8_t *)ZB_BUF_BEGIN(ZB_BUF_FROM_REF(param));
    char str[100];

    sprintf(str, "\n\rLeave callback status: %hd", *ret);
    print(str);

    zb_free_buf(ZB_BUF_FROM_REF(param));

    on_return();
}

/**
 * @brief permit_join command callback
 * 
 * Print permit joining responce. 
 */
void permit_joining_callback(zb_uint8_t param) ZB_CALLBACK {
    zb_buf_t *buf = ZB_BUF_FROM_REF(param);
    zb_uint8_t *ret = (zb_uint8_t *)ZB_BUF_BEGIN(buf);
    char str[100];

    sprintf(str, "\n\rPermit joining status: %hd\n\r", *ret);
    print(str);

    zb_free_buf(buf);

    on_return();
}
