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

#ifndef H_HCI_TRANSPORT_
#define H_HCI_TRANSPORT_

#include <inttypes.h>
struct os_mbuf;

#define BLE_HCI_TRANS_CMD_SZ        260

#define BLE_HCI_TRANS_BUF_EVT_LO    1
#define BLE_HCI_TRANS_BUF_EVT_HI    2
#define BLE_HCI_TRANS_BUF_CMD       3

typedef int ble_hci_trans_rx_cmd_fn(uint8_t *cmd, void *arg);
typedef int ble_hci_trans_rx_acl_fn(struct os_mbuf *om, void *arg);

void ble_hci_trans_set_rx_cbs_hs(ble_hci_trans_rx_cmd_fn *cmd_cb,
                                 void *cmd_arg,
                                 ble_hci_trans_rx_acl_fn *acl_cb,
                                 void *acl_arg);
void ble_hci_trans_set_rx_cbs_ll(ble_hci_trans_rx_cmd_fn *cmd_cb,
                                 void *cmd_arg,
                                 ble_hci_trans_rx_acl_fn *acl_cb,
                                 void *acl_arg);

/* Send a HCI command from the host to the controller */
int ble_hci_trans_hs_cmd_send(uint8_t *cmd);

/* Send a HCI event from the controller to the host */
int ble_hci_trans_ll_evt_send(uint8_t *hci_ev);

/* Send ACL data from host to contoller */
int ble_hci_trans_hs_acl_send(struct os_mbuf *om);

/* Send ACL data from controller to host */
int ble_hci_trans_ll_acl_send(struct os_mbuf *om);

uint8_t *ble_hci_trans_alloc_buf(int type);
int ble_hci_trans_free_buf(uint8_t *buf);

#endif /* H_HCI_TRANSPORT_ */
