#ifndef __NBLE_SERVER_H__
#define __NBLE_SERVER_H__

/*Initialize the list for ble server*/
extern void ble_server_init();

extern void ble_server_start_service();

/*Free servcie*/
extern int ble_server_free(int server_id);

/*Register one service, and return server if*/
extern int ble_server_alloc(BleGattService *srvcinfo);

extern void ble_server_gap_event(struct ble_gap_event *event, void *arg);

/*Internal function*/
extern void ble_server_update_svc_handle(ble_uuid_t *uuid, uint16_t attr_handle);
extern void ble_server_retrieve_id_by_service_id(uint16_t svc_handle, uint16_t *server_id);
extern void ble_server_retrieve_id_by_uuid(ble_uuid_t *uuid, uint16_t *server_id);
extern void ble_server_retrieve_service_handle_by_server_id(uint16_t server_id, uint16_t *service_handle);


#endif
