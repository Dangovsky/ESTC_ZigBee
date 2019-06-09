#include "cmd_callbacks.c"
#include "console.h"

/** used when command handeled localy and successfully ended */
#define SUCCESS_END_MESS(str)   \
    print(CLEAR_LINE "> " str); \
    ON_RETURN

/** used when command calls remote device and we wait for callback */
#define SUCCESS_SEND_MESS(str) \
    print(CLEAR_LINE "> " str " request is send.");

/** used when command ended becouse some error */
#define ERROR_MESS                                                                       \
    print(CLEAR_LINE "> Incorrect arguments. Print 'help', to see commands arguments."); \
    ON_RETURN

/*
 * Clear
 */
void clear_cmd_handler(zb_uint8_t param) ZB_CALLBACK {
    print(
        "\033[2J"  /* ESC seq for clear entire screen            */
        "\033[H"); /* ESC seq for move cursor at left-top corner */
    WRITE_PROMPT
    if (param) {
        zb_free_buf(ZB_BUF_FROM_REF(param));
    }
    set_current_command(NULL);
    return;
}

/*
 * Help
 */
void help_cmd_handler(zb_uint8_t param) ZB_CALLBACK {
    print(
        CLEAR_LINE
        "Use TAB key for completion\n\rCommands:\n\r");
    print(
        "\t" _CMD_CLEAR
        "                  - clear screen\n\r"
        "\t" _CMD_BEACON_REQ
        "             - send beacon request\n\r"
        "\t" _CMD_IEEE_ADDR
        "        [nwk addr] - get ieee address descriptor\n\r");
    print(
        "\t" _CMD_PERMIT_JOIN
        " [duration] [dest_nwk](opt) - open network for new devices for [duration] seconds\n\r"
        "\t\t\ton 0 - close network, on 255 open till new permit_join request\n\r");
    print(
        "\t" _CMD_ACTIVE_EP
        "          [nwk addr] - get active endpoints descriptor\n\r"
        "\t" _CMD_SIMPLE_DISC
        "      [nwk addr] [ep] - get simple descriptor\n\r"
        "\t" _CMD_NEIGHBORS "   [nwk addr] [start_index](opt) - get neighbors table\n\r");
    print(
        "\t" _CMD_NWK_ADDR
        "         [ieee addr in 8 space-separated numbers] [dst_nwk](opt) - \n\r"
        "\t\t\t- get nwk address descriptor\n\r"
        "\t" _CMD_LEAVE
        "       [ieee addr in 8 space-separated numbers] - device leave the network\n\r");
    print(
        "\t" _CMD_DATA_REQ
        "        [nwk addr] [src ep] [dst ep] [profile id] [payload](opt space-separated)\n\r"
        "\t\t\t- send apse data request to [nwk addr]");

    WRITE_PROMPT
    if (param) {
        zb_free_buf(ZB_BUF_FROM_REF(param));
    }
    set_current_command(NULL);
    return;
}

/*
 * IEEE
 */
void ieee_cmd_handler(zb_uint8_t param) ZB_CALLBACK {
    zb_buf_t *buf = ZB_BUF_FROM_REF(param);
    zb_zdo_ieee_addr_req_t *req;
    zb_uint8_t i = 1;

    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_ieee_addr_req_t), req);

    if (i < get_current_argc()) {
        req->nwk_addr = (zb_uint8_t)strtol(get_current_argv(i), NULL, 16);
    }

    if (i >= get_current_argc() || ZB_PIB_SHORT_ADDRESS() == req->nwk_addr) {
        /* get self address */
        char str[50];

        sprintf(str,
                CLEAR_LINE "> My ieee address: " FORMAT_64,
                ARG_64(ZB_PIB_EXTENDED_ADDRESS()));
        print(str);

        zb_free_buf(buf);
        ON_RETURN
    } else {
        /* remote address request */
        ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_ieee_addr_req_t), req);
        req->nwk_addr = (zb_uint8_t)strtol(get_current_argv(i), NULL, 16);
        req->request_type = ZB_ZDO_SINGLE_DEV_RESPONSE;
        req->start_index = 0;

#ifdef IEEE_TEST
        ZB_SCHEDULE_ALARM(ieee_addr_callback, param, 100);
#else
        zb_zdo_ieee_addr_req(param, ieee_addr_callback);
#endif
        SUCCESS_SEND_MESS("IEEE");
    }
    return;
}

/*
 * active ep 
 */
void active_ep_cmd_handler(zb_uint8_t param) ZB_CALLBACK {
    zb_buf_t *buf = ZB_BUF_FROM_REF(param);
    zb_zdo_active_ep_req_t *req;
    zb_uint8_t i = 1;

    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_active_ep_req_t), req);

    if (i < get_current_argc()) {
        req->nwk_addr = (zb_uint8_t)strtol(get_current_argv(i), NULL, 16);
    }

    if (i >= get_current_argc() || ZB_PIB_SHORT_ADDRESS() == req->nwk_addr) {
        /* get self endpoints */
        char str[50];

        sprintf(str,
                CLEAR_LINE
                "> My ep count: %hd"
                "\n\r\teps: ",
                ZB_MAX_EP_NUMBER);
        print(str);
        for (i = 0; i < ZB_MAX_EP_NUMBER; ++i) {
            sprintf(str, "%d ", ZB_ZDO_SIMPLE_DESC_LIST()[i]->endpoint);
            print(str);
        }

        zb_free_buf(buf);
        ON_RETURN
    } else {
        /* remote request */
        zb_zdo_active_ep_req(param, active_ep_callback);

        SUCCESS_SEND_MESS("Active end points");
    }
    return;
}

/*
 * simple descriptor 
 */
void simple_desk_cmd_handler(zb_uint8_t param) ZB_CALLBACK {
    zb_buf_t *buf = ZB_BUF_FROM_REF(param);
    zb_uint8_t i = 1, ep;
    zb_uint16_t nwk_addr;

    if (i >= get_current_argc()) {
        zb_free_buf(buf);
        ERROR_MESS;
        return;
    }
    ep = (zb_uint8_t)strtol(get_current_argv(i), NULL, 10);

    ++i;
    if (i < get_current_argc()) {
        nwk_addr = (zb_uint8_t)strtol(get_current_argv(i), NULL, 16);
    }

    if (i >= get_current_argc() || ZB_PIB_SHORT_ADDRESS() == nwk_addr) {
        /* self simple descriptor 
         * this is copy of a part of zdo_send_simple_desc_resp */
        zb_zdo_simple_desc_resp_hdr_t *resp_hdr;
        zb_af_simple_desc_1_1_t *src_desc = NULL;
        zb_uint8_t *desc_body;

        ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_simple_desc_resp_hdr_t), resp_hdr);
        resp_hdr->length = 0;

        if (!ep) {
            src_desc = (zb_af_simple_desc_1_1_t *)ZB_ZDO_SIMPLE_DESC();
        } else {
            for (i = 0; i < ZB_ZDO_SIMPLE_DESC_NUMBER(); i++) {
                if (ZB_ZDO_SIMPLE_DESC_LIST()[i]->endpoint == ep) {
                    src_desc = (zb_af_simple_desc_1_1_t *)ZB_ZDO_SIMPLE_DESC_LIST()[i];
                }
            }
        }

        if (src_desc) {
            resp_hdr->status = ZB_ZDP_STATUS_SUCCESS;
            resp_hdr->length = sizeof(zb_af_simple_desc_1_1_t) +
                               /* take into account app_cluster_list */
                               (src_desc->app_input_cluster_count + src_desc->app_output_cluster_count - 2) * sizeof(zb_uint16_t);

            ZB_BUF_ALLOC_RIGHT(buf, resp_hdr->length, desc_body);
            zb_copy_simple_desc((zb_af_simple_desc_1_1_t *)desc_body, src_desc);
        } else {
            resp_hdr->status = ZB_ZDP_STATUS_INVALID_EP;
        }
        ZB_LETOH16(&resp_hdr->nwk_addr, &ZB_PIB_SHORT_ADDRESS());

        ZB_SCHEDULE_CALLBACK(simple_desc_callback, param);

        print(CLEAR_LINE
              "> My simple descriptor:");
    } else {
        /* send remote simple descriptorrequest */
        zb_zdo_simple_desc_req_t *req;
        ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_simple_desc_req_t), req);
        req->nwk_addr = nwk_addr;
        req->endpoint = ep;

        zb_zdo_simple_desc_req(param, simple_desc_callback);

        SUCCESS_SEND_MESS("Simple desk");
    }
    return;
}

/*
 * Mgmt_Lqi_req 
 * get neighbor table
 */
void neighbors_cmd_handler(zb_uint8_t param) ZB_CALLBACK {
    zb_buf_t *buf = ZB_BUF_FROM_REF(param);
    zb_uint8_t i = 1, start_index;
    zb_uint16_t nwk_addr;

    if (i < get_current_argc()) {
        nwk_addr = (zb_uint8_t)strtol(get_current_argv(i), NULL, 16);
    } else {
        nwk_addr = ZB_PIB_SHORT_ADDRESS();
    }

    ++i;
    if (i >= get_current_argc()) {
        start_index = 0;
    } else {
        start_index = (zb_uint8_t)strtol(get_current_argv(i), NULL, 10);
    }

    if (ZB_PIB_SHORT_ADDRESS() == nwk_addr) {
        /* self simple descriptor 
         * this is copy of a part of zdo_lqi_resp */
        zb_zdo_mgmt_lqi_resp_t *resp;
        zb_zdo_neighbor_table_record_t *record;
        zb_uint8_t max_records_num;
        zb_uint8_t records_num;

        i = ZB_APS_HDR_SIZE(ZB_APS_FRAME_DATA)
#ifdef ZB_SECURITY
            + sizeof(zb_aps_nwk_aux_frame_hdr_t)
#endif
            + ZB_NWK_FULL_HDR_SIZE(1) + ZB_MAC_MAX_HEADER_SIZE(1, 1) + sizeof(zb_zdo_mgmt_lqi_resp_t) + ZB_TAIL_SIZE_FOR_SENDER_MAC_FRAME;
        max_records_num = ZB_IO_BUF_SIZE / i;

        records_num = (ZG->nwk.neighbor.base_neighbor_used > start_index) ? ZG->nwk.neighbor.base_neighbor_used - start_index : 0;
        TRACE_MSG(TRACE_ZDO3, "max rec %hd, used %hd, start indx %hd",
                  (FMT__H_H_H, max_records_num, ZG->nwk.neighbor.base_neighbor_used, start_index));

        records_num = (records_num < max_records_num) ? records_num : max_records_num;

        ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_mgmt_lqi_resp_t) + records_num * sizeof(zb_zdo_neighbor_table_record_t), resp);

        resp->status = ZB_ZDP_STATUS_SUCCESS;
        resp->neighbor_table_entries = ZG->nwk.neighbor.base_neighbor_used;
        resp->start_index = start_index;
        resp->neighbor_table_list_count = records_num;
        record = (zb_zdo_neighbor_table_record_t *)(resp + 1);

        for (i = 0; i < ZG->nwk.neighbor.base_neighbor_size && records_num; ++i) {
            if (ZG->nwk.neighbor.base_neighbor[i].used) {
                ZB_MEMCPY(record->ext_pan_id, ZB_NIB_EXT_PAN_ID(), sizeof(zb_ext_pan_id_t));
                zb_address_by_ref(record->ext_addr, &record->network_addr, ZG->nwk.neighbor.base_neighbor[i].addr_ref);

                ZB_ZDO_RECORD_SET_DEVICE_TYPE(record->type_flags, ZG->nwk.neighbor.base_neighbor[i].device_type);
                ZB_ZDO_RECORD_SET_RX_ON_WHEN_IDLE(record->type_flags, ZG->nwk.neighbor.base_neighbor[i].rx_on_when_idle);
                ZB_ZDO_RECORD_SET_RELATIONSHIP(record->type_flags, ZG->nwk.neighbor.base_neighbor[i].relationship);
                record->permit_join = ZG->nwk.neighbor.base_neighbor[i].permit_joining;
                record->depth = ZG->nwk.neighbor.base_neighbor[i].depth;
                record->lqi = ZG->nwk.neighbor.base_neighbor[i].lqi;
                records_num--;
                record++;
            }
        }

        ZB_SCHEDULE_CALLBACK(neighbors_callback, param);

        print(CLEAR_LINE
              "> My neighbors table:");
    } else {
        zb_zdo_mgmt_lqi_param_t *req;
        req = ZB_GET_BUF_TAIL(buf, sizeof(zb_zdo_mgmt_lqi_param_t));
        req->dst_addr = nwk_addr;
        req->start_index = start_index;

        zb_zdo_mgmt_lqi_req(param, neighbors_callback);
        SUCCESS_SEND_MESS("Neighbors");
    }
    return;
}

/*
 *  NWK_addr_req
 */
void nwk_addr_cmd_handler(zb_uint8_t param) ZB_CALLBACK {
    zb_buf_t *buf;
    zb_zdo_nwk_addr_req_param_t *req;
    zb_uint8_t i = 1, j;

    /* if there are no arguments */
    if (i >= get_current_argc()) {
        char str[20];
        sprintf(str,
                CLEAR_LINE "> My nwk addr: %hd",
                ZB_PIB_SHORT_ADDRESS());
        print(str);
        ON_RETURN
        return;
    }

    buf = ZB_BUF_FROM_REF(param);
    req = ZB_GET_BUF_PARAM(buf, zb_zdo_nwk_addr_req_param_t);

    for (j = 0; j < 8; j++) {
        if (j + i >= get_current_argc()) {
            zb_free_buf(buf);
            ERROR_MESS;
            return;
        }
        req->ieee_addr[7 - j] = (zb_uint8_t)strtol(get_current_argv(i + j), NULL, 16);
    }

    //  ZB_64BIT_ADDR_CMP(ZB_PIB_EXTENDED_ADDRESS(), req.ieee_addr);

    i += j;
    if (i < get_current_argc()) {
        req->dst_addr = (zb_uint8_t)strtol(get_current_argv(i), NULL, 16);
    } else {
        req->dst_addr = ZB_PIB_SHORT_ADDRESS();
    }

    if (ZB_PIB_SHORT_ADDRESS() == req->dst_addr) {
        /* self nwk addr request 
         * this is partialy a copy of zdo_device_nwk_addr_res
         */
        if (ZB_64BIT_ADDR_CMP(ZB_PIB_EXTENDED_ADDRESS(), req->ieee_addr)) {            
            /* it is "my" addr */
            char str[20];
            sprintf(str,
                    CLEAR_LINE "> My nwk addr: %hd",
                    ZB_PIB_SHORT_ADDRESS());
            print(str);
            ON_RETURN
            zb_free_buf(buf);
            return;
        } else {
            /* look for addr localy */
            zb_uint8_t i;
            zb_ieee_addr_t ieee_addr;
            zb_uint16_t nwk_addr = ZB_UNKNOWN_SHORT_ADDR;
            char str[20];

            for (i = 0; i < ZB_NEIGHBOR_TABLE_SIZE; ++i) {
                if (ZG->nwk.neighbor.base_neighbor[i].used) {
                    zb_address_ieee_by_ref(ieee_addr, ZG->nwk.neighbor.base_neighbor[i].addr_ref);
                    if (ZB_64BIT_ADDR_CMP(ieee_addr, req->ieee_addr)) {
                        zb_address_short_by_ref(&nwk_addr, ZG->nwk.neighbor.base_neighbor[i].addr_ref);

                        sprintf(str,
                                CLEAR_LINE "> Localy found nwk addr: %hd",
                                nwk_addr);
                        print(str);
                        ON_RETURN

                        zb_free_buf(buf);
                        return;
                    }
                }
            } 
            print(CLEAR_LINE "> Don`t find nwk addr localy");
            ON_RETURN
        }
    } else {
        /* Remote nwk addr request */
        req->request_type = ZB_ZDO_SINGLE_DEVICE_RESP;
        req->start_index = 0;

        zb_zdo_nwk_addr_req(param, nwk_addr_callback);

        SUCCESS_SEND_MESS("NWK");
    }
    return;
}

/*
 * Mgmt_Leave_req
 */
void leave_cmd_handler(zb_uint8_t param) ZB_CALLBACK {
    zb_buf_t *buf = ZB_BUF_FROM_REF(param);
    zb_zdo_mgmt_leave_param_t *req;
    zb_uint8_t i = 1, j;

    req = ZB_GET_BUF_PARAM(buf, zb_zdo_mgmt_leave_param_t);

    for (j = 0; j < 8; j++) {
        if (j + i >= get_current_argc()) {
            zb_free_buf(buf);
            ERROR_MESS;
            return;
        }
        req->device_address[7 - j] = (zb_uint8_t)strtol(get_current_argv(i + j), NULL, 16);
    }

    req->dst_addr = 0;
    req->remove_children = ZB_FALSE;
    req->rejoin = ZB_FALSE;

    zdo_mgmt_leave_req(param, leave_callback);

    SUCCESS_SEND_MESS("Leave");
    return;
}

/*
 * Permit joining
 */
void permit_joining_cmd_handler(zb_uint8_t param) ZB_CALLBACK {
    zb_buf_t *buf = ZB_BUF_FROM_REF(param);
    zb_nlme_permit_joining_request_t *local_req;
    zb_zdo_mgmt_permit_joining_req_param_t *send_req;
    zb_uint8_t i = 1;

    if (i >= get_current_argc()) {
        zb_free_buf(buf);
        ERROR_MESS;
        return;
    }
    send_req = ZB_GET_BUF_PARAM(buf, zb_zdo_mgmt_permit_joining_req_param_t);
    send_req->permit_duration = (zb_uint8_t)strtol(get_current_argv(i), NULL, 10);

    ++i;
    if (i < get_current_argc()) {
        send_req->dest_addr = (zb_uint8_t)strtol(get_current_argv(i), NULL, 16);
    }

    if (i >= get_current_argc() || ZB_PIB_SHORT_ADDRESS() == send_req->dest_addr) {
        /* this device */
        local_req = (zb_nlme_permit_joining_request_t *)ZB_GET_BUF_PARAM(buf, zb_nlme_permit_joining_request_t);
        local_req->permit_duration = send_req->permit_duration;
        ZB_SCHEDULE_CALLBACK(zb_nlme_permit_joining_request, param);

        SUCCESS_END_MESS("Permit joining is reset");
    } else {
        /* remote device */
        send_req->tc_significance = 0;
        zb_zdo_mgmt_permit_joining_req(param, permit_joining_callback);
        SUCCESS_SEND_MESS("Permit join");
    }
    return;
}

/*
 * Beacon request
 */
void beacon_cmd_handler(zb_uint8_t param) ZB_CALLBACK {
    zb_beacon_request_command();

    if (param) {
        zb_free_buf(ZB_BUF_FROM_REF(param));
    }
    SUCCESS_END_MESS("Beacon request is send");
    return;
}

/*
 * APSE data request
 */
void data_req_handler(zb_uint8_t param) ZB_CALLBACK {
    zb_buf_t *buf = ZB_BUF_FROM_REF(param);
    zb_apsde_data_req_t *req = ZB_GET_BUF_TAIL(buf, sizeof(zb_apsde_data_req_t));
    zb_uint8_t i = 1;
    char *ptr;

    if (i >= get_current_argc()) {
        zb_free_buf(buf);
        ERROR_MESS;
        return;
    }
    req->dst_addr.addr_short = (zb_uint8_t)strtol(get_current_argv(i), NULL, 16);

    ++i;
    if (i >= get_current_argc()) {
        zb_free_buf(buf);
        ERROR_MESS;
        return;
    }
    req->src_endpoint = (zb_uint8_t)strtol(get_current_argv(i), NULL, 10);

    ++i;
    if (i >= get_current_argc()) {
        zb_free_buf(buf);
        ERROR_MESS;
        return;
    }
    req->dst_endpoint = (zb_uint8_t)strtol(get_current_argv(i), NULL, 10);

    ++i;
    if (i >= get_current_argc()) {
        zb_free_buf(buf);
        ERROR_MESS;
        return;
    }
    req->profileid = (zb_uint8_t)strtol(get_current_argv(i), NULL, 10);

    /* first word in payload */
    ++i;
    if (i >= get_current_argc()) {
        zb_free_buf(buf);
        ERROR_MESS;
        return;
    }
    ZB_BUF_INITIAL_ALLOC(buf, strlen(get_current_argv(i)), ptr);
    strcpy(ptr, get_current_argv(i));

    /* other words in payload (if exist) */
    ++i;
    for (; i < get_current_argc(); ++i) {
        ZB_BUF_ALLOC_RIGHT(buf, strlen(get_current_argv(i)), ptr);
        strcat(ptr, get_current_argv(i));
    }

    req->addr_mode = ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
    req->tx_options = ZB_APSDE_TX_OPT_ACK_TX;
    req->radius = 5;
    buf->u.hdr.handle = 0x11;

    ZB_SCHEDULE_CALLBACK(zb_apsde_data_request, ZB_REF_FROM_BUF(buf));

    SUCCESS_END_MESS("Sendind apse data request");
    return;
}
