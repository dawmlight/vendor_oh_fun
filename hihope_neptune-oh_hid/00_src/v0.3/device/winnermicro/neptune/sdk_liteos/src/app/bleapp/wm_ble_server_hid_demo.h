#ifndef __WM_BLE_SERVER_HID_DEMO_H__
#define __WM_BLE_SERVER_HID_DEMO_H__

extern int tls_ble_server_demo_hid_init();
extern int tls_ble_server_demo_hid_deinit();
extern int tls_ble_server_demo_hid_keyboard_notify(uint8_t *key_array, int length);
extern int tls_ble_server_demo_hid_keyboard_cc_notify(uint8_t *key_array, int length);
extern int tls_ble_server_demo_hid_mouse_notify(uint8_t *mouse_array, int length);
extern int tls_ble_server_demo_hid_disconnect(bool continue_to_adv);
extern int tls_ble_server_demo_hid_advertise(uint8_t state, void *addr);
extern int tls_ble_server_demo_hid_conn_param_update_high_duty_slave();
extern int tls_ble_server_demo_hid_conn_param_update_low_duty_slave();

extern uint8_t tls_ble_server_demo_hid_connected();
extern uint8_t tls_ble_server_demo_hid_advertising();


#endif
