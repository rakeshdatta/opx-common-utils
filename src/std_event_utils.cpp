/*
 * Copyright (c) 2016 Dell Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * THIS CODE IS PROVIDED ON AN *AS IS* BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 * LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
 * FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
 *
 * See the Apache Version 2.0 License for specific language governing
 * permissions and limitations under the License.
 */

/**
 * filename: std_event_utils.c
 **/

/*
 * std_event_utils.c
 *
 *  Created on: May 23, 2014
 */

#include "private/std_event_utils.h"
#include "std_event_service.h"
#include "std_file_utils.h"
#include "std_socket_tools.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STD_ERR_RC(type,x) STD_ERR_MK(e_std_err_COM,e_std_err_code_##type,(x))

#define STD_CMN_IPC_VER 1

struct std_event_ipc_hdr_t {
    uint32_t size;
    uint32_t version;
};

struct std_event_ipc_data_t {
    std_event_ipc_hdr_t hdr;
    uint32_t version;
    uint32_t size;
};

t_std_error std_event_util_event_send(std_event_client_handle handle,
        event_serv_msg_t *msg, std_event_msg_descr_t *data, size_t len, size_t timeout) {
    t_std_error rc = STD_ERR_OK;
    std_event_ipc_data_t hdr;

    hdr.hdr.version = STD_CMN_IPC_VER;
    hdr.hdr.size =sizeof(std_event_ipc_data_t)-sizeof(std_event_ipc_hdr_t);
    hdr.version=0;

    hdr.size = 0;
    for (size_t ix = 0 ; ix < len ; ++ix ) {
        hdr.size += data[ix].len;
    }
    hdr.size += sizeof(*msg) ;

    struct iovec _iov[len+2];
    size_t ix = 0;
    size_t mx = len+2;

    _iov[ix].iov_base = &hdr;
    _iov[ix++].iov_len = sizeof(hdr);

    _iov[ix].iov_base = msg;
    _iov[ix++].iov_len = sizeof(*msg);

    for ( ; ix < mx ; ++ix ) {
        _iov[ix].iov_base = data[ix-2].data;
        _iov[ix].iov_len = data[ix-2].len;
    }

    std_socket_msg_t _pkt;
    memset(&_pkt,0,sizeof(_pkt));
    _pkt.msg_iov = _iov;
    _pkt.msg_iovlen = mx;

    std_socket_op(std_socket_transit_o_WRITE,handle,&_pkt,
            (std_socket_transit_flags_t)(std_socket_transit_f_NONBLOCK|
                    std_socket_transit_f_ALL),timeout,&rc);

    return rc;
}

static bool read_header(std_event_client_handle handle,std_event_ipc_data_t &hdr) {
    t_std_error rc = STD_ERR_OK;
    int by = std_read(handle,&hdr,sizeof(hdr.hdr),true,&rc);
    if (by!=sizeof(hdr.hdr)) return false;
    if (hdr.hdr.size >(sizeof(std_event_ipc_data_t)-sizeof(hdr.hdr))) {
        return false;
    }

    //check version and determine new hearder size
    //XXX check the message version if important to translate header
    by = std_read(handle,((char*)&hdr) + sizeof(hdr.hdr),hdr.hdr.size,true,&rc);
    if (by!=(int)hdr.hdr.size) return false;
    return true;
}

t_std_error std_event_util_event_recv(std_event_client_handle handle, std::vector<uint8_t> &buff, bool allow_resize) {
    t_std_error rc = STD_ERR_OK;
    std_event_ipc_data_t hdr;
    if (!read_header(handle,hdr)) return STD_ERR(COM,FAIL,0);
    if (hdr.size > buff.size()) {
        if (!allow_resize) {
            return STD_ERR(COM,TOOBIG,0);
        }
        try {
            buff.resize((unsigned int)hdr.size);
        } catch (...) {
            return STD_ERR_RC(TOOBIG,hdr.size);
        }
    }
    int by = std_read(handle,&(buff[0]),hdr.size,true,&rc);
    return by==(int)hdr.size ? STD_ERR_OK : STD_ERR_RC(FAIL,by);
}

t_std_error std_event_util_event_recv_msg(std_event_client_handle handle,
        std_event_msg_t *msg, void * data, size_t len) {

    t_std_error rc = STD_ERR_OK;
    std_event_ipc_data_t hdr;

    if (!read_header(handle,hdr)) return STD_ERR(COM,FAIL,0);
    event_serv_msg_t mtype;
    if (hdr.size < (sizeof(event_serv_msg_t) + sizeof(*msg))) return STD_ERR(COM,FAIL,0);

    int by = std_read(handle,&mtype,sizeof(mtype),true,&rc);
    if (by!=sizeof(mtype)) return STD_ERR_RC(CLOSED,handle);

    if (mtype.op!=event_serv_msg_t_PUBLISH) return STD_ERR(COM,FAIL,0);
    hdr.size -= sizeof(mtype);
    if (hdr.size > len ) return STD_ERR(COM,TOOBIG,0);

    by = std_read(handle,msg,sizeof(*msg),true,&rc);
    if (by!=(int)sizeof(*msg)) return STD_ERR(COM,FAIL,0);

    hdr.size-=sizeof(*msg);
    if (hdr.size != msg->data_len) return STD_ERR(COM,FAIL,0);

    by = std_read(handle,data,hdr.size,true,&rc);
    if (by!=(int)hdr.size) return STD_ERR(COM,FAIL,0);

    return STD_ERR_OK;
}
