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

#include <syscfg/syscfg.h>

#include <hal/hal_bsp.h>
#include <bsp/bsp.h>
#include <mcu/hal_apollo.h>

#if MYNEWT_VAL(UART_0)
#include "uart/uart.h"
#include "uart_hal/uart_hal.h"
#endif


#if MYNEWT_VAL(UART_0)
static struct uart_dev os_bsp_uart0;
static const struct apollo_uart_cfg os_bsp_uart0_cfg = {
    .suc_pin_tx = MYNEWT_VAL(UART_0_PIN_TX),
    .suc_pin_rx = MYNEWT_VAL(UART_0_PIN_RX),
    .suc_pin_rts = MYNEWT_VAL(UART_0_PIN_RTS),
    .suc_pin_cts = MYNEWT_VAL(UART_0_PIN_CTS),
};
#endif

const struct hal_flash *
hal_bsp_flash_dev(uint8_t id)
{
    if (id != 0) {
        return (NULL);
    }
    return &apollo_flash_dev;
}


void
hal_bsp_init(void)
{
    int rc;

    (void) rc;

#if MYNEWT_VAL(UART_0)
    rc = os_dev_create((struct os_dev *) &os_bsp_uart0, "uart0",
            OS_DEV_INIT_PRIMARY, 0, uart_hal_init, (void *) &os_bsp_uart0_cfg);
    assert(rc == 0);
#endif

}

int
hal_bsp_hw_id(uint8_t *id, int max_len)
{
#if 0
    if (max_len > sizeof(DEVID)) {
        max_len = sizeof(DEVID);
    }

    memcpy(id, &DEVID, max_len);
    return max_len;
#endif
    return 0;
}
