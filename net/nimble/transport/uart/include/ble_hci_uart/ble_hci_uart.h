#ifndef H_BLE_HCI_UART_
#define H_BLE_HCI_UART_

#define H4_NONE 0x00
#define H4_CMD  0x01
#define H4_ACL  0x02
#define H4_SCO  0x03
#define H4_EVT  0x04

typedef void ble_hci_uart_rx_fn(const uint8_t *data);

int ble_hci_uart_init(ble_hci_uart_rx_fn *rx_cb, void *cb_arg);

#endif
