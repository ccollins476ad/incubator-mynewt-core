/*
Copyright (c) 2013, SEMTECH S.A.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the Semtech corporation nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL SEMTECH S.A. BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Description: Ping-Pong implementation.  Adapted to run in the MyNewt OS.
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

#define SPI_BAUDRATE                    500
#define USE_MODEM_LORA
#define USE_BAND_915

#if defined(USE_BAND_433)

#define RF_FREQUENCY                    434000000 /* Hz */

#elif defined(USE_BAND_780)

#define RF_FREQUENCY                    780000000 /* Hz */

#elif defined(USE_BAND_868)

#define RF_FREQUENCY                    868000000 /* Hz */

#elif defined(USE_BAND_915)

#define RF_FREQUENCY                    915000000 /* Hz */

#else
    #error "Please define a frequency band in the compiler options."
#endif

#define TX_OUTPUT_POWER                 14        /* dBm */

#if defined(USE_MODEM_LORA)

#define LORA_BANDWIDTH                  0         /* [0: 125 kHz, */
                                                  /*  1: 250 kHz, */
                                                  /*  2: 500 kHz, */
                                                  /*  3: Reserved] */
#define LORA_SPREADING_FACTOR           7         /* [SF7..SF12] */
#define LORA_CODINGRATE                 1         /* [1: 4/5, */
                                                  /*  2: 4/6, */
                                                  /*  3: 4/7, */
                                                  /*  4: 4/8] */
#define LORA_PREAMBLE_LENGTH            8         /* Same for Tx and Rx */
#define LORA_SYMBOL_TIMEOUT             5         /* Symbols */
#define LORA_FIX_LENGTH_PAYLOAD_ON      false
#define LORA_IQ_INVERSION_ON            false

#elif defined(USE_MODEM_FSK)

#define FSK_FDEV                        25e3      /* Hz */
#define FSK_DATARATE                    50e3      /* bps */
#define FSK_BANDWIDTH                   50e3      /* Hz */
#define FSK_AFC_BANDWIDTH               83.333e3  /* Hz */
#define FSK_PREAMBLE_LENGTH             5         /* Same for Tx and Rx */
#define FSK_FIX_LENGTH_PAYLOAD_ON       false

#else
    #error "Please define a modem in the compiler options."
#endif

#define RX_TIMEOUT_VALUE                1000
#define BUFFER_SIZE                     64

const uint8_t ping_msg[] = "PING";
const uint8_t pong_msg[] = "PONG";

volatile int go_tx;
volatile int go_rx;

static int8_t rssi_value;
static int8_t snr_value;
static uint8_t buffer[BUFFER_SIZE];
static int rx_size;
static int is_master = 1;

struct {
    int rx_timeout;
    int rx_ping;
    int rx_pong;
    int rx_other;
    int rx_error;
    int tx_timeout;
    int tx_success;
} loraping_stats;

static void loraping_tx(struct os_event *ev);
static void loraping_rx(struct os_event *ev);

static struct os_event loraping_ev_tx = {
    .ev_cb = loraping_tx,
};
static struct os_event loraping_ev_rx = {
    .ev_cb = loraping_rx,
};

static void
send_once(int is_ping)
{
    int i;

    if (is_ping) {
        memcpy(buffer, ping_msg, 4);
    } else {
        memcpy(buffer, pong_msg, 4);
    }
    for (i = 4; i < sizeof buffer; i++) {
        buffer[i] = i - 4;
    }

    Radio.Send(buffer, sizeof buffer);
}

static void
loraping_tx(struct os_event *ev)
{
    if (rx_size == 0) {
        /* Timeout. */
    } else {
        os_time_delay(1);
        if (memcmp(buffer, pong_msg, 4) == 0) {
            loraping_stats.rx_ping++;
        } else if (memcmp(buffer, ping_msg, 4) == 0) {
            loraping_stats.rx_pong++;

            /* A master already exists.  Become a slave. */
            is_master = 0;
        } else { 
            /* Valid reception but neither a PING nor a PONG message. */
            loraping_stats.rx_other++;
            /* Set device as master and start again. */
            is_master = 1;
        }
    }

    rx_size = 0;
    send_once(is_master);
}

static void
loraping_rx(struct os_event *ev)
{
    Radio.Rx(RX_TIMEOUT_VALUE);
}

static void
on_tx_done(void)
{
    loraping_stats.tx_success++;
    Radio.Sleep();

    os_eventq_put(os_eventq_dflt_get(), &loraping_ev_rx);
}

static void
on_rx_done(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
    Radio.Sleep();

    if (size > sizeof buffer) {
        size = sizeof buffer;
    }

    rx_size = size;
    memcpy(buffer, payload, size);
    rssi_value = rssi;
    snr_value = snr;

    os_eventq_put(os_eventq_dflt_get(), &loraping_ev_tx);
}

static void
on_tx_timeout(void)
{
    loraping_stats.tx_timeout++;
    Radio.Sleep();

    os_eventq_put(os_eventq_dflt_get(), &loraping_ev_rx);
}

static void
on_rx_timeout(void)
{
    loraping_stats.rx_timeout++;
    Radio.Sleep();

    os_eventq_put(os_eventq_dflt_get(), &loraping_ev_tx);
}

static void
on_rx_error(void)
{
    loraping_stats.rx_error++;
    Radio.Sleep();

    os_eventq_put(os_eventq_dflt_get(), &loraping_ev_tx);
}

static void
loraping_spi_cfg(void)
{
    struct hal_spi_settings my_spi;
    int rc;

    hal_gpio_init_out(MYNEWT_VAL(SPI_0_MASTER_SS_PIN), 1);

    my_spi.data_order = HAL_SPI_MSB_FIRST;
    my_spi.data_mode = HAL_SPI_MODE0;
    my_spi.baudrate = SPI_BAUDRATE;
    my_spi.word_size = HAL_SPI_WORD_SIZE_8BIT;

    rc = hal_spi_config(0, &my_spi);
    assert(rc == 0);

    rc = hal_spi_enable(0);
    assert(rc == 0);
}

int
main(void)
{
    RadioEvents_t radio_events;

#ifdef ARCH_sim
    mcu_sim_parse_args(argc, argv);
#endif

    sysinit();

    loraping_spi_cfg();

    /* Radio initialization. */
    radio_events.TxDone = on_tx_done;
    radio_events.RxDone = on_rx_done;
    radio_events.TxTimeout = on_tx_timeout;
    radio_events.RxTimeout = on_rx_timeout;
    radio_events.RxError = on_rx_error;

    Radio.Init(&radio_events);

    Radio.SetChannel(RF_FREQUENCY);

    Radio.SetTxConfig(MODEM_LORA,
                      TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                      LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                      LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                      true, 0, 0, LORA_IQ_INVERSION_ON, 3000);

    Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                      LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                      LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                      0, true, 0, 0, LORA_IQ_INVERSION_ON, true);

    /* Immediately send a ping on start up. */
    os_eventq_put(os_eventq_dflt_get(), &loraping_ev_tx);

    /*
     * As the last thing, process events from default event queue.
     */
    while (1) {
        os_eventq_run(os_eventq_dflt_get());
    }
}
