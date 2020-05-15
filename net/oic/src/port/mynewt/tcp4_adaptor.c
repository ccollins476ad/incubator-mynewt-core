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

#include <assert.h>
#include <string.h>
#include <stdint.h>

#include "os/mynewt.h"

uint8_t oc_tcp4_transport_id = -1;

#if (MYNEWT_VAL(OC_TRANSPORT_IP) == 1) && (MYNEWT_VAL(OC_TRANSPORT_IPV4) == 1)
#include <log/log.h>
#include <mn_socket/mn_socket.h>
#include <stats/stats.h>

#include "oic/oc_log.h"
#include "oic/port/oc_connectivity.h"
#include "oic/port/mynewt/adaptor.h"
#include "oic/port/mynewt/transport.h"
#include "oic/port/mynewt/ip.h"
#include "oic/port/mynewt/tcp.h"
#include "oic/port/mynewt/tcp4.h"

#ifdef OC_SECURITY
#error This implementation does not yet support security
#endif

static void oc_send_buffer_tcp4(struct os_mbuf *m);
static uint8_t oc_ep_tcp4_size(const struct oc_endpoint *oe);
static char *oc_log_ep_tcp4(char *ptr, int maxlen, const struct oc_endpoint *);
static int oc_connectivity_init_tcp4(void);
void oc_connectivity_shutdown_tcp4(void);
static void oc_event_tcp4(struct os_event *ev);
static bool oc_tcp4_ep_match(const void *ep, void *arg);
static void oc_tcp4_ep_fill(void *ep, void *arg);

static const struct oc_transport oc_tcp4_transport = {
    .ot_flags = OC_TRANSPORT_USE_TCP,
    .ot_ep_size = oc_ep_tcp4_size,
    .ot_tx_ucast = oc_send_buffer_tcp4,
    .ot_tx_mcast = NULL,
    .ot_get_trans_security = NULL,
    .ot_ep_str = oc_log_ep_tcp4,
    .ot_init = oc_connectivity_init_tcp4,
    .ot_shutdown = oc_connectivity_shutdown_tcp4
};

static struct os_event oc_tcp4_read_event = {
    .ev_cb = oc_event_tcp4,
};

#define COAP_PORT_UNSECURED (5683)

STATS_SECT_START(oc_tcp4_stats)
    STATS_SECT_ENTRY(iframe)
    STATS_SECT_ENTRY(ibytes)
    STATS_SECT_ENTRY(ierr)
    STATS_SECT_ENTRY(oucast)
    STATS_SECT_ENTRY(obytes)
    STATS_SECT_ENTRY(oerr)
STATS_SECT_END
STATS_SECT_DECL(oc_tcp4_stats) oc_tcp4_stats;
STATS_NAME_START(oc_tcp4_stats)
    STATS_NAME(oc_tcp4_stats, iframe)
    STATS_NAME(oc_tcp4_stats, ibytes)
    STATS_NAME(oc_tcp4_stats, ierr)
    STATS_NAME(oc_tcp4_stats, oucast)
    STATS_NAME(oc_tcp4_stats, obytes)
    STATS_NAME(oc_tcp4_stats, oerr)
STATS_NAME_END(oc_tcp4_stats)

struct oc_tcp4_ep_desc {
    struct mn_socket *sock;
    oc_ipv4_addr_t addr;
    uint16_t port;
};

static struct oc_tcp_reassembler oc_tcp4_r = {
    .pkt_q = STAILQ_HEAD_INITIALIZER(oc_tcp4_r.pkt_q),
    .ep_match = oc_tcp4_ep_match,
    .ep_fill = oc_tcp4_ep_fill,
    .endpoint_size = sizeof(struct oc_endpoint_ip),
};

struct oc_tcp4_conn {
    struct mn_socket *sock;
    oc_tcp4_err_fn *err_cb;
    void *err_arg;
    SLIST_ENTRY(oc_tcp4_conn) next;
};

struct os_mempool oc_tcp4_conn_pool;
static os_membuf_t oc_tcp4_conn_buf[
    OS_MEMPOOL_SIZE(MYNEWT_VAL(OC_TCP4_MAX_CONNS),
                    sizeof (struct oc_tcp4_conn))
];

static SLIST_HEAD(, oc_tcp4_conn) oc_tcp4_conn_list;

/* sockets to use for coap unicast and multicast */
static struct mn_socket *oc_ucast4;

static char *
oc_log_ep_tcp4(char *ptr, int maxlen, const struct oc_endpoint *oe)
{
    const struct oc_endpoint_ip *oe_ip = (const struct oc_endpoint_ip *)oe;
    int len;

    mn_inet_ntop(MN_PF_INET, oe_ip->v4.address, ptr, maxlen);
    len = strlen(ptr);
    snprintf(ptr + len, maxlen - len, "-%u", oe_ip->port);
    return ptr;
}

static uint8_t
oc_ep_tcp4_size(const struct oc_endpoint *oe)
{
    return sizeof(struct oc_endpoint_ip);
}

static void
oc_send_buffer_tcp4(struct os_mbuf *m)
{
    struct oc_endpoint_ip *oe_ip;
    int rc;

    STATS_INC(oc_tcp4_stats, oucast);

    assert(OS_MBUF_USRHDR_LEN(m) >= sizeof(struct oc_endpoint_ip));
    oe_ip = OS_MBUF_USRHDR(m);

    assert(oe_ip->sock != NULL);

    STATS_INCN(oc_tcp4_stats, obytes, OS_MBUF_PKTLEN(m));

    rc = mn_sendto(oe_ip->sock, m, NULL);
    if (rc != 0) {
        OC_LOG_ERROR("Failed to send buffer %u ucast\n",
                     OS_MBUF_PKTHDR(m)->omp_len);
        STATS_INC(oc_tcp4_stats, oerr);
        os_mbuf_free_chain(m);
    }
}

static bool
oc_tcp4_ep_match(const void *ep, void *arg)
{
    struct oc_tcp4_ep_desc *ep_desc;
    const struct oc_endpoint_ip *oe_ip;

    oe_ip = ep;
    ep_desc = arg;

    return ep_desc->sock == oe_ip->sock;
}

static void
oc_tcp4_ep_fill(void *ep, void *arg)
{
    struct oc_tcp4_ep_desc *ep_desc;
    struct oc_endpoint_ip *oe_ip;

    oe_ip = ep;
    ep_desc = arg;

    oe_ip->ep.oe_type = oc_tcp4_transport_id;
    oe_ip->ep.oe_flags = 0;
    oe_ip->sock = ep_desc->sock;
    oe_ip->v4 = ep_desc->addr;
    oe_ip->port = ep_desc->port;
}

// XXX: Rename
int
oc_attempt_rx_tcp4(struct mn_socket *sock, struct os_mbuf *frag,
                   const struct mn_sockaddr_in *from)
{
    struct oc_tcp4_ep_desc ep_desc;
    struct os_mbuf *pkt;
    int rc;

    STATS_INC(oc_tcp4_stats, iframe);
    STATS_INCN(oc_tcp4_stats, ibytes, OS_MBUF_PKTLEN(frag));

    ep_desc.sock = sock;
    memcpy(&ep_desc.addr, &from->msin_addr, sizeof ep_desc.addr);
    ep_desc.port = ntohs(from->msin_port);

    rc = oc_tcp_reass(&oc_tcp4_r, frag, &ep_desc, &pkt);
    if (rc != 0) {
        if (rc == SYS_ENOMEM) {
            OC_LOG_ERROR("oc_tcp4_rx: Could not allocate mbuf\n");
        }
        STATS_INC(oc_tcp4_stats, ierr);
        return rc;
    }

    if (pkt != NULL) {
        STATS_INC(oc_tcp4_stats, iframe);
        oc_recv_message(pkt);
    }

    return 0;
}

static void oc_socks4_readable(void *cb_arg, int err);

union mn_socket_cb oc_tcp4_cbs = {
    .socket.readable = oc_socks4_readable,
    .socket.writable = NULL
};

void
oc_socks4_readable(void *cb_arg, int err)
{
    os_eventq_put(oc_evq_get(), &oc_tcp4_read_event);
}

void
oc_connectivity_shutdown_tcp4(void)
{
    if (oc_ucast4) {
        mn_close(oc_ucast4);
    }
}

static void
oc_event_tcp4(struct os_event *ev)
{
    struct mn_sockaddr_in from;
    struct oc_tcp4_conn *conn;
    struct os_mbuf *frag;
    int rc;

    SLIST_FOREACH(conn, &oc_tcp4_conn_list, next) {
        while (1) {
            rc = mn_recvfrom(conn->sock, &frag, (struct mn_sockaddr *) &from);
            if (rc != 0) {
                break;
            }
            assert(OS_MBUF_IS_PKTHDR(frag));

            oc_attempt_rx_tcp4(conn->sock, frag, &from);
        }
    }
}

int
oc_connectivity_init_tcp4(void)
{
#if 0
    int rc;
    struct mn_sockaddr_in sin;
    struct mn_itf itf;

    memset(&itf, 0, sizeof(itf));

    rc = mn_socket(&oc_ucast4, MN_PF_INET, MN_SOCK_STREAM, 0);
    if (rc != 0 || !oc_ucast4) {
        OC_LOG_ERROR("Could not create oc unicast v4 socket\n");
        return rc;
    }
    mn_socket_set_cbs(oc_ucast4, oc_ucast4, &oc_tcp4_cbs);

    sin.msin_len = sizeof(sin);
    sin.msin_family = MN_AF_INET;
    sin.msin_port = 0;
    memset(&sin.msin_addr, 0, sizeof(sin.msin_addr));

    rc = mn_bind(oc_ucast4, (struct mn_sockaddr *)&sin);
    if (rc != 0) {
        OC_LOG_ERROR("Could not bind oc unicast v4 socket\n");
        goto oc_connectivity_init_err;
    }

    // XXX: Listen

    return 0;

oc_connectivity_init_err:
    oc_connectivity_shutdown_tcp4();
    return rc;
#endif

return 0;
}

int
oc_tcp4_add_conn(struct mn_socket *sock)
{
    struct oc_tcp4_conn *conn;

    conn = os_memblock_get(&oc_tcp4_conn_pool);
    if (conn == NULL) {
        return SYS_ENOMEM;
    }

    *conn = (struct oc_tcp4_conn) {
        .sock = sock,
        .err_cb = NULL,
        .err_arg = NULL,
    };
    mn_socket_set_cbs(conn->sock, conn->sock, &oc_tcp4_cbs);

    SLIST_INSERT_HEAD(&oc_tcp4_conn_list, conn, next);

    return 0;
}

int
oc_tcp4_del_conn(struct mn_socket *sock)
{
    struct oc_tcp4_conn *conn;
    struct oc_tcp4_conn *prev;

    prev = NULL;
    SLIST_FOREACH(conn, &oc_tcp4_conn_list, next) {
        if (conn->sock == sock) {
            if (prev == NULL) {
                SLIST_REMOVE_HEAD(&oc_tcp4_conn_list, next);
            } else {
                SLIST_NEXT(prev, next) = SLIST_NEXT(conn, next);
            }

            return 0;
        }

        prev = conn;
    }

    return SYS_ENOENT;
}

#endif

void
oc_register_tcp4(void)
{
#if (MYNEWT_VAL(OC_TRANSPORT_IP) == 1) && (MYNEWT_VAL(OC_TRANSPORT_IPV4) == 1)
    int rc;

    SLIST_INIT(&oc_tcp4_conn_list);

    rc = os_mempool_init(&oc_tcp4_conn_pool, MYNEWT_VAL(OC_TCP4_MAX_CONNS),
                         sizeof (struct oc_tcp4_conn), oc_tcp4_conn_buf,
                         "oc_tcp4_conn_pool");
    SYSINIT_PANIC_ASSERT(rc == 0);

    oc_tcp4_transport_id = oc_transport_register(&oc_tcp4_transport);
#endif
}
