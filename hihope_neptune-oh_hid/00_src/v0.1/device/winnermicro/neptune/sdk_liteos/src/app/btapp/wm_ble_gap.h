#ifndef __WM_BLE_DM_H__
#define __WM_BLE_DM_H__
#include "wm_bt_def.h"
/*Init function, register an device manager from btif_gatt*/
int tls_ble_gap_init();
int tls_ble_gap_deinit();

int tls_ble_register_report_evt(tls_ble_dm_evt_t rpt_evt,  tls_ble_dm_callback_t rpt_callback);
int tls_ble_deregister_report_evt(tls_ble_dm_evt_t rpt_evt,  tls_ble_dm_callback_t rpt_callback);

#endif
