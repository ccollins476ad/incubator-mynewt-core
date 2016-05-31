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
#include <errno.h>
#include "console/console.h"
#include "nimble/ble.h"
#include "nimble/nimble_opt.h"
#include "ble_hs_priv.h"

/**
 * Create some shortened names for the passkey actions so that the table is
 * easier to read.
 */
#define PKACT_NONE  BLE_GAP_PKACT_NONE
#define PKACT_OOB   BLE_GAP_PKACT_OOB
#define PKACT_INPUT BLE_GAP_PKACT_INPUT
#define PKACT_DISP  BLE_GAP_PKACT_DISP

/* This is the initiator passkey action action dpeneding on the io
 * capabilties of both parties
 */
static const uint8_t ble_sm_lgcy_init_pka[5 /*init*/ ][5 /*resp */] =
{
    {PKACT_NONE,    PKACT_NONE,   PKACT_INPUT, PKACT_NONE, PKACT_INPUT},
    {PKACT_NONE,    PKACT_NONE,   PKACT_INPUT, PKACT_NONE, PKACT_INPUT},
    {PKACT_DISP,    PKACT_DISP,   PKACT_INPUT, PKACT_NONE, PKACT_DISP},
    {PKACT_NONE,    PKACT_NONE,   PKACT_NONE,  PKACT_NONE, PKACT_NONE},
    {PKACT_DISP,    PKACT_DISP,   PKACT_DISP,  PKACT_NONE, PKACT_DISP},
};

/* This is the initiator passkey action action depending on the io
 * capabilities of both parties */
static const uint8_t ble_sm_lgcy_resp_pka[5 /*init*/ ][5 /*resp */] =
{
    {PKACT_NONE,    PKACT_NONE,   PKACT_DISP,  PKACT_NONE, PKACT_DISP},
    {PKACT_NONE,    PKACT_NONE,   PKACT_DISP,  PKACT_NONE, PKACT_DISP},
    {PKACT_INPUT,   PKACT_INPUT,  PKACT_INPUT, PKACT_NONE, PKACT_INPUT},
    {PKACT_NONE,    PKACT_NONE,   PKACT_NONE,  PKACT_NONE, PKACT_NONE},
    {PKACT_INPUT,   PKACT_INPUT,  PKACT_INPUT, PKACT_NONE, PKACT_INPUT},
};

int
ble_sm_lgcy_passkey_action(struct ble_sm_proc *proc)
{
    int action;

    if (proc->pair_req.oob_data_flag && proc->pair_rsp.oob_data_flag) {
        action = BLE_GAP_PKACT_OOB;
    } else if (!(proc->pair_req.authreq & BLE_SM_PAIR_AUTHREQ_MITM) ||
               !(proc->pair_rsp.authreq & BLE_SM_PAIR_AUTHREQ_MITM)) {

        action = BLE_GAP_PKACT_NONE;
    } else if (proc->flags & BLE_SM_PROC_F_INITIATOR) {
        action = ble_sm_lgcy_init_pka[proc->pair_req.io_cap]
                                     [proc->pair_rsp.io_cap];
    } else {
        action = ble_sm_lgcy_resp_pka[proc->pair_req.io_cap]
                                     [proc->pair_rsp.io_cap];
    }

    switch (action) {
    case BLE_GAP_PKACT_NONE:
        proc->pair_alg = BLE_SM_PAIR_ALG_JW;
        break;

    case BLE_GAP_PKACT_OOB:
        proc->pair_alg = BLE_SM_PAIR_ALG_OOB;
        proc->flags |= BLE_SM_PROC_F_AUTHENTICATED;
        break;

    case BLE_GAP_PKACT_INPUT:
    case BLE_GAP_PKACT_DISP:
        proc->pair_alg = BLE_SM_PAIR_ALG_PASSKEY;
        proc->flags |= BLE_SM_PROC_F_AUTHENTICATED;
        break;

    default:
        BLE_HS_DBG_ASSERT(0);
        break;
    }

    return action;
}

static int
ble_sm_lgcy_confirm_prepare_args(struct ble_sm_proc *proc,
                                 uint8_t *k, uint8_t *preq, uint8_t *pres,
                                 uint8_t *iat, uint8_t *rat,
                                 uint8_t *ia, uint8_t *ra)
{
    struct ble_hs_conn *conn;

    BLE_HS_DBG_ASSERT(ble_hs_thread_safe());

    conn = ble_hs_conn_find(proc->conn_handle);
    if (conn != NULL) {
        if (proc->flags & BLE_SM_PROC_F_INITIATOR) {
            *iat = BLE_ADDR_TYPE_PUBLIC; /* XXX: Support random addresses. */
            memcpy(ia, ble_hs_our_dev.public_addr, 6);

            *rat = conn->bhc_addr_type;
            memcpy(ra, conn->bhc_addr, 6);
        } else {
            *rat = BLE_ADDR_TYPE_PUBLIC; /* XXX: Support random addresses. */
            memcpy(ra, ble_hs_our_dev.public_addr, 6);

            *iat = conn->bhc_addr_type;
            memcpy(ia, conn->bhc_addr, 6);
        }
    }

    if (conn == NULL) {
        return BLE_HS_ENOTCONN;
    }

    memcpy(k, proc->tk, sizeof proc->tk);

    ble_sm_pair_cmd_write(
        preq, BLE_SM_HDR_SZ + BLE_SM_PAIR_CMD_SZ, 1,
        &proc->pair_req);

    ble_sm_pair_cmd_write(
        pres, BLE_SM_HDR_SZ + BLE_SM_PAIR_CMD_SZ, 0,
        &proc->pair_rsp);

    return 0;
}

void
ble_sm_lgcy_confirm_go(struct ble_sm_proc *proc, struct ble_sm_result *res)
{
    struct ble_sm_pair_confirm cmd;
    uint8_t preq[BLE_SM_HDR_SZ + BLE_SM_PAIR_CMD_SZ];
    uint8_t pres[BLE_SM_HDR_SZ + BLE_SM_PAIR_CMD_SZ];
    uint8_t k[16];
    uint8_t ia[6];
    uint8_t ra[6];
    uint8_t iat;
    uint8_t rat;
    int rc;

    rc = ble_sm_lgcy_confirm_prepare_args(proc, k, preq, pres,
                                          &iat, &rat, ia, ra);
    if (rc != 0) {
        goto err;
    }

    rc = ble_sm_alg_c1(k, ble_sm_our_pair_rand(proc),
                             preq, pres, iat, rat, ia, ra, cmd.value);
    if (rc != 0) {
        goto err;
    }

    rc = ble_sm_pair_confirm_tx(proc->conn_handle, &cmd);
    if (rc != 0) {
        goto err;
    }

    if (!(proc->flags & BLE_SM_PROC_F_INITIATOR)) {
        proc->state = BLE_SM_PROC_STATE_RANDOM;
    }

    return;

err:
    res->app_status = rc;
    res->enc_cb = 1;
    res->sm_err = BLE_SM_ERR_UNSPECIFIED;
}

static int
ble_sm_gen_stk(struct ble_sm_proc *proc)
{
    uint8_t key[16];
    int rc;

    rc = ble_sm_alg_s1(proc->tk, proc->rands, proc->randm, key);
    if (rc != 0) {
        return rc;
    }

    memcpy(proc->ltk, key, sizeof key);

    return 0;
}

void
ble_sm_lgcy_random_go(struct ble_sm_proc *proc,
                      struct ble_sm_result *res)
{
    struct ble_sm_pair_random cmd;
    int rc;

    memcpy(cmd.value, ble_sm_our_pair_rand(proc), 16);

    rc = ble_sm_pair_random_tx(proc->conn_handle, &cmd);
    if (rc != 0) {
        res->app_status = rc;
        res->enc_cb = 1;
        res->sm_err = BLE_SM_ERR_UNSPECIFIED;
        return;
    }

    if (!(proc->flags & BLE_SM_PROC_F_INITIATOR)) {
        proc->state = BLE_SM_PROC_STATE_LTK_START;
    }
}

void
ble_sm_lgcy_random_handle(struct ble_sm_proc *proc, struct ble_sm_result *res)
{
    uint8_t preq[BLE_SM_HDR_SZ + BLE_SM_PAIR_CMD_SZ];
    uint8_t pres[BLE_SM_HDR_SZ + BLE_SM_PAIR_CMD_SZ];
    uint8_t confirm_val[16];
    uint8_t k[16];
    uint8_t ia[6];
    uint8_t ra[6];
    uint8_t iat;
    uint8_t rat;
    int rc;

    rc = ble_sm_lgcy_confirm_prepare_args(proc, k, preq, pres,
                                          &iat, &rat, ia, ra);
    if (rc != 0) {
        res->app_status = rc;
        res->sm_err = BLE_SM_ERR_UNSPECIFIED;
        res->enc_cb = 1;
        return;
    }

    rc = ble_sm_alg_c1(k, ble_sm_their_pair_rand(proc), preq, pres,
                             iat, rat, ia, ra, confirm_val);
    if (rc != 0) {
        res->app_status = rc;
        res->sm_err = BLE_SM_ERR_UNSPECIFIED;
        res->enc_cb = 1;
        return;
    }

    if (memcmp(proc->confirm_their, confirm_val, 16) != 0) {
        /* Random number mismatch. */
        res->app_status = BLE_HS_SM_US_ERR(BLE_SM_ERR_CONFIRM_MISMATCH);
        res->sm_err = BLE_SM_ERR_CONFIRM_MISMATCH;
        res->enc_cb = 1;
        return;
    }

    /* Generate the key. */
    rc = ble_sm_gen_stk(proc);
    if (rc != 0) {
        res->app_status = rc;
        res->sm_err = BLE_SM_ERR_UNSPECIFIED;
        res->enc_cb = 1;
        return;
    }

    if (proc->flags & BLE_SM_PROC_F_INITIATOR) {
        /* Send the start-encrypt HCI command to the controller.   For
         * short-term key generation, we always set ediv and rand to 0.
         * (Vol. 3, part H, 2.4.4.1).
         */
        proc->state = BLE_SM_PROC_STATE_ENC_START;
    }

    res->do_state = 1;
}
