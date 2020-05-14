/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#ifndef H_MYNEWT_TCP_
#define H_MYNEWT_TCP_

#include "os/mynewt.h"
#include "oic/oc_ri.h"
#include "oic/oc_ri_const.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef bool oc_tcp_frag_belongs_fn(const struct os_mbuf *frag,
                                    const struct os_mbuf *pkt,
                                    void *arg);

typedef void oc_tcp_fill_endpoint(struct os_mbuf *om, void *arg);

struct oc_tcp_reassembler {
    STAILQ_HEAD(, os_mbuf_pkthdr) pkt_q;
    oc_tcp_frag_belongs_fn *frag_belongs;
    oc_tcp_fill_endpoint *fill_endpoint;
    int endpoint_size;
};

int oc_tcp_reass(struct oc_tcp_reassembler *reassembler, struct os_mbuf *om1,
                 void *arg, struct os_mbuf **out_pkt);

#ifdef __cplusplus
}
#endif

#endif
