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
#include "node/radio.h"
#include "console/console.h"
#include "shell/shell.h"
#include "parse/parse.h"

#define LORASHELL_NUM_RX_ENTRIES    10

struct lorashell_rx_entry {
    int16_t rssi;
    int8_t snr;
};

static struct lorashell_rx_entry
    lorashell_rx_entries[LORASHELL_NUM_RX_ENTRIES];
static int lorashell_rx_entry_idx;
static int lorashell_rx_entry_cnt;

static int lorashell_rx_rpt;
static int lorashell_rx_verbose;

static int lorashell_rx_info_cmd(int argc, char **argv);
static int lorashell_rx_rpt_cmd(int argc, char **argv);
static int lorashell_rx_verbose_cmd(int argc, char **argv);

static void lorashell_print_last_rx(struct os_event *ev);

static struct shell_cmd lorashell_cli_cmds[] = {
    {
        .sc_cmd = "lora_rx_info",
        .sc_cmd_func = lorashell_rx_info_cmd,
    },
    {
        .sc_cmd = "lora_rx_rpt",
        .sc_cmd_func = lorashell_rx_rpt_cmd,
    },
    {
        .sc_cmd = "lora_rx_verbose",
        .sc_cmd_func = lorashell_rx_verbose_cmd,
    },
};
#define LORASHELL_NUM_CLI_CMDS  \
    (sizeof lorashell_cli_cmds / sizeof lorashell_cli_cmds[0])

static struct os_event lorashell_print_last_rx_ev = {
    .ev_cb = lorashell_print_last_rx,
};

static void
lorashell_rx_rpt_begin(void)
{
    Radio.Rx(0);
}

static const char *
lorashell_rx_entry_str(const struct lorashell_rx_entry *entry)
{
    static char buf[32];

    snprintf(buf, sizeof buf, "rssi=%-d snr=%-d", entry->rssi, entry->snr);
    return buf;
}

static void
on_tx_done(void)
{
    Radio.Sleep();
}

static void
on_rx_done(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{

    struct lorashell_rx_entry *entry;

    if (!lorashell_rx_rpt) {
        Radio.Sleep();
    } else {
        lorashell_rx_rpt_begin();
    }

    entry = lorashell_rx_entries + lorashell_rx_entry_idx;
    entry->rssi = rssi;
    entry->snr = snr;

    if (lorashell_rx_verbose) {
        os_eventq_put(os_eventq_dflt_get(), &lorashell_print_last_rx_ev);
    }

    lorashell_rx_entry_idx++;
    if (lorashell_rx_entry_idx >= LORASHELL_NUM_RX_ENTRIES) {
        lorashell_rx_entry_idx = 0;
    }

    if (lorashell_rx_entry_cnt < LORASHELL_NUM_RX_ENTRIES) {
        lorashell_rx_entry_cnt++;
    }
}

static void
on_tx_timeout(void)
{
    Radio.Sleep();
}

static void
on_rx_timeout(void)
{
    Radio.Sleep();
}

static void
on_rx_error(void)
{
    Radio.Sleep();
}

static void
lorashell_print_last_rx(struct os_event *ev)
{
    const struct lorashell_rx_entry *entry;
    int idx;

    idx = lorashell_rx_entry_idx - 1;
    if (idx < 0) {
        idx = lorashell_rx_entry_cnt - 1;
    }

    entry = lorashell_rx_entries + idx;
    console_printf("rxed lora packet: %s\n", lorashell_rx_entry_str(entry));
}

static void
lorashell_avg_rx_entry(struct lorashell_rx_entry *out_entry)
{
    long long rssi_sum;
    long long snr_sum;
    int i;

    rssi_sum = 0;
    snr_sum = 0;
    for (i = 0; i < lorashell_rx_entry_cnt; i++) {
        rssi_sum += lorashell_rx_entries[i].rssi;
        snr_sum += lorashell_rx_entries[i].snr;
    }

    memset(out_entry, 0, sizeof *out_entry);
    if (lorashell_rx_entry_cnt > 0) {
        out_entry->rssi = rssi_sum / lorashell_rx_entry_cnt;
        out_entry->snr = snr_sum / lorashell_rx_entry_cnt;
    }
}

static int
lorashell_rx_rpt_cmd(int argc, char **argv)
{
    if (argc > 1 && strcmp(argv[1], "stop") == 0) {
        lorashell_rx_rpt = 0;
        Radio.Sleep();
        console_printf("lora rx stopped\n");
        return 0;
    }

    lorashell_rx_rpt = 1;
    lorashell_rx_rpt_begin();

    return 0;
}

static int
lorashell_rx_verbose_cmd(int argc, char **argv)
{
    int rc;

    if (argc <= 1) {
        console_printf("lora rx verbose: %d\n", lorashell_rx_verbose);
        return 0;
    }

    lorashell_rx_verbose = parse_ull_bounds(argv[1], 0, 1, &rc);
    if (rc != 0) {
        console_printf("error: rc=%d\n", rc);
        return rc;
    }

    return 0;
}

static int
lorashell_rx_info_cmd(int argc, char **argv)
{
    const struct lorashell_rx_entry *entry;
    struct lorashell_rx_entry avg;
    int idx;
    int i;

    if (argc > 1 && argv[1][0] == 'c') {
        lorashell_rx_entry_idx = 0;
        lorashell_rx_entry_cnt = 0;
        console_printf("lora rx info cleared\n");
        return 0;
    }

    if (lorashell_rx_entry_cnt < LORASHELL_NUM_RX_ENTRIES) {
        idx = 0;
    } else {
        idx = lorashell_rx_entry_idx;
    }

    console_printf("entries in log: %d\n", lorashell_rx_entry_cnt);

    for (i = 0; i < lorashell_rx_entry_cnt; i++) {
        entry = lorashell_rx_entries + idx;
        console_printf("%4d: %s\n", i + 1, lorashell_rx_entry_str(entry));

        idx++;
        if (idx >= LORASHELL_NUM_RX_ENTRIES) {
            idx = 0;
        }
    }

    if (lorashell_rx_entry_cnt > 0) {
        lorashell_avg_rx_entry(&avg);
        console_printf(" avg: %s\n", lorashell_rx_entry_str(&avg));
    }

    return 0;
}

int
main(void)
{
    RadioEvents_t radio_events;
    int rc;
    int i;

#ifdef ARCH_sim
    mcu_sim_parse_args(argc, argv);
#endif

    sysinit();

    for (i = 0; i < LORASHELL_NUM_CLI_CMDS; i++) {
        rc = shell_cmd_register(lorashell_cli_cmds + i);
        SYSINIT_PANIC_ASSERT_MSG(
            rc == 0, "Failed to register lorashell CLI commands");
    }

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
