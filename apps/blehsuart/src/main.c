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
#include "console/console.h"

/* BLE */
#include "nimble/ble.h"
#include "nimble/nimble_opt.h"
#include "nimble/hci_transport.h"
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

/* Our global device address (public) */
uint8_t g_dev_addr[BLE_DEV_ADDR_LEN] = { 0 };

/* Our random address (in case we need it) */
uint8_t g_random_addr[BLE_DEV_ADDR_LEN] = { 0 };

#define HCI_MAX_BUFS        (5)

os_membuf_t default_mbuf_mpool_data[MBUF_MEMPOOL_SIZE];

struct os_mbuf_pool default_mbuf_pool;
struct os_mempool default_mbuf_mpool;

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
    struct ble_hs_cfg cfg;
    uint32_t seed;
    int rc;
    int i;

    /* Initialize OS */
    os_init();

    /* Set cputime to count at 1 usec increments */
    rc = cputime_init(1000000);
    assert(rc == 0);

    /* Seed random number generator with least significant bytes of device
     * address.
     */
    seed = 0;
    for (i = 0; i < 4; ++i) {
        seed |= g_dev_addr[i];
        seed <<= 8;
    }
    srand(seed);

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

    rc = ble_hci_uart_init(cfg.max_hci_bufs, 260);
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
