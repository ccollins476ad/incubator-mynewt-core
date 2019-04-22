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

#include "os/mynewt.h"

#if MYNEWT_VAL(LOG_FCB_BOOKMARKS)

#include <string.h>
#include "fcb/fcb.h"
#include "fcb_priv.h"
#include "log/log.h"

void
fcb_log_init_bmarks(struct fcb_log *fcb_log,
                    struct fcb_log_bmark *buf, int bmark_count)
{
    fcb_log->fl_bset = (struct fcb_log_bset) {
        .fls_bmarks = buf,
        .fls_cap = bmark_count,
    };

    fcb_log_clear_bmarks(fcb_log);
}

void
fcb_log_clear_bmarks(struct fcb_log *fcb_log)
{
    struct fcb_log_bmark *bmark;
    struct fcb_log_bset *bset;
    int i;

    bset = &fcb_log->fl_bset;

    for (i = 0; i < bset->fls_cap; i++) {
        bmark = &bset->fls_bmarks[i];
        bmark->flb_index = FCB_LOG_BMARK_IDX_NONE;
    }

    fcb_log->fl_bset.fls_next = 0;
}

const struct fcb_log_bmark *
fcb_log_closest_bmark(const struct fcb_log *fcb_log, uint32_t index)
{
    const struct fcb_log_bmark *closest;
    const struct fcb_log_bmark *bmark;
    uint32_t min_diff;
    uint32_t diff;
    int i;

    min_diff = UINT32_MAX;
    closest = NULL;

    for (i = 0; i < fcb_log->fl_bset.fls_cap; i++) {
        bmark = &fcb_log->fl_bset.fls_bmarks[i];
        if (bmark->flb_index != FCB_LOG_BMARK_IDX_NONE &&
            bmark->flb_index <= index) {

            diff = index - bmark->flb_index;
            if (diff < min_diff) {
                min_diff = diff;
                closest = bmark;
            }
        }
    }

    return closest;
}

void
fcb_log_add_bmark(struct fcb_log *fcb_log, const struct fcb_entry *entry,
                  uint32_t index)
{
    struct fcb_log_bset *bset;

    bset = &fcb_log->fl_bset;

    if (bset->fls_cap == 0) {
        return;
    }

    bset->fls_bmarks[bset->fls_next] = (struct fcb_log_bmark) {
        .flb_entry = *entry,
        .flb_index = index,
    };

    bset->fls_next++;
    if (bset->fls_next >= bset->fls_cap) {
        bset->fls_next = 0;
    }
}

void
fcb_log_clear_bmarks_for_rotate(struct fcb_log *fcb_log)
{
    struct fcb_log_bmark *bmark;
    struct fcb_log_bset *bset;
    const struct fcb *fcb;
    int i;

    bset = &fcb_log->fl_bset;
    fcb = &fcb_log->fl_fcb;

    /* The oldest area in the FCB is about to reclaimed.  Invalidate all
     * bookmarks that point to this area.
     */
    for (i = 0; i < bset->fls_cap; i++) {
        bmark = &bset->fls_bmarks[i];
        if (bmark->flb_entry.fe_area == fcb->f_oldest) {
            bmark->flb_index = FCB_LOG_BMARK_IDX_NONE;
        }
    }
}

int
fcb_log_verify_bmark(struct log *log,
                     const struct fcb_log_bmark *bmark)
{
    struct log_entry_hdr hdr;
    struct fcb_entry entry;
    struct fcb_log *fcb_log;
    int rc;

    fcb_log = log->l_arg;

    entry = bmark->flb_entry;
    rc = fcb_elem_info(&fcb_log->fl_fcb, &entry);
    if (rc != 0) {
        return rc;
    }

    rc = log_read_hdr(log, &entry, &hdr);
    if (rc != 0) {
        return rc;
    }

    if (hdr.ue_index != bmark->flb_index) {
        return SYS_ENOENT;
    }

    return 0;
}

#endif /* MYNEWT_VAL(LOG_FCB_BOOKMARKS) */
