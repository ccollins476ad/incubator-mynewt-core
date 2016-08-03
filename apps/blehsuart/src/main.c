/**
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
#include "bsp/bsp.h"
#include "os/os.h"
#include "hal/hal_cputime.h"
#include "hal/hal_uart.h"
#include "console/console.h"

/* BLE */
#include "nimble/ble.h"
#include "nimble/nimble_opt.h"
#include "nimble/ble_hci_trans.h"
#include "transport/uart/ble_hci_uart.h"
#include "host/ble_hs.h"

/* RAM persistence layer. */
#include "store/ram/ble_store_ram.h"

/* Mandatory services. */
#include "services/mandatory/ble_svc_gap.h"
#include "services/mandatory/ble_svc_gatt.h"

/** blehsuart task settings. */
#define BLEHSUART_TASK_PRIO           1
#define BLEHSUART_STACK_SIZE          (OS_STACK_ALIGN(336))

struct os_eventq blehsuart_evq;
struct os_task blehsuart_task;
bssnz_t os_stack_t blehsuart_stack[BLEHSUART_STACK_SIZE];

/* Create a mbuf pool of BLE mbufs */
#define MBUF_NUM_MBUFS      (7)
#define MBUF_BUF_SIZE       OS_ALIGN(BLE_MBUF_PAYLOAD_SIZE, 4)
#define MBUF_MEMBLOCK_SIZE  (MBUF_BUF_SIZE + BLE_MBUF_MEMBLOCK_OVERHEAD)
#define MBUF_MEMPOOL_SIZE   OS_MEMPOOL_SIZE(MBUF_NUM_MBUFS, MBUF_MEMBLOCK_SIZE)

static os_membuf_t blehsuart_mbuf_mpool_data[MBUF_MEMPOOL_SIZE];
struct os_mbuf_pool blehsuart_mbuf_pool;
struct os_mempool blehsuart_mbuf_mpool;

/** Log data. */
//static struct log_handler blehsuart_log_console_handler;
struct log blehsuart_log;

#define HCI_MAX_BUFS        (5)

os_membuf_t default_mbuf_mpool_data[MBUF_MEMPOOL_SIZE];

struct os_mbuf_pool default_mbuf_pool;
struct os_mempool default_mbuf_mpool;

static void blehsuart_advertise(void);

/**
 * The nimble host executes this callback when a GAP event occurs.  The
 * application associates a GAP event callback with each connection that forms.
 * blehsuart uses the same callback for all connections.
 *
 * @param event                 The type of event being signalled.
 * @param ctxt                  Various information pertaining to the event.
 * @param arg                   Application-specified argument; unuesd by
 *                                  blehsuart.
 *
 * @return                      0 if the application successfully handled the
 *                                  event; nonzero on failure.  The semantics
 *                                  of the return code is specific to the
 *                                  particular GAP event being signalled.
 */
static int
blehsuart_gap_event(struct ble_gap_event *event, void *arg)
{
    struct ble_gap_conn_desc desc;
    int rc;

    switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:
        /* A new connection was established or a connection attempt failed. */
        if (event->connect.status == 0) {
            rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
            assert(rc == 0);
        }

        if (event->connect.status != 0) {
            /* Connection failed; resume advertising. */
            blehsuart_advertise();
        }
        return 0;

    case BLE_GAP_EVENT_DISCONNECT:
        /* Connection terminated; resume advertising. */
        blehsuart_advertise();
        return 0;

    case BLE_GAP_EVENT_CONN_UPDATE:
        /* The central has updated the connection parameters. */
        rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
        assert(rc == 0);
        return 0;

    case BLE_GAP_EVENT_ENC_CHANGE:
        /* Encryption has been enabled or disabled for this connection. */
        rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
        assert(rc == 0);
        return 0;

    case BLE_GAP_EVENT_SUBSCRIBE:
        return 0;
    }

    return 0;
}

/**
 * Enables advertising with the following parameters:
 *     o General discoverable mode.
 *     o Undirected connectable mode.
 */
static void
blehsuart_advertise(void)
{
    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;
    const char *name;
    int rc;

    /**
     *  Set the advertisement data included in our advertisements:
     *     o Flags (indicates advertisement type and other general info).
     *     o Advertising tx power.
     *     o Device name.
     *     o 16-bit service UUIDs (alert notifications).
     */

    memset(&fields, 0, sizeof fields);

    /* Indicate that the flags field should be included; specify a value of 0
     * to instruct the stack to fill the value in for us.
     */
    fields.flags_is_present = 1;
    fields.flags = 0;

    /* Indicate that the TX power level field should be included; have the
     * stack fill this one automatically as well.  This is done by assiging the
     * special value BLE_HS_ADV_TX_PWR_LVL_AUTO.
     */
    fields.tx_pwr_lvl_is_present = 1;
    fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

    name = ble_svc_gap_device_name();
    fields.name = (uint8_t *)name;
    fields.name_len = strlen(name);
    fields.name_is_complete = 1;

    rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0) {
        return;
    }

    /* Begin advertising. */
    memset(&adv_params, 0, sizeof adv_params);
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    rc = ble_gap_adv_start(BLE_ADDR_TYPE_PUBLIC, 0, NULL, BLE_HS_FOREVER,
                           &adv_params, blehsuart_gap_event, NULL);
    if (rc != 0) {
        return;
    }
}

static void
blehsuart_on_reset(int reason)
{
}

static void
blehsuart_on_sync(void)
{
    static const uint8_t rndaddr[6] = { 0x44, 0x44, 0x44, 0x44, 0x44, 0xcc };
    int rc;

    rc = ble_hs_id_set_rnd(rndaddr);
    assert(rc == 0);

    /* Begin advertising. */
    blehsuart_advertise();
}

/**
 * Event loop for the main blehsuart task.
 */
static void
blehsuart_task_handler(void *unused)
{
    struct os_event *ev;
    struct os_callout_func *cf;

    /* Activate the host.  This causes the host to synchronize with the
     * controller.
     */
    ble_hs_start();

    while (1) {
        ev = os_eventq_get(&blehsuart_evq);
        switch (ev->ev_type) {
        case OS_EVENT_T_TIMER:
            cf = (struct os_callout_func *)ev;
            assert(cf->cf_func);
            cf->cf_func(CF_ARG(cf));
            break;

        default:
            assert(0);
            break;
        }
    }
}

int
main(void)
{
    struct ble_hci_uart_cfg hci_cfg;
    struct ble_hs_cfg cfg;
    //uint32_t seed;
    int rc;
    //int i;

    /* Initialize OS */
    os_init();

    /* Set cputime to count at 1 usec increments */
    rc = cputime_init(1000000);
    assert(rc == 0);

#if 0
    /* Seed random number generator with least significant bytes of device
     * address.
     */
    seed = 0;
    for (i = 0; i < 4; ++i) {
        seed |= g_dev_addr[i];
        seed <<= 8;
    }
    srand(seed);
#endif

    /* Initialize msys mbufs. */
    rc = os_mempool_init(&blehsuart_mbuf_mpool, MBUF_NUM_MBUFS,
                         MBUF_MEMBLOCK_SIZE, blehsuart_mbuf_mpool_data,
                         "blehsuart_mbuf_data");
    assert(rc == 0);

    rc = os_mbuf_pool_init(&blehsuart_mbuf_pool, &blehsuart_mbuf_mpool,
                           MBUF_MEMBLOCK_SIZE, MBUF_NUM_MBUFS);
    assert(rc == 0);

    rc = os_msys_register(&blehsuart_mbuf_pool);
    assert(rc == 0);

    /* Initialize the console (for log output). */
    //rc = console_init(NULL);
    //assert(rc == 0);

    /* Initialize the logging system. */
    log_init();
    //log_console_handler_init(&blehsuart_log_console_handler);
    //log_register("blehsuart", &blehsuart_log, &blehsuart_log_console_handler);

    /* Initialize the eventq for the application task. */
    os_eventq_init(&blehsuart_evq);

    /* Create the blehsuart task.  All application logic and NimBLE host
     * operations are performed in this task.
     */
    os_task_init(&blehsuart_task, "blehsuart", blehsuart_task_handler,
                 NULL, BLEHSUART_TASK_PRIO, OS_WAIT_FOREVER,
                 blehsuart_stack, BLEHSUART_STACK_SIZE);

    /* Configure the host. */
    cfg = ble_hs_cfg_dflt;
    cfg.max_hci_bufs = 3;
    cfg.max_gattc_procs = 5;
    cfg.sm_bonding = 1;
    cfg.sm_our_key_dist = BLE_SM_PAIR_KEY_DIST_ENC;
    cfg.sm_their_key_dist = BLE_SM_PAIR_KEY_DIST_ENC;
    cfg.reset_cb = blehsuart_on_reset;
    cfg.sync_cb = blehsuart_on_sync;
    cfg.store_read_cb = ble_store_ram_read;
    cfg.store_write_cb = ble_store_ram_write;

    /* Initialize GATT services. */
    rc = ble_svc_gap_init(&cfg);
    assert(rc == 0);

    rc = ble_svc_gatt_init(&cfg);
    assert(rc == 0);

    /* Initialize the BLE host. */
    rc = ble_hs_init(&blehsuart_evq, &cfg);
    assert(rc == 0);

    hci_cfg = ble_hci_uart_cfg_dflt;
    rc = ble_hci_uart_init(&hci_cfg);
    assert(rc == 0);

    /* Set the default device name. */
    rc = ble_svc_gap_device_name_set("nimble-blehsuart");
    assert(rc == 0);

    /* Start the OS */
    os_start();

    /* os start should never return. If it does, this should be an error */
    assert(0);

    return 0;
}
