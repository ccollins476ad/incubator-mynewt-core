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

#include "hal/hal_uart.h"
#include "bsp/cmsis_nvic.h"
#include "bsp/bsp.h"

#include <am_mcu_apollo.h>

#include <mcu/system_apollo1.h>
#include "os/os_trace_api.h"

#include <mcu/hal_apollo.h>

#include <assert.h>

/* XXX: Evil, CMSIS defines "UART" which gets replaced here, and makes Ambiq's macros
 * mess up.  Undef UART so that we can access registers.
 */
#ifdef UART
#undef UART
#endif


/*
 * Only one UART on Ambiq Apollo 1
 */
struct hal_uart {
    uint8_t u_open:1;
    uint8_t u_rx_stall:1;
    uint8_t u_tx_started:1;
    uint8_t u_rx_buf;
    uint8_t u_tx_buf[1];
    hal_uart_rx_char u_rx_func;
    hal_uart_tx_char u_tx_func;
    hal_uart_tx_done u_tx_done;
    void *u_func_arg;
};
static struct hal_uart uart;

static inline void
__uart_enable_tx_interrupt(void)
{
    AM_REGn(UART, 0, IER) |= (AM_REG_UART_IER_TXIM_M);
}

static inline void
__uart_disable_tx_interrupt(void)
{
    AM_REGn(UART, 0, IER) &= ~(AM_REG_UART_IER_TXIM_M);
}


static inline void
__uart_enable_rx_interrupt(void)
{
    AM_REGn(UART, 0, IER) |= (AM_REG_UART_IER_RTIM_M |
            AM_REG_UART_IER_RXIM_M);
}

static inline void
__uart_disable_rx_interrupt(void)
{
    AM_REGn(UART, 0, IER) &= ~(AM_REG_UART_IER_RTIM_M |
            AM_REG_UART_IER_RXIM_M);
}


int
hal_uart_init_cbs(int port, hal_uart_tx_char tx_func, hal_uart_tx_done tx_done,
  hal_uart_rx_char rx_func, void *arg)
{
    struct hal_uart *u;

    if (port != 0) {
        return -1;
    }
    u = &uart;
    if (u->u_open) {
        return -1;
    }
    u->u_rx_func = rx_func;
    u->u_tx_func = tx_func;
    u->u_tx_done = tx_done;
    u->u_func_arg = arg;

    return 0;
}

void
hal_uart_start_tx(int port)
{
    struct hal_uart *u;
    int sr;
    int data;

    if (port != 0) {
        return;
    }
    u = &uart;

    __HAL_DISABLE_INTERRUPTS(sr);
    if (u->u_tx_started == 0) {
        while (1) {
            if (AM_BFRn(UART, 0, FR, TXFF)) {
                u->u_tx_started = 1;
                __uart_enable_tx_interrupt();
                break;
            }

            data = u->u_tx_func(u->u_func_arg);
            if (data < 0) {
                if (u->u_tx_done) {
                    u->u_tx_done(u->u_func_arg);
                }
                break;
            }

            AM_REGn(UART, 0, DR) = (char) data;
        }
    }
    __HAL_ENABLE_INTERRUPTS(sr);
}

void
hal_uart_start_rx(int port)
{
    struct hal_uart *u;
    int sr;
    int rc;

    if (port != 0) {
        return;
    }
    u = &uart;
    if (u->u_rx_stall) {
        __HAL_DISABLE_INTERRUPTS(sr);
        rc = u->u_rx_func(u->u_func_arg, u->u_rx_buf);
        if (rc == 0) {
            u->u_rx_stall = 0;
            __uart_enable_rx_interrupt();
        }

        __HAL_ENABLE_INTERRUPTS(sr);
    }
}

void
hal_uart_blocking_tx(int port, uint8_t data)
{
    struct hal_uart *u;

    if (port != 0) {
        return;
    }

    u = &uart;
    if (!u->u_open) {
        return;
    }

    while (AM_BFRn(UART, 0, FR, TXFF));
    AM_REGn(UART, 0, DR) = (char) data;
}

static void
uart_irq_handler(void)
{
    struct hal_uart *u;
    uint32_t status;
    int data;
    int rc;

    os_trace_enter_isr();

    u = &uart;

    status = AM_REGn(UART, 0, IES);
    AM_REGn(UART, 0, IEC) &= ~status;

    if (status & (AM_REG_UART_IES_TXRIS_M)) {
        if (u->u_tx_started) {
            while (1) {
                if (AM_BFRn(UART, 0, FR, TXFF)) {
                    break;
                }

                data = u->u_tx_func(u->u_func_arg);
                if (data < 0) {
                    if (u->u_tx_done) {
                        u->u_tx_done(u->u_func_arg);
                    }
                    __uart_disable_tx_interrupt();
                    u->u_tx_started = 0;
                    break;
                }

                AM_REGn(UART, 0, DR) = (char) data;
            }
        }
    }

    if (status & (AM_REG_UART_IES_RXRIS_M | AM_REG_UART_IES_RTRIS_M)) {
        /* Service receive buffer */
        while (!AM_BFRn(UART, 0, FR, RXFE)) {
            u->u_rx_buf = AM_REGn(UART, 0, DR);
            rc = u->u_rx_func(u->u_func_arg, u->u_rx_buf);
            if (rc < 0) {
                u->u_rx_stall = 1;
                goto done;
            }
        }
    }

done:
    os_trace_exit_isr();
}

int
hal_uart_init(int port, void *arg)
{
    uint32_t pincfg;
    struct apollo_uart_cfg *cfg;
    int rc;

    if (port != 0) {
        rc = -1;
        goto err;
    }
    cfg = (struct apollo_uart_cfg *)arg;

    switch (cfg->suc_pin_tx) {
        case 35:
        case 0:
        case 14:
            pincfg = AM_HAL_GPIO_FUNC(2);
            break;
        case 39:
            pincfg = AM_HAL_GPIO_FUNC(1);
            break;
        case 22:
            pincfg = AM_HAL_GPIO_FUNC(0);
            break;
        default:
            rc = -1;
            goto err;
    }
    am_hal_gpio_pin_config(cfg->suc_pin_tx, pincfg);

    switch (cfg->suc_pin_rx) {
        case 36:
        case 1:
        case 15:
            pincfg = AM_HAL_GPIO_FUNC(2);
            break;
        case 40:
            pincfg = AM_HAL_GPIO_FUNC(1);
            break;
        case 23:
            pincfg = AM_HAL_GPIO_FUNC(0);
            break;
        default:
            rc = -1;
            goto err;
    }
    pincfg |= AM_HAL_PIN_DIR_INPUT;
    am_hal_gpio_pin_config(cfg->suc_pin_rx, pincfg);

    switch (cfg->suc_pin_rts) {
        case 5:
        case 37:
            pincfg = AM_HAL_GPIO_FUNC(2);
            break;
        default:
            goto done;
    }
    am_hal_gpio_pin_config(cfg->suc_pin_rts, pincfg);

    switch (cfg->suc_pin_cts) {
        case 6:
        case 38:
            pincfg = AM_HAL_GPIO_FUNC(2) | AM_HAL_PIN_DIR_INPUT;
            break;
        default:
            goto done;
    }
    am_hal_gpio_pin_config(cfg->suc_pin_cts, pincfg);

done:
    NVIC_SetVector(UART_IRQn, (uint32_t)uart_irq_handler);
    return (0);
err:
    return (rc);
}

int
hal_uart_config(int port, int32_t baudrate, uint8_t databits, uint8_t stopbits,
  enum hal_uart_parity parity, enum hal_uart_flow_ctl flow_ctl)
{
    struct hal_uart *u;
    am_hal_uart_config_t uart_cfg;

    if (port != 0) {
        return -1;
    }

    u = &uart;
    if (u->u_open) {
        return -1;
    }

    switch (databits) {
        case 8:
            uart_cfg.ui32DataBits = AM_HAL_UART_DATA_BITS_8;
            break;
        case 7:
            uart_cfg.ui32DataBits = AM_HAL_UART_DATA_BITS_7;
            break;
        case 6:
            uart_cfg.ui32DataBits = AM_HAL_UART_DATA_BITS_6;
            break;
        case 5:
            uart_cfg.ui32DataBits = AM_HAL_UART_DATA_BITS_5;
            break;
        default:
            return (-1);
    }

    switch (stopbits) {
        case 2:
            uart_cfg.bTwoStopBits = true;
            break;
        case 1:
        case 0:
            uart_cfg.bTwoStopBits = false;
            break;
        default:
            return (-1);
    }

    switch (parity) {
    case HAL_UART_PARITY_NONE:
        uart_cfg.ui32Parity = AM_HAL_UART_PARITY_NONE;
        break;
    case HAL_UART_PARITY_ODD:
        uart_cfg.ui32Parity = AM_HAL_UART_PARITY_ODD;
    case HAL_UART_PARITY_EVEN:
        uart_cfg.ui32Parity = AM_HAL_UART_PARITY_EVEN;
        break;
    }

    switch (flow_ctl) {
    case HAL_UART_FLOW_CTL_NONE:
        uart_cfg.ui32FlowCtrl = AM_HAL_UART_FLOW_CTRL_NONE;
        break;
    case HAL_UART_FLOW_CTL_RTS_CTS:
        uart_cfg.ui32FlowCtrl = AM_HAL_UART_FLOW_CTRL_RTS_CTS;
        break;
    }

    uart_cfg.ui32BaudRate = baudrate;

    am_hal_uart_pwrctrl_enable(port);
    am_hal_uart_clock_enable(port);

    /* Disable the UART before configuring it */
    am_hal_uart_disable(port);

    am_hal_uart_config(port, &uart_cfg);

    am_hal_uart_fifo_config(port,
              AM_HAL_UART_TX_FIFO_1_8 | AM_HAL_UART_RX_FIFO_1_8);

    NVIC_EnableIRQ(UART_IRQn);

    am_hal_uart_enable(port);

    __uart_enable_rx_interrupt();

    u->u_rx_stall = 0;
    u->u_tx_started = 0;
    u->u_open = 1;

    return (0);
}

int
hal_uart_close(int port)
{
    struct hal_uart *u;

    u = &uart;

    if (port == 0) {
        u->u_open = 0;
        am_hal_uart_disable(port);
        am_hal_uart_clock_disable(port);
        am_hal_uart_pwrctrl_disable(port);
        return (0);
    }
    return (-1);
}
