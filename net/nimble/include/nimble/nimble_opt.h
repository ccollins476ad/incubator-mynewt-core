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

#ifndef H_NIMBLE_OPT_
#define H_NIMBLE_OPT_

/** HOST / CONTROLLER: Maximum number of concurrent connections. */

#ifndef MYNEWT_BLE_MAX_CONNECTIONS
#define MYNEWT_BLE_MAX_CONNECTIONS              1
#endif


/**
 * HOST / CONTROLLER: Supported GAP roles.  By default, all four roles are
 * enabled.
 */

#ifndef MYNEWT_BLE_ROLE_CENTRAL
#define MYNEWT_BLE_ROLE_CENTRAL                 1
#endif

#ifndef MYNEWT_BLE_ROLE_PERIPHERAL
#define MYNEWT_BLE_ROLE_PERIPHERAL              1
#endif

#ifndef MYNEWT_BLE_ROLE_BROADCASTER
#define MYNEWT_BLE_ROLE_BROADCASTER             1
#endif

#ifndef MYNEWT_BLE_ROLE_OBSERVER
#define MYNEWT_BLE_ROLE_OBSERVER                1
#endif

#ifndef MYNEWT_BLE_WHITELIST
#define MYNEWT_BLE_WHITELIST                    1
#endif

/** HOST: L2CAP settings. */

#ifndef MYNEWT_BLE_L2CAP_MAX_CHANS
#define MYNEWT_BLE_L2CAP_MAX_CHANS              (3*MYNEWT_BLE_MAX_CONNECTIONS)
#endif

#ifndef MYNEWT_BLE_L2CAP_SIG_MAX_PROCS
#define MYNEWT_BLE_L2CAP_SIG_MAX_PROCS          1
#endif

/** HOST: Security manager settings. */

/* Security manager legacy pairing.  Enabled by default. */
#ifndef MYNEWT_BLE_SM
#define MYNEWT_BLE_SM                           1
#endif

/* Security manager secure connections (4.2).  Disabled by default. */
#ifndef MYNEWT_BLE_SM_SC
#define MYNEWT_BLE_SM_SC                        0
#endif

#ifndef MYNEWT_BLE_SM_MAX_PROCS
#define MYNEWT_BLE_SM_MAX_PROCS                 1
#endif

#ifndef MYNEWT_BLE_SM_IO_CAP
#define MYNEWT_BLE_SM_IO_CAP                    BLE_HS_IO_NO_INPUT_OUTPUT
#endif

#ifndef MYNEWT_BLE_SM_OOB_DATA_FLAG
#define MYNEWT_BLE_SM_OOB_DATA_FLAG             0
#endif

#ifndef MYNEWT_BLE_SM_BONDING      
#define MYNEWT_BLE_SM_BONDING                   0
#endif

#ifndef MYNEWT_BLE_SM_MITM         
#define MYNEWT_BLE_SM_MITM                      0
#endif

#ifndef MYNEWT_BLE_SM_KEYPRESS     
#define MYNEWT_BLE_SM_KEYPRESS                  0
#endif

#ifndef MYNEWT_BLE_SM_OUR_KEY_DIST 
#define MYNEWT_BLE_SM_OUR_KEY_DIST              0
#endif

#ifndef MYNEWT_BLE_SM_THEIR_KEY_DIST
#define MYNEWT_BLE_SM_THEIR_KEY_DIST            0
#endif

/**
 * HOST: Supported GATT procedures.  By default:
 *     o Notify and indicate are enabled;
 *     o All other procedures are enabled for centrals.
 */

#ifndef MYNEWT_BLE_GATT_DISC_ALL_SVCS
#define MYNEWT_BLE_GATT_DISC_ALL_SVCS           MYNEWT_BLE_ROLE_CENTRAL
#endif

#ifndef MYNEWT_BLE_GATT_DISC_SVC_UUID
#define MYNEWT_BLE_GATT_DISC_SVC_UUID           MYNEWT_BLE_ROLE_CENTRAL
#endif

#ifndef MYNEWT_BLE_GATT_FIND_INC_SVCS
#define MYNEWT_BLE_GATT_FIND_INC_SVCS           MYNEWT_BLE_ROLE_CENTRAL
#endif

#ifndef MYNEWT_BLE_GATT_DISC_ALL_CHRS
#define MYNEWT_BLE_GATT_DISC_ALL_CHRS           MYNEWT_BLE_ROLE_CENTRAL
#endif

#ifndef MYNEWT_BLE_GATT_DISC_CHR_UUID
#define MYNEWT_BLE_GATT_DISC_CHR_UUID           MYNEWT_BLE_ROLE_CENTRAL
#endif

#ifndef MYNEWT_BLE_GATT_DISC_ALL_DSCS
#define MYNEWT_BLE_GATT_DISC_ALL_DSCS           MYNEWT_BLE_ROLE_CENTRAL
#endif

#ifndef MYNEWT_BLE_GATT_READ
#define MYNEWT_BLE_GATT_READ                    MYNEWT_BLE_ROLE_CENTRAL
#endif

#ifndef MYNEWT_BLE_GATT_READ_UUID
#define MYNEWT_BLE_GATT_READ_UUID               MYNEWT_BLE_ROLE_CENTRAL
#endif

#ifndef MYNEWT_BLE_GATT_READ_LONG
#define MYNEWT_BLE_GATT_READ_LONG               MYNEWT_BLE_ROLE_CENTRAL
#endif

#ifndef MYNEWT_BLE_GATT_READ_MULT
#define MYNEWT_BLE_GATT_READ_MULT               MYNEWT_BLE_ROLE_CENTRAL
#endif

#ifndef MYNEWT_BLE_GATT_WRITE_NO_RSP
#define MYNEWT_BLE_GATT_WRITE_NO_RSP            MYNEWT_BLE_ROLE_CENTRAL
#endif

#ifndef MYNEWT_BLE_GATT_SIGNED_WRITE
#define MYNEWT_BLE_GATT_SIGNED_WRITE            MYNEWT_BLE_ROLE_CENTRAL
#endif

#ifndef MYNEWT_BLE_GATT_WRITE
#define MYNEWT_BLE_GATT_WRITE                   MYNEWT_BLE_ROLE_CENTRAL
#endif

#ifndef MYNEWT_BLE_GATT_WRITE_LONG
#define MYNEWT_BLE_GATT_WRITE_LONG              MYNEWT_BLE_ROLE_CENTRAL
#endif

#ifndef MYNEWT_BLE_GATT_WRITE_RELIABLE
#define MYNEWT_BLE_GATT_WRITE_RELIABLE          MYNEWT_BLE_ROLE_CENTRAL
#endif

#ifndef MYNEWT_BLE_GATT_NOTIFY
#define MYNEWT_BLE_GATT_NOTIFY                  1
#endif

#ifndef MYNEWT_BLE_GATT_INDICATE
#define MYNEWT_BLE_GATT_INDICATE                1
#endif

/** HOST: GATT options. */

/* The maximum number of attributes that can be written with a single GATT
 * Reliable Write procedure.
 */
#ifndef MYNEWT_BLE_GATT_WRITE_MAX_ATTRS
#define MYNEWT_BLE_GATT_WRITE_MAX_ATTRS         4
#endif

#ifndef MYNEWT_BLE_GATT_MAX_PROCS
#define MYNEWT_BLE_GATT_MAX_PROCS               4
#endif


/** HOST: Supported server ATT commands. */

#ifndef MYNEWT_BLE_ATT_SVR_FIND_INFO
#define MYNEWT_BLE_ATT_SVR_FIND_INFO            1
#endif

#ifndef MYNEWT_BLE_ATT_SVR_FIND_TYPE
#define MYNEWT_BLE_ATT_SVR_FIND_TYPE            1
#endif

#ifndef MYNEWT_BLE_ATT_SVR_READ_TYPE
#define MYNEWT_BLE_ATT_SVR_READ_TYPE            1
#endif

#ifndef MYNEWT_BLE_ATT_SVR_READ
#define MYNEWT_BLE_ATT_SVR_READ                 1
#endif

#ifndef MYNEWT_BLE_ATT_SVR_READ_BLOB
#define MYNEWT_BLE_ATT_SVR_READ_BLOB            1
#endif

#ifndef MYNEWT_BLE_ATT_SVR_READ_MULT
#define MYNEWT_BLE_ATT_SVR_READ_MULT            1
#endif

#ifndef MYNEWT_BLE_ATT_SVR_READ_GROUP_TYPE
#define MYNEWT_BLE_ATT_SVR_READ_GROUP_TYPE      1
#endif

#ifndef MYNEWT_BLE_ATT_SVR_WRITE
#define MYNEWT_BLE_ATT_SVR_WRITE                1
#endif

#ifndef MYNEWT_BLE_ATT_SVR_WRITE_NO_RSP
#define MYNEWT_BLE_ATT_SVR_WRITE_NO_RSP         1
#endif

#ifndef MYNEWT_BLE_ATT_SVR_SIGNED_WRITE
#define MYNEWT_BLE_ATT_SVR_SIGNED_WRITE         1
#endif

#ifndef MYNEWT_BLE_ATT_SVR_PREP_WRITE
#define MYNEWT_BLE_ATT_SVR_PREP_WRITE           1
#endif

#ifndef MYNEWT_BLE_ATT_SVR_EXEC_WRITE
#define MYNEWT_BLE_ATT_SVR_EXEC_WRITE           1
#endif

#ifndef MYNEWT_BLE_ATT_SVR_NOTIFY
#define MYNEWT_BLE_ATT_SVR_NOTIFY               1
#endif

#ifndef MYNEWT_BLE_ATT_SVR_INDICATE
#define MYNEWT_BLE_ATT_SVR_INDICATE             1
#endif


/** HOST: ATT options. */

#ifndef MYNEWT_BLE_ATT_MAX_PREP_ENTRIES
#define MYNEWT_BLE_ATT_MAX_PREP_ENTRIES         6
#endif


/** HOST: Privacy options. */

#ifndef MYNEWT_BLE_RPA_TIMEOUT
#define MYNEWT_BLE_RPA_TIMEOUT                  300
#endif


/** HOST: Miscellaneous features. */

#ifndef MYNEWT_BLE_EDDYSTONE
#define MYNEWT_BLE_EDDYSTONE                    1
#endif


/*** CONTROLLER ***/

/*
 * Sleep clock accuracy (sca). This is the amount of drift in the system during
 * when the device is sleeping (in parts per million).
 *
 * NOTE: the master sca is an enumerated value based on the sca. Rather than
 * have a piece of code calculate this value, the developer must set this
 * value based on the value of the SCA using the following table:
 *
 *  SCA between 251 and 500 ppm (inclusive); master sca = 0
 *  SCA between 151 and 250 ppm (inclusive); master sca = 1
 *  SCA between 101 and 150 ppm (inclusive); master sca = 2
 *  SCA between 76 and 100 ppm (inclusive); master sca = 3
 *  SCA between 51 and 75 ppm (inclusive); master sca = 4
 *  SCA between 31 and 50 ppm (inclusive); master sca = 5
 *  SCA between 21 and 30 ppm (inclusive); master sca = 6
 *  SCA between 0 and 20 ppm (inclusive); master sca = 7
 *
 *  For example:
 *      if your clock drift is 101 ppm, your master should be set to 2.
 *      if your clock drift is 20, your master sca should be set to 7.
 *
 *  The values provided below are merely meant to be an example and should
 *  be replaced by values appropriate for your platform.
 */
#ifndef MYNEWT_BLE_LL_OUR_SCA
#define MYNEWT_BLE_LL_OUR_SCA                   (60)    /* in ppm */
#endif

#ifndef MYNEWT_BLE_LL_MASTER_SCA
#define MYNEWT_BLE_LL_MASTER_SCA                (4)
#endif

/* transmit power level */
#ifndef MYNEWT_BLE_LL_TX_PWR_DBM
#define MYNEWT_BLE_LL_TX_PWR_DBM                (0)
#endif

/*
 * Determines the maximum rate at which the controller will send the
 * number of completed packets event to the host. Rate is in os time ticks
 */
#ifndef MYNEWT_BLE_NUM_COMP_PKT_RATE
#define MYNEWT_BLE_NUM_COMP_PKT_RATE    ((2000 * OS_TICKS_PER_SEC) / 1000)
#endif

/* Manufacturer ID. Should be set to unique ID per manufacturer */
#ifndef MYNEWT_BLE_LL_MFRG_ID
#define MYNEWT_BLE_LL_MFRG_ID                   (0xFFFF)
#endif

/*
 * Configuration items for the number of duplicate advertisers and the
 * number of advertisers from which we have heard a scan response.
 */
#ifndef MYNEWT_BLE_LL_NUM_SCAN_DUP_ADVS
#define MYNEWT_BLE_LL_NUM_SCAN_DUP_ADVS         (8)
#endif

#ifndef MYNEWT_BLE_LL_NUM_SCAN_RSP_ADVS
#define MYNEWT_BLE_LL_NUM_SCAN_RSP_ADVS         (8)
#endif

/* Size of the LL whitelist */
#ifndef MYNEWT_BLE_LL_WHITELIST_SIZE
#define MYNEWT_BLE_LL_WHITELIST_SIZE            (8)
#endif

/* Size of the resolving ist */
#ifndef MYNEWT_BLE_LL_RESOLV_LIST_SIZE
#define MYNEWT_BLE_LL_RESOLV_LIST_SIZE          (4)
#endif

/*
 * Data length management definitions for connections. These define the maximum
 * size of the PDU's that will be sent and/or received in a connection.
 */
#ifndef MYNEWT_BLE_LL_MAX_PKT_SIZE
#define MYNEWT_BLE_LL_MAX_PKT_SIZE              (251)
#endif

#ifndef MYNEWT_BLE_LL_SUPP_MAX_RX_BYTES
#define MYNEWT_BLE_LL_SUPP_MAX_RX_BYTES         (MYNEWT_BLE_LL_MAX_PKT_SIZE)
#endif

#ifndef MYNEWT_BLE_LL_SUPP_MAX_TX_BYTES
#define MYNEWT_BLE_LL_SUPP_MAX_TX_BYTES         (MYNEWT_BLE_LL_MAX_PKT_SIZE)
#endif

#ifndef MYNEWT_BLE_LL_CONN_INIT_MAX_TX_BYTES
#define MYNEWT_BLE_LL_CONN_INIT_MAX_TX_BYTES    (27)
#endif

/* The number of slots that will be allocated to each connection */
#ifndef MYNEWT_BLE_LL_CONN_INIT_SLOTS
#define MYNEWT_BLE_LL_CONN_INIT_SLOTS           (2)
#endif

/* The number of random bytes to store */
#ifndef MYNEWT_BLE_LL_RNG_BUFSIZE
#define MYNEWT_BLE_LL_RNG_BUFSIZE               (32)
#endif

/*
 * Configuration for LL supported features.
 *
 * There are a total 8 features that the LL can support. These can be found in
 * v4.2, Vol 6 Part B Section 4.6.
 *
 * These feature definitions are used to inform a host or other controller
 * about the LL features supported by the controller.
 *
 * NOTE: the controller always supports extended reject indicate and thus is
 * not listed here.
 */

 /*
  * This option enables/disables encryption support in the controller. This
  * option saves both both code and RAM.
  */
#ifndef BLE_LL_CFG_FEAT_LE_ENCRYPTION
#define BLE_LL_CFG_FEAT_LE_ENCRYPTION           (1)
#endif

/*
 * This option enables/disables the connection parameter request procedure.
 * This is implemented in the controller but is disabled by default.
 */
#ifndef BLE_LL_CFG_FEAT_CONN_PARAM_REQ
#define BLE_LL_CFG_FEAT_CONN_PARAM_REQ          (0)
#endif

/*
 * This option allows a slave to initiate the feature exchange procedure.
 * This feature is implemented but currently has no impact on code or ram size
 */
#ifndef BLE_LL_CFG_FEAT_SLAVE_INIT_FEAT_XCHG
#define BLE_LL_CFG_FEAT_SLAVE_INIT_FEAT_XCHG    (1)
#endif

/*
 * This option allows a controller to send/receive LE pings. Currently,
 * this feature is not implemented by the controller so turning it on or off
 * has no effect.
 */
#ifndef BLE_LL_CFG_FEAT_LE_PING
#define  BLE_LL_CFG_FEAT_LE_PING                (1)
#endif

/*
 * This option enables/disables the data length update procedure in the
 * controller. If enabled, the controller is allowed to change the size of
 * tx/rx pdu's used in a connection. This option has only minor impact on
 * code size and non on RAM.
 */
#ifndef BLE_LL_CFG_FEAT_DATA_LEN_EXT
#define  BLE_LL_CFG_FEAT_DATA_LEN_EXT           (1)
#endif

/*
 * This option is used to enable/disable LL privacy. Currently, this feature
 * is not supported by the nimble controller.
 */
#ifndef BLE_LL_CFG_FEAT_LL_PRIVACY
#define BLE_LL_CFG_FEAT_LL_PRIVACY              (1)
#endif

/*
 * This option is used to enable/disable the extended scanner filter policy
 * feature. Currently, this feature is not supported by the nimble controller.
 */
#ifndef BLE_LL_CFG_FEAT_EXT_SCAN_FILT
#define  BLE_LL_CFG_FEAT_EXT_SCAN_FILT          (0)
#endif

#ifndef BLE_LL_NUM_ACL_PKTS
#define BLE_LL_NUM_ACL_PKTS                     (12)
#endif

#ifndef BLE_LL_ACL_PKT_SIZE
#define BLE_LL_ACL_PKT_SIZE                     (260)
#endif

/**
 * This macro exists to help catch bugs at compile time.  If code uses this
 * macro to check an option value, the compiler will complain when this header
 * is not included.  If the code checks the option symbol directly without
 * including this header, it will appear as though the option is set to 0.
 */
#define MYNEWT_BLE(x)                           MYNEWT_BLE_ ## x

/* Include automatically-generated settings. */
#include "nimble/nimble_opt_auto.h"

#endif
