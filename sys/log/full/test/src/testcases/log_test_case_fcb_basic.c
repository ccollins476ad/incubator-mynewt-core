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

#include "log_test.h"

static char *str_logs[] = {
    "testdata",
    "1testdata2",
    NULL
};

static int ltcfb_str_idx = 0;
static int ltcfb_str_max_idx = 0;

static int
ltcfb_walk1(struct log *log, struct log_offset *log_offset,
            void *dptr, uint16_t len)
{
    int rc;
    struct log_entry_hdr ueh;
    char data[128];
    int dlen;

    TEST_ASSERT(ltcfb_str_idx < ltcfb_str_max_idx);

    rc = log_read(log, dptr, &ueh, 0, sizeof(ueh));
    TEST_ASSERT(rc == sizeof(ueh));

    dlen = len - sizeof(ueh);
    TEST_ASSERT(dlen < sizeof(data));

    rc = log_read(log, dptr, data, sizeof(ueh), dlen);
    TEST_ASSERT(rc == dlen);

    data[rc] = '\0';

    TEST_ASSERT(strlen(str_logs[ltcfb_str_idx]) == dlen);
    TEST_ASSERT(!memcmp(str_logs[ltcfb_str_idx], data, dlen));
    ltcfb_str_idx++;

    return 0;
}

static int
ltcfb_walk2(struct log *log, struct log_offset *log_offset,
            void *dptr, uint16_t len)
{
    TEST_ASSERT(0);
    return 0;
}

TEST_CASE(log_test_case_fcb_basic)
{
    struct log_offset log_offset = { 0 };
    char *str;
    int rc;

    ltu_setup_fcb();

    while (1) {
        str = str_logs[ltcfb_str_max_idx];
        if (!str) {
            break;
        }
        log_printf(&my_log, 0, 0, str, strlen(str));
        ltcfb_str_max_idx++;
    }

    ltcfb_str_idx = 0;

    rc = log_walk(&my_log, ltcfb_walk1, &log_offset);
    TEST_ASSERT(rc == 0);
    TEST_ASSERT(ltcfb_str_idx == ltcfb_str_max_idx);

    rc = log_flush(&my_log);
    TEST_ASSERT(rc == 0);

    rc = log_walk(&my_log, ltcfb_walk2, &log_offset);
    TEST_ASSERT(rc == 0);
}
