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
#include <os/os.h>
#include <am_hal_flash.h>
#include <hal/hal_flash_int.h>
#include <hal/hal_flash.h>
#include <mcu/system_apollo1.h>


static int
apollo_flash_read(const struct hal_flash *dev, uint32_t address, void *dst,
    uint32_t num_bytes);
static int
apollo_flash_write(const struct hal_flash *dev, uint32_t address,
        const void *src, uint32_t num_bytes);
static int
apollo_flash_erase_sector(const struct hal_flash *dev, uint32_t sector_addr);
static int
apollo_flash_sector_info(const struct hal_flash *dev, int idx, uint32_t *addr,
    uint32_t *sz);
static int
apollo_flash_init(const struct hal_flash *dev);

static const struct hal_flash_funcs apollo_flash_funcs = {
    .hff_read = apollo_flash_read,
    .hff_write = apollo_flash_write,
    .hff_erase_sector = apollo_flash_erase_sector,
    .hff_sector_info = apollo_flash_sector_info,
    .hff_init = apollo_flash_init
};

const struct hal_flash apollo_flash_dev = {
        .hf_itf = &apollo_flash_funcs,
        .hf_base_addr = 0x00000000,
        .hf_size = 512 * 1024,
        .hf_sector_cnt = 256,
        .hf_align = 1
};

static int
apollo_flash_read(const struct hal_flash *dev, uint32_t address, void *dst,
    uint32_t num_bytes)
{
    memcpy(dst, (void *) address, num_bytes);

    return (0);
}

static int
apollo_flash_write(const struct hal_flash *dev, uint32_t address,
    const void *src, uint32_t num_bytes)
{
    uint32_t val;
    int words;
    int sr;
    int remainder;
    int rc;

    words = num_bytes / 4;
    remainder = num_bytes - (words * 4);

    __HAL_DISABLE_INTERRUPTS(sr);

    rc = am_hal_flash_program_main(AM_HAL_FLASH_PROGRAM_KEY, (uint32_t *) src, (uint32_t *) address, words);
    if (rc != 0) {
        goto err;
    }
    if (remainder > 0) {
        val = *((uint32_t *) address + words);
        memcpy((uint8_t * ) &val, (uint32_t *) src + words, remainder);
        rc = am_hal_flash_program_main(AM_HAL_FLASH_PROGRAM_KEY, &val, (uint32_t *) address + words, 1);
        if (rc != 0) {
            goto err;
        }
    }

    __HAL_ENABLE_INTERRUPTS(sr);

    return (0);
err:
    __HAL_ENABLE_INTERRUPTS(sr);
    return (rc);
}

static int
apollo_flash_erase_sector(const struct hal_flash *dev, uint32_t sector_addr)
{
    uint32_t block;
    uint32_t page;
    int rc;

    block = AM_HAL_FLASH_ADDR2BLOCK(sector_addr);
    page = AM_HAL_FLASH_ADDR2PAGE(sector_addr);

    rc = am_hal_flash_page_erase(AM_HAL_FLASH_PROGRAM_KEY, block, page);
    if (rc != 0) {
        goto err;
    }

    return (0);
err:
    return (rc);
}

static int
apollo_flash_sector_info(const struct hal_flash *dev, int idx, uint32_t *addr,
    uint32_t *sz)
{
    *addr = idx * AM_HAL_FLASH_PAGE_SIZE;
    *sz = AM_HAL_FLASH_PAGE_SIZE;

    return (0);
}

static int
apollo_flash_init(const struct hal_flash *dev)
{
    return (0);
}
