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

#include "os/os_mempool.h"
#include "util/mem.h"

#if MYNEWT_MSYS_1_BLOCK_COUNT > 0
#define SYSINIT_MSYS_1_MEMBLOCK_SIZE    OS_ALIGN(MYNEWT_MSYS_1_BLOCK_SIZE, 4)
#define SYSINIT_MSYS_1_MEMPOOL_SIZE     \
    OS_MEMPOOL_SIZE(MYNEWT_MSYS_1_BLOCK_COUNT, SYSINIT_MSYS_1_MEMBLOCK_SIZE)
static os_membuf_t init_msys_1_data[SYSINIT_MSYS_1_MEMPOOL_SIZE];
static struct os_mbuf_pool init_msys_1_mbuf_pool;
static struct os_mempool init_msys_1_mempool;
#endif

#if MYNEWT_MSYS_2_BLOCK_COUNT > 0
#define SYSINIT_MSYS_2_MEMBLOCK_SIZE    OS_ALIGN(MYNEWT_MSYS_2_BLOCK_SIZE, 4)
#define SYSINIT_MSYS_2_MEMPOOL_SIZE     \
    OS_MEMPOOL_SIZE(MYNEWT_MSYS_2_BLOCK_COUNT, SYSINIT_MSYS_2_MEMBLOCK_SIZE)
static os_membuf_t init_msys_2_data[SYSINIT_MSYS_2_MEMPOOL_SIZE];
static struct os_mbuf_pool init_msys_2_mbuf_pool;
static struct os_mempool init_msys_2_mempool;
#endif

#if MYNEWT_MSYS_3_BLOCK_COUNT > 0
#define SYSINIT_MSYS_3_MEMBLOCK_SIZE    OS_ALIGN(MYNEWT_MSYS_3_BLOCK_SIZE, 4)
#define SYSINIT_MSYS_3_MEMPOOL_SIZE     \
    OS_MEMPOOL_SIZE(MYNEWT_MSYS_3_BLOCK_COUNT, SYSINIT_MSYS_3_MEMBLOCK_SIZE)
static os_membuf_t init_msys_3_data[SYSINIT_MSYS_3_MEMPOOL_SIZE];
static struct os_mbuf_pool init_msys_3_mbuf_pool;
static struct os_mempool init_msys_3_mempool;
#endif

#if MYNEWT_MSYS_4_BLOCK_COUNT > 0
#define SYSINIT_MSYS_4_MEMBLOCK_SIZE    OS_ALIGN(MYNEWT_MSYS_4_BLOCK_SIZE, 4)
#define SYSINIT_MSYS_4_MEMPOOL_SIZE     \
    OS_MEMPOOL_SIZE(MYNEWT_MSYS_4_BLOCK_COUNT, SYSINIT_MSYS_4_MEMBLOCK_SIZE)
static os_membuf_t init_msys_4_data[SYSINIT_MSYS_4_MEMPOOL_SIZE];
static struct os_mbuf_pool init_msys_4_mbuf_pool;
static struct os_mempool init_msys_4_mempool;
#endif

#if MYNEWT_MSYS_5_BLOCK_COUNT > 0
#define SYSINIT_MSYS_5_MEMBLOCK_SIZE    OS_ALIGN(MYNEWT_MSYS_5_BLOCK_SIZE, 4)
#define SYSINIT_MSYS_5_MEMPOOL_SIZE     \
    OS_MEMPOOL_SIZE(MYNEWT_MSYS_5_BLOCK_COUNT, SYSINIT_MSYS_5_MEMBLOCK_SIZE)
static os_membuf_t init_msys_5_data[SYSINIT_MSYS_5_MEMPOOL_SIZE];
static struct os_mbuf_pool init_msys_5_mbuf_pool;
static struct os_mempool init_msys_5_mempool;
#endif

static int
init_msys_once(void *data, struct os_mempool *mempool,
               struct os_mbuf_pool *mbuf_pool, int block_count, int block_size,
               char *name)
{
    int rc;

    rc = mem_init_mbuf_pool(data, mempool, mbuf_pool, block_count, block_size,
                            name);
    if (rc != 0) {
        return rc;
    }

    rc = os_msys_register(mbuf_pool);
    if (rc != 0) {
        return rc;
    }

    return 0;
}

int
init_msys(void)
{
    int rc;

    (void)rc;

#if MYNEWT_MSYS_1_BLOCK_COUNT > 0
    rc = init_msys_once(init_msys_1_data,
                        &init_msys_1_mempool,
                        &init_msys_1_mbuf_pool,
                        MYNEWT_MSYS_1_BLOCK_COUNT,
                        SYSINIT_MSYS_1_MEMBLOCK_SIZE,
                        "msys_1");
    if (rc != 0) {
        return rc;
    }
#endif

#if MYNEWT_MSYS_2_BLOCK_COUNT > 0
    rc = init_msys_once(init_msys_2_data,
                        &init_msys_2_mempool,
                        &init_msys_2_mbuf_pool,
                        MYNEWT_MSYS_2_BLOCK_COUNT,
                        SYSINIT_MSYS_2_MEMBLOCK_SIZE,
                        "msys_2");
    if (rc != 0) {
        return rc;
    }
#endif

#if MYNEWT_MSYS_3_BLOCK_COUNT > 0
    rc = init_msys_once(init_msys_3_data,
                        &init_msys_3_mempool,
                        &init_msys_3_mbuf_pool,
                        MYNEWT_MSYS_3_BLOCK_COUNT,
                        SYSINIT_MSYS_3_MEMBLOCK_SIZE,
                        "msys_3");
    if (rc != 0) {
        return rc;
    }
#endif

#if MYNEWT_MSYS_4_BLOCK_COUNT > 0
    rc = init_msys_once(init_msys_4_data,
                        &init_msys_4_mempool,
                        &init_msys_4_mbuf_pool,
                        MYNEWT_MSYS_4_BLOCK_COUNT,
                        SYSINIT_MSYS_4_MEMBLOCK_SIZE,
                        "msys_4");
    if (rc != 0) {
        return rc;
    }
#endif

#if MYNEWT_MSYS_5_BLOCK_COUNT > 0
    rc = init_msys_once(init_msys_5_data,
                        &init_msys_5_mempool,
                        &init_msys_5_mbuf_pool,
                        MYNEWT_MSYS_5_BLOCK_COUNT,
                        SYSINIT_MSYS_5_MEMBLOCK_SIZE,
                        "msys_5");
    if (rc != 0) {
        return rc;
    }
#endif

    return 0;
}
