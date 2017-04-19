/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Ping-Pong implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
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

static void loraping_tx(struct os_event *ev);
static void loraping_rx(struct os_event *ev);

static struct os_event loraping_ev_tx = {
    .ev_cb = loraping_tx,
};
static struct os_event loraping_ev_rx = {
    .ev_cb = loraping_rx,
};

#define SPI_BAUDRATE 500
#define USE_MODEM_LORA
#define USE_BAND_915
//#define USE_BAND_868

#if defined( USE_BAND_433 )

#define RF_FREQUENCY                                434000000 // Hz

#elif defined( USE_BAND_780 )

#define RF_FREQUENCY                                780000000 // Hz

#elif defined( USE_BAND_868 )

#define RF_FREQUENCY                                868000000 // Hz

#elif defined( USE_BAND_915 )

#define RF_FREQUENCY                                915000000 // Hz

#else
    #error "Please define a frequency band in the compiler options."
#endif

#define TX_OUTPUT_POWER                             14        // dBm

#if defined( USE_MODEM_LORA )

#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         5         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false

#elif defined( USE_MODEM_FSK )

#define FSK_FDEV                                    25e3      // Hz
#define FSK_DATARATE                                50e3      // bps
#define FSK_BANDWIDTH                               50e3      // Hz
#define FSK_AFC_BANDWIDTH                           83.333e3  // Hz
#define FSK_PREAMBLE_LENGTH                         5         // Same for Tx and Rx
#define FSK_FIX_LENGTH_PAYLOAD_ON                   false

#else
    #error "Please define a modem in the compiler options."
#endif

#if 0
typedef enum
{
    LOWPOWER,
    RX,
    RX_TIMEOUT,
    RX_ERROR,
    TX,
    TX_TIMEOUT,
}States_t;
#endif

struct {
    int rx_timeout;
    int rx_ping;
    int rx_pong;
    int rx_other;
    int rx_error;
    int tx_timeout;
    int tx_success;
} loraping_stats;

#define RX_TIMEOUT_VALUE                            (1000)
#define BUFFER_SIZE                                 64 // Define the payload size here

const uint8_t PingMsg[] = "PING";
const uint8_t PongMsg[] = "PONG";

int rx_size;
uint8_t Buffer[BUFFER_SIZE];

//States_t State = LOWPOWER;
volatile int go_tx;
volatile int go_rx;

int8_t RssiValue = 0;
int8_t SnrValue = 0;

/*!
 * Radio events function pointer
 */
static RadioEvents_t RadioEvents;

/*!
 * \brief Function to be executed on Radio Tx Done event
 */
void OnTxDone( void );

/*!
 * \brief Function to be executed on Radio Rx Done event
 */
void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );

/*!
 * \brief Function executed on Radio Tx Timeout event
 */
void OnTxTimeout( void );

/*!
 * \brief Function executed on Radio Rx Timeout event
 */
void OnRxTimeout( void );

/*!
 * \brief Function executed on Radio Rx Error event
 */
void OnRxError( void );

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

bool isMaster = true;

static void
send_once(int is_ping)
{
    if (is_ping) {
        // Send the next PING frame
        memcpy(Buffer, PingMsg, 4);
    } else {
        memcpy(Buffer, PongMsg, 4);
    }

    Radio.Send(Buffer, sizeof Buffer);
}

static void
loraping_tx(struct os_event *ev)
{
    if (rx_size == 0) {
        /* Timeout. */
    } else {
        os_time_delay(1);
        if (memcmp(Buffer, PongMsg, 4) == 0) {
            loraping_stats.rx_ping++;
        } else if (memcmp(Buffer, PingMsg, 4) == 0) {
            loraping_stats.rx_pong++;

            // A master already exists then become a slave
            isMaster = false;
        } else { 
            // valid reception but neither a PING or a PONG message
            loraping_stats.rx_other++;
            // Set device as master and start again
            isMaster = true;
        }
    }

    rx_size = 0;
    send_once(isMaster);
}

static void
loraping_rx(struct os_event *ev)
{
    Radio.Rx(RX_TIMEOUT_VALUE);
}

void OnTxDone( void )
{
    loraping_stats.tx_success++;
    Radio.Sleep( );

    os_eventq_put(os_eventq_dflt_get(), &loraping_ev_rx);
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    Radio.Sleep( );
    assert(size <= sizeof Buffer);
    rx_size = size;
    memcpy(Buffer, payload, size);
    RssiValue = rssi;
    SnrValue = snr;

    os_eventq_put(os_eventq_dflt_get(), &loraping_ev_tx);
}

void OnTxTimeout( void )
{
    loraping_stats.tx_timeout++;
    Radio.Sleep( );

    //os_eventq_put(os_eventq_dflt_get(), &loraping_ev_tx);
    os_eventq_put(os_eventq_dflt_get(), &loraping_ev_rx);
}

void OnRxTimeout( void )
{
    loraping_stats.rx_timeout++;
    Radio.Sleep( );

    os_eventq_put(os_eventq_dflt_get(), &loraping_ev_tx);
}

void OnRxError( void )
{
    loraping_stats.rx_error++;
    Radio.Sleep( );

    os_eventq_put(os_eventq_dflt_get(), &loraping_ev_tx);
}

int
main(void)
{
#ifdef ARCH_sim
    mcu_sim_parse_args(argc, argv);
#endif

    sysinit();

    loraping_spi_cfg();

    // Radio initialization
    RadioEvents.TxDone = OnTxDone;
    RadioEvents.RxDone = OnRxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxTimeout = OnRxTimeout;
    RadioEvents.RxError = OnRxError;

    Radio.Init(&RadioEvents);

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

    hal_gpio_read(RADIO_DIO_0);

    os_eventq_put(os_eventq_dflt_get(), &loraping_ev_tx);

    /*
     * As the last thing, process events from default event queue.
     */
    while (1) {
        os_eventq_run(os_eventq_dflt_get());
    }
}
