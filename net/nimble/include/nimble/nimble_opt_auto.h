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

#ifndef H_MYNEWT_BLE_AUTO_
#define H_MYNEWT_BLE_AUTO_

#include "nimble/nimble_opt.h"

/***
 * Automatic options.
 * 
 * These settings are generated automatically from the user-specified settings
 * in nimble_opt.h.
 */

#undef MYNEWT_BLE_ADVERTISE
#define MYNEWT_BLE_ADVERTISE                    \
    (MYNEWT_BLE_ROLE_BROADCASTER || MYNEWT_BLE_ROLE_PERIPHERAL)

#undef MYNEWT_BLE_SCAN
#define MYNEWT_BLE_SCAN                         \
    (MYNEWT_BLE_ROLE_CENTRAL || MYNEWT_BLE_ROLE_OBSERVER)

#undef MYNEWT_BLE_CONNECT
#define MYNEWT_BLE_CONNECT                      \
    (MYNEWT_BLE_ROLE_CENTRAL || MYNEWT_BLE_ROLE_PERIPHERAL)


/** Supported client ATT commands. */

#undef MYNEWT_BLE_ATT_CLT_FIND_INFO
#define MYNEWT_BLE_ATT_CLT_FIND_INFO            (MYNEWT_BLE_GATT_DISC_ALL_DSCS)

#undef MYNEWT_BLE_ATT_CLT_FIND_TYPE
#define MYNEWT_BLE_ATT_CLT_FIND_TYPE            (MYNEWT_BLE_GATT_DISC_SVC_UUID)

#undef MYNEWT_BLE_ATT_CLT_READ_TYPE
#define MYNEWT_BLE_ATT_CLT_READ_TYPE            \
    (MYNEWT_BLE_GATT_FIND_INC_SVCS ||           \
     MYNEWT_BLE_GATT_DISC_ALL_CHRS ||           \
     MYNEWT_BLE_GATT_DISC_CHRS_UUID ||          \
     MYNEWT_BLE_GATT_READ_UUID)
    
#undef MYNEWT_BLE_ATT_CLT_READ
#define MYNEWT_BLE_ATT_CLT_READ                 \
    (MYNEWT_BLE_GATT_READ ||                    \
     MYNEWT_BLE_GATT_READ_LONG ||               \
     MYNEWT_BLE_GATT_FIND_INC_SVCS)

#undef MYNEWT_BLE_ATT_CLT_READ_BLOB
#define MYNEWT_BLE_ATT_CLT_READ_BLOB            (MYNEWT_BLE_GATT_READ_LONG)

#undef MYNEWT_BLE_ATT_CLT_READ_MULT
#define MYNEWT_BLE_ATT_CLT_READ_MULT            (MYNEWT_BLE_GATT_READ_MULT)

#undef MYNEWT_BLE_ATT_CLT_READ_GROUP_TYPE
#define MYNEWT_BLE_ATT_CLT_READ_GROUP_TYPE      \
    (MYNEWT_BLE_GATT_DISC_ALL_SVCS)

#undef MYNEWT_BLE_ATT_CLT_WRITE
#define MYNEWT_BLE_ATT_CLT_WRITE                (MYNEWT_BLE_GATT_WRITE)

#undef MYNEWT_BLE_ATT_CLT_WRITE_NO_RSP
#define MYNEWT_BLE_ATT_CLT_WRITE_NO_RSP         (MYNEWT_BLE_GATT_WRITE_NO_RSP)

#undef MYNEWT_BLE_ATT_CLT_PREP_WRITE
#define MYNEWT_BLE_ATT_CLT_PREP_WRITE           (MYNEWT_BLE_GATT_WRITE_LONG)

#undef MYNEWT_BLE_ATT_CLT_EXEC_WRITE
#define MYNEWT_BLE_ATT_CLT_EXEC_WRITE           (MYNEWT_BLE_GATT_WRITE_LONG)

#undef MYNEWT_BLE_ATT_CLT_NOTIFY  
#define MYNEWT_BLE_ATT_CLT_NOTIFY               (MYNEWT_BLE_GATT_NOTIFY)

#undef MYNEWT_BLE_ATT_CLT_INDICATE
#define MYNEWT_BLE_ATT_CLT_INDICATE             (MYNEWT_BLE_GATT_INDICATE)

/** Security manager settings. */

/* Secure connections implies security manager support
 * Note: For now, security manager is synonymous with legacy pairing.  In the
 * future, a new setting for legacy pairing may be introduced as a sibling of
 * the SC setting.
 */
#if MYNEWT_BLE_SM_SC
#undef MYNEWT_BLE_SM
#define MYNEWT_BLE_SM                           1
#endif

#endif
