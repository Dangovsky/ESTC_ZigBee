/**
 Trace format for 64-bit address
*/
#define TRACE_FORMAT_64 "%hx.%hx.%hx.%hx.%hx.%hx.%hx.%hx"

/**
 Trace format arguments for 64-bit address
*/
#define TRACE_ARG_64(a) (zb_uint8_t)((a)[7]),(zb_uint8_t)((a)[6]),(zb_uint8_t)((a)[5]),(zb_uint8_t)((a)[4]),(zb_uint8_t)((a)[3]),(zb_uint8_t)((a)[2]),(zb_uint8_t)((a)[1]),(zb_uint8_t)((a)[0])

extern microrl_t microrl;

void print(const char *str);
void zdo_desc_ieee_addr(zb_uint8_t param) ZB_CALLBACK;
void zdo_desc_active_ep(zb_uint8_t param) ZB_CALLBACK;
void zdo_simple_desk(zb_uint8_t param) ZB_CALLBACK;
void get_lqi_cb(zb_uint8_t param) ZB_CALLBACK;

int clear_cmd_handler(int argc, const char *const *argv) {                                     
    print("\033[2J"  /* ESC seq for clear entire screen            */     
          "\033[H"); /* ESC seq for move cursor at left-top corner */  
    return 0;
}

int help_cmd_handler(int argc, const char *const *argv) {                                     
    print(                                                                
        "Use TAB key for completion\n\rCommands:\n\r"                     
        "\tclear                  - clear screen\n\r"                     
        "\tieee   [nwk addr]      - get ieee address discriptor\n\r"      
        "\tep     [nwk addr]      - get active endpoints discriptor\n\r"  
        "\tsimple [nwk addr] [ep] - get simple discriptor\n\r");          
    return 0;
}

int ieee_cmd_handler(int argc, const char *const *argv){
    zb_buf_t *buf;
    zb_uint16_t *tail;
    int i = 1;

    if (i >= argc) {
        return 1;
    }
    buf = zb_get_out_buf();
    tail = ZB_GET_BUF_TAIL(buf, sizeof(zb_uint16_t));
    *tail = atoi(argv[i]);

    ZB_SCHEDULE_CALLBACK(zdo_desc_ieee_addr, ZB_REF_FROM_BUF(buf));
    print(">Request is send. When responce is arrived it will be printed.\n\r");
    return 0;
}

int active_ep_cmd_handler(int argc, const char *const *argv){
    zb_buf_t *buf;
    zb_uint16_t *tail;
    int i = 1;

    if (i >= argc) {
        return 1;
    }
    buf = zb_get_out_buf();
    tail = ZB_GET_BUF_TAIL(buf, sizeof(zb_uint16_t));
    *tail = atoi(argv[i]);

    ZB_SCHEDULE_CALLBACK(zdo_desc_active_ep, ZB_REF_FROM_BUF(buf));
    print(">Request is send. When responce is arrived it will be printed.\n\r");
    return 0;
}

int simple_disk_cmd_handler(int argc, const char *const *argv){
    zb_buf_t *buf;
    zb_zdo_simple_desc_req_t *req;
    int i = 1;

    if (i >= argc) {
        return 1;
    }
    buf = zb_get_out_buf();
    req = ZB_GET_BUF_TAIL(buf, sizeof(zb_zdo_simple_desc_req_t));
    req->nwk_addr = atoi(argv[i]);

    ++i;
    if (i >= argc) {
        return 1;
    }
    req->endpoint = atoi(argv[i]);

    ZB_SCHEDULE_CALLBACK(zdo_simple_desk, ZB_REF_FROM_BUF(buf));
    print(">Request is send. When responce is arrived it will be printed.\n\r");
    return 0;
}

int neibors_cmd_handler(int argc, const char* const* argv) {
    zb_buf_t *buf;
    zb_zdo_mgmt_lqi_param_t *req;
    int i = 1;

    if (i >= argc) {
        return 1;
    }
    buf = zb_get_out_buf();
    req = ZB_GET_BUF_TAIL(buf, sizeof(zb_zdo_mgmt_lqi_param_t));
    req->dst_addr = atoi(argv[i]);

    ++i;
    if (i >= argc) {
        req->start_index = 0;
    } else {
        req->start_index = atoi(argv[i]);
    }

    zb_zdo_mgmt_lqi_req(ZB_REF_FROM_BUF(buf), get_lqi_cb);
    print(">Request is send. When responce is arrived it will be printed.\n\r");
    return 0;
}

/* IEEE */
#ifdef IEEE_TEST
void ieee_addr_callback(zb_uint8_t param) ZB_CALLBACK {
    zb_uint8_t status = 15, nwk_addr = 10, ieee_addr[] = {12, 13, 14, 15, 16, 18, 19, 20};
    char str[100];
    sprintf(str, "\n\rget ieee address responce:\n\r\tstatus: %hd\n\r\tnwk address: %d\n\r",
            status, nwk_addr);            
    print(str);
    sprintf(str, "\tieee address: " TRACE_FORMAT_64 "\n\r",
            TRACE_ARG_64(ieee_addr));
    print(str);
    interrupt_new_line_handler(&microrl);
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
    sprintf(str, "\tieee address: " TRACE_FORMAT_64 "\n\r",
            TRACE_ARG_64(ieee_addr));
    print(str);
    interrupt_new_line_handler(&microrl);

    zb_free_buf(buf);
}
#endif /* IEEE_TEST */

#ifdef IEEE_TEST
void zdo_desc_ieee_addr(zb_uint8_t param) ZB_CALLBACK {
    ZB_SCHEDULE_ALARM(ieee_addr_callback,0,250);
    zb_free_buf(ZB_BUF_FROM_REF(param));
}
#else
void zdo_desc_ieee_addr(zb_uint8_t param) ZB_CALLBACK {
    zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
    zb_zdo_ieee_addr_req_t *req = NULL;
    zb_uint16_t addr = *((zb_uint16_t *)ZB_GET_BUF_TAIL(buf, sizeof(zb_uint16_t)));

    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_ieee_addr_req_t), req);

    req->nwk_addr = addr;
    req->request_type = ZB_ZDO_SINGLE_DEV_RESPONSE;
    req->start_index = 0;
    zb_zdo_ieee_addr_req(ZB_REF_FROM_BUF(buf), ieee_addr_callback);
}
#endif /* IEEE_TEST */

/* active ep */
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

    if (resp->status != ZB_ZDP_STATUS_SUCCESS || resp->nwk_addr != 0x0) {
        print("\n\rError incorrect status/addr");
    }

    sprintf(str, "\n\rep count: %hd\n\r", resp->ep_count);
    print(str);
    for (i = 0, print("\teps: "); i < resp->ep_count; ++i) {
        sprintf(str,"%d ",ep_list[i]);
        print(str);
    }
    interrupt_new_line_handler(&microrl);

    zb_free_buf(buf);
}
void zdo_desc_active_ep(zb_uint8_t param) ZB_CALLBACK {
    zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
    zb_zdo_active_ep_req_t *req;
    zb_uint16_t addr = *((zb_uint16_t *)ZB_GET_BUF_TAIL(buf, sizeof(zb_uint16_t)));

    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_active_ep_req_t), req);
    req->nwk_addr = addr;
    zb_zdo_active_ep_req(ZB_REF_FROM_BUF(buf), active_ep_callback);
}


/* simple discriptor */
void simple_desc_callback(zb_uint8_t param) ZB_CALLBACK {
    zb_buf_t *buf = ZB_BUF_FROM_REF(param);
    zb_uint8_t *zdp_cmd = ZB_BUF_BEGIN(buf);
    zb_zdo_simple_desc_resp_t *resp = (zb_zdo_simple_desc_resp_t *)(zdp_cmd);
    char str[100];
    zb_uint_t i;

    sprintf(str, "\n\rSimple desciptor status %hd, addr 0x%x\n\r",
            resp->hdr.status, resp->hdr.nwk_addr);
    print(str);

    if (resp->hdr.status != ZB_ZDP_STATUS_SUCCESS || resp->hdr.nwk_addr != 0x0) {
        print("Error incorrect status/addr");
    }

    sprintf(str, "\tep: %d, app profile: %d, dev id: %d, dev ver: %d\n\r",
            resp->simple_desc.endpoint,
            resp->simple_desc.app_profile_id,
            resp->simple_desc.app_device_id,
            resp->simple_desc.app_device_version);
    print(str);
    sprintf(str, "\tinput clusters count: 0x%x\n\r",
            resp->simple_desc.app_input_cluster_count);
    print(str);

    zb_uint8_t *cnt_ptr = &(resp->simple_desc.app_input_cluster_count);
    zb_uint16_t *ptr = (zb_uint16_t *)(cnt_ptr + 1);

    for (i = 0, print("\n\r\tinput clusters: "); i < *(cnt_ptr); i++) {
        sprintf(str,"%d ",*ptr);
        print(str);
        ++ptr;
    }

    cnt_ptr = (zb_uint8_t *)ptr;
    ptr = (zb_uint16_t *)(cnt_ptr + 1);

    sprintf(str, "\n\r\totput cluster count %x", *(cnt_ptr));
    for (i = 0, print("\n\r\tinput clusters: "); i < *(cnt_ptr); i++) {
        sprintf(str,"%d ",*ptr);
        print(str);
        ++ptr;
    }
    interrupt_new_line_handler(&microrl);

    zb_free_buf(buf);
}

void zdo_simple_desk(zb_uint8_t param) ZB_CALLBACK {
    zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
    zb_zdo_simple_desc_req_t *req_tail = ZB_GET_BUF_TAIL(buf, sizeof(zb_zdo_simple_desc_req_t));
    zb_zdo_simple_desc_req_t *req;

    ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_simple_desc_req_t), req);
    req->nwk_addr = req_tail->nwk_addr;
    req->endpoint = req_tail->endpoint;
    zb_zdo_simple_desc_req(param, simple_desc_callback);
}

/* Mgmt_Lqi_req 
 * get neighbor table*/
void get_lqi_cb(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_uint8_t *zdp_cmd = ZB_BUF_BEGIN(buf);
  zb_zdo_mgmt_lqi_resp_t *resp = (zb_zdo_mgmt_lqi_resp_t*)(zdp_cmd);
  zb_zdo_neighbor_table_record_t *record = (zb_zdo_neighbor_table_record_t*)(resp + 1);
  zb_uint_t i;
  char str[100];

  sprintf(str, "\n\rget_lqi_cb status %hd, neighbor_table_entries %hd",
          resp->status,
          resp->neighbor_table_entries);
  print(str);

  sprintf(str, "\n\rstart_index %hd, neighbor_table_list_count %d",
          resp->start_index,
          resp->neighbor_table_list_count);
  print(str);

  for (i = 0; i < resp->neighbor_table_list_count; i++)
  {
    sprintf(str,"\n\r#%hd: long addr " TRACE_FORMAT_64 " pan id " TRACE_FORMAT_64,
              i, 
              TRACE_ARG_64(record->ext_addr), 
              TRACE_ARG_64(record->ext_pan_id));
    print(str);

    sprintf(str, "\tnetwork_addr %d, dev_type %hd, rx_on_wen_idle %hd\n\r",
            record->network_addr,
            ZB_ZDO_RECORD_GET_DEVICE_TYPE(record->type_flags),
            ZB_ZDO_RECORD_GET_RX_ON_WHEN_IDLE(record->type_flags));
    print(str);

    sprintf(str, "\trelationship %hd, permit_join %hd, depth %hd, lqi %hd",
            ZB_ZDO_RECORD_GET_RELATIONSHIP(record->type_flags),
            record->permit_join,
            record->depth,
            record->lqi);
    print(str);

    record++;
  }
  interrupt_new_line_handler(&microrl);
}

