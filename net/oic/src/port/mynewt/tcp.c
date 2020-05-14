#include "os/mynewt.h"
#include "oic/port/mynewt/tcp.h"
#include "oic/messaging/coap/coap.h"

int
oc_tcp_reass(struct oc_tcp_reassembler *r, struct os_mbuf *om1, void *arg,
             struct os_mbuf **out_pkt)
{
    struct os_mbuf_pkthdr *pkt1;
    struct os_mbuf_pkthdr *pkt2;
    struct os_mbuf *om2;
    uint8_t hdr[sizeof (struct coap_tcp_hdr32)];

    *out_pkt = NULL;

    pkt1 = OS_MBUF_PKTHDR(om1);
    assert(pkt1);

    /* Find the packet that this fragment belongs to, if any. */
    STAILQ_FOREACH(pkt2, &r->pkt_q, omp_next) {
        om2 = OS_MBUF_PKTHDR_TO_MBUF(pkt2);
        if (r->ep_match(OS_MBUF_USRHDR(om2), arg)) {
            /* Data from same connection.  Append. */
            os_mbuf_concat(om2, om1);
            os_mbuf_copydata(om2, 0, sizeof(hdr), hdr);

            if (coap_tcp_msg_size(hdr, sizeof(hdr)) <= pkt2->omp_len) {
                /* Packet complete. */
                STAILQ_REMOVE(&r->pkt_q, pkt2, os_mbuf_pkthdr, omp_next);
                *out_pkt = om2;
            }

            return 0;
        }
    }

    /* New frame, need to add endpoint to the front.  Check if there is enough
     * space available. If not, allocate a new pkthdr.
     */
    if (OS_MBUF_USRHDR_LEN(om1) < r->endpoint_size) {
        om2 = os_msys_get_pkthdr(0, r->endpoint_size);
        if (!om2) {
            return SYS_ENOMEM;
        }
        OS_MBUF_PKTHDR(om2)->omp_len = pkt1->omp_len;
        SLIST_NEXT(om2, om_next) = om1;
    } else {
        om2 = om1;
    }

    r->ep_fill(OS_MBUF_USRHDR(om2), arg);
    pkt2 = OS_MBUF_PKTHDR(om2);

    os_mbuf_copydata(om2, 0, sizeof(hdr), hdr);
    if (coap_tcp_msg_size(hdr, sizeof(hdr)) > pkt2->omp_len) {
        STAILQ_INSERT_TAIL(&r->pkt_q, pkt2, omp_next);
    } else {
        *out_pkt = om2;
    }
    return 0;
}

#if 0
void
oc_tcp_conn_del(uint16_t conn_handle)
{
    struct os_mbuf_pkthdr *pkt;
    struct os_mbuf *m;
    struct oc_endpoint_ble *oe_ble;
    struct oc_conn_ev *oce;

    STAILQ_FOREACH(pkt, &oc_ble_reass_q, omp_next) {
        m = OS_MBUF_PKTHDR_TO_MBUF(pkt);
        oe_ble = (struct oc_endpoint_ble *)OC_MBUF_ENDPOINT(m);
        if (oe_ble->conn_handle == conn_handle) {
            STAILQ_REMOVE(&oc_ble_reass_q, pkt, os_mbuf_pkthdr, omp_next);
            os_mbuf_free_chain(m);
            break;
        }
    }

    /*
     * Notify listeners that this connection is gone.
     */
    oce = oc_conn_ev_alloc();
    assert(oce);
    memset(&oce->oce_oe, 0, sizeof(oce->oce_oe));
    oe_ble = (struct oc_endpoint_ble *)&oce->oce_oe;
    oe_ble->ep.oe_type = oc_gatt_transport_id;
    oe_ble->ep.oe_flags = 0;
    oe_ble->conn_handle = conn_handle;
    oc_conn_removed(oce);
}
#endif
