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
#include <hal/flash_map.h>
#include <hal/hal_bsp.h>
#include <hal/hal_cputime.h>
#include <mcu/nrf52_hal.h>

#include <os/os_dev.h>

#include <uart/uart.h>
#include <uart_hal/uart_hal.h>
#include <hal/hal_spi.h>
#ifdef BSP_CFG_SPI_MASTER
#include "nrf_drv_spi.h"
#endif
#ifdef BSP_CFG_SPI_SLAVE
#include "nrf_drv_spis.h"
#endif
#include "nrf_drv_config.h"
#include <app_util_platform.h>

#include "init/init.h"

/* BLE */
#include "nimble/ble.h"
#include "controller/ble_ll.h"
#include "host/ble_hs.h"

/* RAM HCI transport. */
#include "transport/ram/ble_hci_ram.h"

/* RAM persistence layer. */
#include "store/ram/ble_store_ram.h"

/* Mandatory services. */
#include "services/mandatory/ble_svc_gap.h"
#include "services/mandatory/ble_svc_gatt.h"

/* Newtmgr include */
#include "newtmgr/newtmgr.h"
#include "nmgrble/newtmgr_ble.h"

#include "imgmgr/imgmgr.h"

/** Priority of the nimble host and controller tasks. */
#define BLE_LL_TASK_PRI             (OS_TASK_PRI_HIGHEST)

#define NEWTMGR_TASK_PRIO (4)
#define NEWTMGR_TASK_STACK_SIZE (OS_STACK_ALIGN(512))
os_stack_t newtmgr_stack[NEWTMGR_TASK_STACK_SIZE];

static struct flash_area bsp_flash_areas[] = {
    [FLASH_AREA_BOOTLOADER] = {
        .fa_flash_id = 0,       /* internal flash */
        .fa_off = 0x00000000,   /* beginning */
        .fa_size = (32 * 1024)
    },
    /* 2*16K and 1*64K sectors here */
    [FLASH_AREA_IMAGE_0] = {
        .fa_flash_id = 0,
        .fa_off = 0x00008000,
        .fa_size = (232 * 1024)
    },
    [FLASH_AREA_IMAGE_1] = {
        .fa_flash_id = 0,
        .fa_off = 0x00042000,
        .fa_size = (232 * 1024)
    },
    [FLASH_AREA_IMAGE_SCRATCH] = {
        .fa_flash_id = 0,
        .fa_off = 0x0007c000,
        .fa_size = (4 * 1024)
    },
    [FLASH_AREA_NFFS] = {
        .fa_flash_id = 0,
        .fa_off = 0x0007d000,
        .fa_size = (12 * 1024)
    }
};
static struct uart_dev hal_uart0;

void _close(int fd);

/*
 * Returns the flash map slot where the currently active image is located.
 * If executing from internal flash from fixed location, that slot would
 * be easy to find.
 * If images are in external flash, and copied to RAM for execution, then
 * this routine would have to figure out which one of those slots is being
 * used.
 */
int
bsp_imgr_current_slot(void)
{
    return FLASH_AREA_IMAGE_0;
}

void
bsp_init(void)
{
    uint32_t seed;
    int rc;
    int i;

#ifdef BSP_CFG_SPI_MASTER
    nrf_drv_spi_config_t spi_cfg = NRF_DRV_SPI_DEFAULT_CONFIG(0);
#endif
#ifdef BSP_CFG_SPI_SLAVE
    nrf_drv_spis_config_t spi_cfg = NRF_DRV_SPIS_DEFAULT_CONFIG(0);
#endif

    /*
     * XXX this reference is here to keep this function in.
     */
    _sbrk(0);
    _close(0);

    flash_area_init(bsp_flash_areas,
      sizeof(bsp_flash_areas) / sizeof(bsp_flash_areas[0]));

    rc = os_dev_create((struct os_dev *) &hal_uart0, "uart0",
      OS_DEV_INIT_PRIMARY, 0, uart_hal_init, (void *)bsp_uart_config());
    assert(rc == 0);

#ifdef BSP_CFG_SPI_MASTER
    /*  We initialize one SPI interface as a master. */
    rc = hal_spi_init(0, &spi_cfg, HAL_SPI_TYPE_MASTER);
    assert(rc == 0);
#endif

#ifdef BSP_CFG_SPI_SLAVE
    /*  We initialize one SPI interface as a master. */
    spi_cfg.csn_pin = SPIS0_CONFIG_CSN_PIN;
    rc = hal_spi_init(0, &spi_cfg, HAL_SPI_TYPE_SLAVE);
    assert(rc == 0);
#endif

    /* Set cputime to count at 1 usec increments */
    rc = cputime_init(MN_CFG_CLOCK_FREQ);
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

    init_msys();

    /* Initialize the console (for log output). */
    rc = console_init(NULL);
    assert(rc == 0);

    /* Initialize the logging system. */
    log_init();

    /* Initialize the BLE LL */
    rc = ble_ll_init(BLE_LL_TASK_PRI, BLE_LL_NUM_ACL_PKTS,
                     BLE_LL_ACL_PKT_SIZE);
    assert(rc == 0);

    /* Initialize the RAM HCI transport. */
    rc = ble_hci_ram_init(&ble_hci_ram_cfg_dflt);
    assert(rc == 0);

    /* Initialize the NimBLE host configuration. */
    ble_hs_cfg.sm_bonding = 1;
    ble_hs_cfg.sm_our_key_dist = BLE_SM_PAIR_KEY_DIST_ENC;
    ble_hs_cfg.sm_their_key_dist = BLE_SM_PAIR_KEY_DIST_ENC;
    ble_hs_cfg.store_read_cb = ble_store_ram_read;
    ble_hs_cfg.store_write_cb = ble_store_ram_write;

    /* Initialize GATT services. */
    rc = ble_svc_gap_init();
    assert(rc == 0);

    rc = ble_svc_gatt_init();
    assert(rc == 0);

    rc = nmgr_ble_gatt_svr_init();
    assert(rc == 0);

    /* Initialize NimBLE host. */
    rc = ble_hs_init();
    assert(rc == 0);

    nmgr_task_init(NEWTMGR_TASK_PRIO, newtmgr_stack, NEWTMGR_TASK_STACK_SIZE);
    imgmgr_module_init();
}
