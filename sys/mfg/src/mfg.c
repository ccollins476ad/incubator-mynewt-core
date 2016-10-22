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

#include <string.h>

#include "sysflash/sysflash.h"

#include "os/os.h"
#include "mfg/mfg.h"

#define MFG_META_MAGIC          0x3bb2a269
#define MFG_META_HEADER_SZ      4
#define MFG_META_FOOTER_SZ      8
#define MFG_META_TLV_SZ         2
#define MFG_META_FLASH_AREA_SZ  12

struct {
    uint8_t valid:1;
    uint32_t off;
    uint32_t size;
} mfg_state;

struct mfg_meta_header {
    uint8_t version;
    uint8_t pad8;
    uint16_t pad16;
};

struct mfg_meta_footer {
    uint16_t size;
    uint16_t pad16;
    uint32_t magic;
};

_Static_assert(sizeof (struct mfg_meta_header) == MFG_META_HEADER_SZ,
               "mfg_meta_header must be 4 bytes");
_Static_assert(sizeof (struct mfg_meta_footer) == MFG_META_FOOTER_SZ,
               "mfg_meta_footer must be 8 bytes");
_Static_assert(sizeof (struct mfg_meta_flash_area) == MFG_META_FLASH_AREA_SZ,
               "mfg_meta_flash_area must be 12 bytes");

int
mfg_next_tlv(struct mfg_meta_tlv *tlv, uint32_t *off)
{
    const struct flash_area *fap;
    int rc;

    if (!mfg_state.valid) {
        return MFG_EUNINIT;
    }

    rc = flash_area_open(FLASH_AREA_BOOTLOADER, &fap);
    if (rc != 0) {
        rc = MFG_EFLASH;
        goto done;
    }

    if (*off == 0) {
        *off = mfg_state.off + MFG_META_HEADER_SZ;
    } else {
        /* Advance past current TLV. */
        *off += MFG_META_TLV_SZ + tlv->size;
    }

    if (*off + MFG_META_FOOTER_SZ >= fap->fa_size) {
        /* Reached end of boot area; no more TLVs. */
        return MFG_EDONE;
    }

    /* Read next TLV. */
    memset(tlv, 0, sizeof *tlv);
    rc = flash_area_read(fap, *off, tlv, MFG_META_TLV_SZ);
    if (rc != 0) {
        rc = MFG_EFLASH;
        goto done;
    }

    rc = 0;

done:
    flash_area_close(fap);
    return rc;
}

int
mfg_next_tlv_with_code(struct mfg_meta_tlv *tlv, uint32_t *off, uint8_t code)
{
    int rc;

    while (1) {
        rc = mfg_next_tlv(tlv, off);
        if (rc != 0) {
            break;
        }

        if (tlv->code == code) {
            break;
        }

        /* Proceed to next TLV. */
    }

    return rc;
}

int
mfg_read_flash_area(const struct mfg_meta_tlv *tlv, uint32_t off,
                    struct mfg_meta_flash_area *out_mfa)
{
    const struct flash_area *fap;
    int read_sz;
    int rc;

    rc = flash_area_open(FLASH_AREA_BOOTLOADER, &fap);
    if (rc != 0) {
        rc = MFG_EFLASH;
        goto done;
    }

    memset(out_mfa, 0, sizeof *out_mfa);

    read_sz = min(MFG_META_FLASH_AREA_SZ, tlv->size);
    rc = flash_area_read(fap, off + MFG_META_TLV_SZ, out_mfa, read_sz);
    if (rc != 0) {
        rc = MFG_EFLASH;
        goto done;
    }

    rc = 0;
    
done:
    flash_area_close(fap);
    return rc;
}

int
mfg_read_hash(const struct mfg_meta_tlv *tlv, uint32_t off, void *out_hash)
{
    const struct flash_area *fap;
    int read_sz;
    int rc;

    rc = flash_area_open(FLASH_AREA_BOOTLOADER, &fap);
    if (rc != 0) {
        rc = MFG_EFLASH;
        goto done;
    }

    read_sz = min(MFG_HASH_SZ, tlv->size);
    rc = flash_area_read(fap, off + MFG_META_TLV_SZ, out_hash, read_sz);
    if (rc != 0) {
        rc = MFG_EFLASH;
        goto done;
    }

    rc = 0;
    
done:
    flash_area_close(fap);
    return rc;
}

int
mfg_init(void)
{
    const struct flash_area *fap;
    struct mfg_meta_footer ftr;
    uint16_t off;
    int rc;

    if (mfg_state.valid) {
        /* Already initialized. */
        return 0;
    }

    mfg_state.valid = 0;

    rc = flash_area_open(FLASH_AREA_BOOTLOADER, &fap);
    if (rc != 0) {
        rc = MFG_EFLASH;
        goto done;
    }

    off = fap->fa_size - sizeof ftr;
    rc = flash_area_read(fap, off, &ftr, sizeof ftr);
    if (rc != 0) {
        rc = MFG_EFLASH;
        goto done;
    }

    if (ftr.magic != MFG_META_MAGIC) {
        rc = MFG_EBADDATA;
        goto done;
    }
    if (ftr.size > fap->fa_size) {
        rc = MFG_EBADDATA;
        goto done;
    }

    mfg_state.valid = 1;
    mfg_state.off = fap->fa_size - ftr.size;
    mfg_state.size = ftr.size;

    rc = 0;

done:
    flash_area_close(fap);
    return rc;
}
