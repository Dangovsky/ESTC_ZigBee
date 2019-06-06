#include "cmd_callbacks.c"
#include "console.h"

#define SUCCESS_MESS                                                         \
    print("> Request is send. When responce arrived it will be printed.");   \
    interrupt_new_line_handler(&microrl);

#define ERROR_MESS                                                           \
    print("> Incorrect arguments. Print 'help', to see commands arguments."); \
    interrupt_new_line_handler(&microrl);

void clear_cmd_handler(zb_uint8_t param) ZB_CALLBACK{
    print(
        "\033[2J"  /* ESC seq for clear entire screen            */
        "\033[H"); /* ESC seq for move cursor at left-top corner */
    interrupt_new_line_handler(&microrl);
    zb_free_buf(ZB_BUF_FROM_REF(param));
    return;
}

void help_cmd_handler(zb_uint8_t param)  ZB_CALLBACK{
    print(
        "Use TAB key for completion\n\rCommands:\n\r"
        "\tclear                  - clear screen\n\r"
        "\tieee        [nwk addr] - get ieee address discriptor\n\r");
    print(
        "\tep          [nwk addr] - get active endpoints discriptor\n\r"
        "\tsimple      [nwk addr] [ep] - get simple discriptor\n\r"
        "\tneighbors   [nwk addr] [start_index](opt) - get neighbors table\n\r");
    print(
        "\tnwk         [ieee addr in 8 space-separated numbers] - get nwk address discriptor\n\r"
        "\tleave       [ieee addr in 8 space-separated numbers] - device leave the network\n\r");
    print(
        "\tpermit_join [duration] - open network for new devices for [duration] seconds;\n\r"    
            "\t\ton 0 - close network, on 255 open till new permit_join request");
    interrupt_new_line_handler(&microrl);
    zb_free_buf(ZB_BUF_FROM_REF(param));
    return;
}

void ieee_cmd_handler(zb_uint8_t param)  ZB_CALLBACK{
    zb_buf_t *buf = ZB_BUF_FROM_REF(param);
    zb_zdo_ieee_addr_req_t *req;
    zb_uint8_t i = 1;

    if (i >= argc_g) {
        ERROR_MESS;
        return;
    }
    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_ieee_addr_req_t), req);
    req->nwk_addr = atoi(argv_g[i]);
    req->request_type = ZB_ZDO_SINGLE_DEV_RESPONSE;
    req->start_index = 0;

    zb_zdo_ieee_addr_req(ZB_REF_FROM_BUF(buf), ieee_addr_callback);

    SUCCESS_MESS;
    return;
}

void active_ep_cmd_handler(zb_uint8_t param)  ZB_CALLBACK{
    zb_buf_t *buf = ZB_BUF_FROM_REF(param);
    zb_zdo_active_ep_req_t *req;
    zb_uint8_t i = 1;

    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_active_ep_req_t), req);
    req->nwk_addr = atoi(argv_g[i]);

    zb_zdo_active_ep_req(ZB_REF_FROM_BUF(buf), active_ep_callback);

    SUCCESS_MESS;
    return;
}

void simple_disk_cmd_handler(zb_uint8_t param)  ZB_CALLBACK{
    zb_buf_t *buf = ZB_BUF_FROM_REF(param);
    zb_zdo_simple_desc_req_t *req;
    int i = 1;

    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_simple_desc_req_t), req);
    req->nwk_addr = atoi(argv_g[i]);

    ++i;
    if (i >= argc_g) {
        zb_free_buf(buf);
        ERROR_MESS;
        return;
    }
    req->endpoint = atoi(argv_g[i]);

    zb_zdo_simple_desc_req(ZB_REF_FROM_BUF(buf), simple_desc_callback);

    SUCCESS_MESS;
    return;
}

void neighbors_cmd_handler(zb_uint8_t param)  ZB_CALLBACK{
    zb_buf_t *buf = ZB_BUF_FROM_REF(param);
    zb_zdo_mgmt_lqi_param_t *req;
    int i = 1;

    req = ZB_GET_BUF_TAIL(buf, sizeof(zb_zdo_mgmt_lqi_param_t));
    req->dst_addr = atoi(argv_g[i]);

    ++i;
    if (i >= argc_g) {
        req->start_index = 0;
    } else {
        req->start_index = atoi(argv_g[i]);
    }

    SUCCESS_MESS;
    return;
}

void nwk_addr_cmd_handler(zb_uint8_t param)  ZB_CALLBACK{
    zb_buf_t *buf = ZB_BUF_FROM_REF(param);
    zb_zdo_nwk_addr_req_param_t *req;
    zb_uint8_t i = 1, j;

    req = ZB_GET_BUF_PARAM(buf, zb_zdo_nwk_addr_req_param_t);

    for (j = 0; j < 8; j++) {
        if (j + i >= argc_g) {
            zb_free_buf(buf);
            ERROR_MESS;
            return;
        }
        req->ieee_addr[7 - j] = atoi(argv_g[i + j]);
    }

    req->dst_addr = 0;
    req->request_type = ZB_ZDO_SINGLE_DEVICE_RESP;
    req->start_index = 0;
    zb_zdo_nwk_addr_req(ZB_REF_FROM_BUF(buf), nwk_addr_callback);
    
    SUCCESS_MESS;
    return;
}

void leave_cmd_handler(zb_uint8_t param) ZB_CALLBACK{
    zb_buf_t *buf = ZB_BUF_FROM_REF(param);
    zb_zdo_mgmt_leave_param_t *req;
    zb_uint8_t i = 1, j;

    req = ZB_GET_BUF_PARAM(buf, zb_zdo_mgmt_leave_param_t);

    for (j = 0; j < 8; j++) {
        if (j + i >= argc_g) {
            zb_free_buf(buf);
            ERROR_MESS;
            return;
        }
        req->device_address[7 - j] = atoi(argv_g[i + j]);
    }

    req->dst_addr = 0;
    req->remove_children = ZB_FALSE;
    req->rejoin = ZB_FALSE;
    zdo_mgmt_leave_req(ZB_REF_FROM_BUF(buf), leave_callback);

    SUCCESS_MESS;
    return;
}

/*
 * Permit joining
 */
void permit_joining_cmd_handler(zb_uint8_t param)  ZB_CALLBACK{
    zb_buf_t *buf = ZB_BUF_FROM_REF(param);
    zb_zdo_mgmt_permit_joining_req_param_t *req;
    zb_uint8_t i = 1;

    req = ZB_GET_BUF_PARAM(buf, zb_zdo_mgmt_permit_joining_req_param_t);
    req->permit_duration = atoi(argv_g[i]);

    req->dest_addr = 0;
    req->tc_significance = 0;
    zb_zdo_mgmt_permit_joining_req(ZB_REF_FROM_BUF(buf), permit_joining_callback);

    SUCCESS_MESS;
    return;
}
