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

#include <string.h>
#include "sysinit/sysinit.h"
#include "syscfg/syscfg.h"
#include "hal/hal_gpio.h"
#include "hal/hal_spi.h"
#include "bsp/bsp.h"
#include "os/os.h"
#include "board/board.h"
#include "loramac-node/radio.h"
#include "console/console.h"

struct {
    int rx_timeout;
    int rx_success;
    int rx_error;
    int tx_timeout;
    int tx_success;
} lorashell_stats;

static void
on_tx_done(void)
{
    lorashell_stats.tx_success++;
    Radio.Sleep();
}

static void
on_rx_done(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
    Radio.Sleep();
    lorashell_stats.rx_success++;
}

static void
on_tx_timeout(void)
{
    Radio.Sleep();

    lorashell_stats.tx_timeout++;
}

static void
on_rx_timeout(void)
{
    Radio.Sleep();

    lorashell_stats.rx_timeout++;
}

static void
on_rx_error(void)
{
    Radio.Sleep();
    lorashell_stats.rx_error++;
}

int
main(void)
{
    RadioEvents_t radio_events;

#ifdef ARCH_sim
    mcu_sim_parse_args(argc, argv);
#endif

    sysinit();

    /* Radio initialization. */
    radio_events.TxDone = on_tx_done;
    radio_events.RxDone = on_rx_done;
    radio_events.TxTimeout = on_tx_timeout;
    radio_events.RxTimeout = on_rx_timeout;
    radio_events.RxError = on_rx_error;

    Radio.Init(&radio_events);

    console_printf("lorashell\n");

    /*
     * As the last thing, process events from default event queue.
     */
    while (1) {
        os_eventq_run(os_eventq_dflt_get());
    }
}
