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

/**
 * Indicates whether an incoming fragment belongs to the partially received
 * packet identified by `arg`.
 */
typedef bool oc_tcp_ep_match(const void *ep, void *arg);

/**
 * Fills the given endpoint data structure.
 */
typedef void oc_tcp_ep_fill(void *ep, void *arg);

/**
 * Used for reassembling CoAP-over-TCP packets.  A transport should only have
 * one reassembler.
 *
 * The user must initialize ALL members.  After initialization, the user should
 * not directly access any members.
 */
struct oc_tcp_reassembler {
    STAILQ_HEAD(, os_mbuf_pkthdr) pkt_q; /* Use STAILQ_HEAD_INITIALIZER() */
    oc_tcp_ep_match *ep_match;
    oc_tcp_ep_fill *ep_fill;
    int endpoint_size;
};

/**
 * Partially reassembles a CoAP-over-TCP packet from an incoming fragment.  If
 * the fragment completes a packet, the reassembled packet is communicated back
 * to the user via the `out_pkt` parameter.  Otherwise, the partial packet is
 * recorded in the reassembler object.
 *
 * @param r                     The reassembler associated with this transport.
 * @param frag                  The incoming fragment.
 * @param arg                   The generic argument to pass to the
 *                                  reassembler's callbacks.
 * @param out_pkt               If the fragment completes a packet, the
 *                                  completed packet is stored here.
 *
 * @return                      0 if the fragment was successfully processed;
 *                              SYS_ENOMEM on mbuf allocation failure.
 */
int oc_tcp_reass(struct oc_tcp_reassembler *r, struct os_mbuf *frag, void *arg,
                 struct os_mbuf **out_pkt);

#ifdef __cplusplus
}
#endif

#endif
