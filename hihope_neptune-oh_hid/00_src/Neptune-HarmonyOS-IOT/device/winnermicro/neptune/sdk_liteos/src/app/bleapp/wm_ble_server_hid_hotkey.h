#ifndef __WM_BLE_SERVER_HID_HOTKEY_IF__
#define __WM_BLE_SERVER_HID_HOTKEY_IF__

extern int tls_ble_server_hid_hotkey_init();
extern int tls_ble_server_hid_hotkey_deinit();

extern int check_hotkey_released(uint8_t  *key_index);
extern int check_hotkey_pressed(uint8_t *key_array, int len);
extern int process_hotkey(uint8_t key_action, uint8_t key_index);


#endif

