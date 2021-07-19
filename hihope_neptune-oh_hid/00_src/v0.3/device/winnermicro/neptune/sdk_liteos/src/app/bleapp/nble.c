/*
 * Copyright (c) 2020, HiHope Community.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "wm_ble.h"
#include "wm_bt_def.h"
#include "bt_gatt.h"
#include "ble_util.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "harmony_bt_def.h"
#include "harmony_bt_gatt.h"
#include "nble_server.h"

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

static volatile tls_bt_state_t bt_adapter_state = WM_BT_STATE_OFF;
static volatile bool ble_system_state_on = false;

/*back up the structure function pointer*/
static BtGattClientCallbacks *gattc_struct_func_ptr_cb=NULL;
static BtGattServerCallbacks *gatts_struct_func_ptr_cb=NULL;
static BtGattCallbacks       *gap_func_ptr_cb=NULL;

static StartAdvRawData       g_adv_raw_data;
static BleAdvParams          g_adv_param;
static uint16_t              g_conn_handle;

static void on_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg);
static void app_adapter_state_changed_callback(tls_bt_state_t status)
{
	BLE_IF_DEBUG("adapter status = %s\r\n", status==WM_BT_STATE_ON?"bt_state_on":"bt_state_off");

    bt_adapter_state = status;
    
	#if (TLS_CONFIG_BLE == CFG_ON)

    if(status == WM_BT_STATE_ON)
    {
    	BLE_IF_PRINTF("init base application\r\n");
        ble_server_init();

		//at here , user run their own applications;     

        test_adv();

    }else
    {
        BLE_IF_PRINTF("deinit base application\r\n");

        //here, user may free their application;

    }

    #endif

}


int InitBtStack(void) {
    return OHOS_BT_STATUS_SUCCESS;
}

static void
on_sync(void)
{
    //int rc;
    /* Make sure we have proper identity address set (public preferred) */
    //rc = ble_hs_util_ensure_addr(1);
    //assert(rc == 0);

    app_adapter_state_changed_callback(WM_BT_STATE_ON);
}
static void
on_reset(int reason)
{
    BLE_IF_DEBUG("Resetting state; reason=%d\r\n", reason);
    app_adapter_state_changed_callback(WM_BT_STATE_OFF);
}


int EnableBtStack(void) {
    
    if(ble_system_state_on)
    {
        return BLE_HS_EALREADY;
    }

    memset(&ble_hs_cfg, 0, sizeof(ble_hs_cfg));

    /** Security manager settings. */
    ble_hs_cfg.sm_io_cap = MYNEWT_VAL(BLE_SM_IO_CAP),
    ble_hs_cfg.sm_oob_data_flag = MYNEWT_VAL(BLE_SM_OOB_DATA_FLAG),
    ble_hs_cfg.sm_bonding = MYNEWT_VAL(BLE_SM_BONDING),
    ble_hs_cfg.sm_mitm = MYNEWT_VAL(BLE_SM_MITM),
    ble_hs_cfg.sm_sc = MYNEWT_VAL(BLE_SM_SC),
    ble_hs_cfg.sm_keypress = MYNEWT_VAL(BLE_SM_KEYPRESS),
    ble_hs_cfg.sm_our_key_dist = MYNEWT_VAL(BLE_SM_OUR_KEY_DIST),
    ble_hs_cfg.sm_their_key_dist = MYNEWT_VAL(BLE_SM_THEIR_KEY_DIST),

    ble_hs_cfg.sync_cb = on_sync;
    ble_hs_cfg.reset_cb = on_reset;
    ble_hs_cfg.shutdown_cb = on_reset; /*same callback as on_reset */
    ble_hs_cfg.gatts_register_cb = on_svr_register_cb;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;   
    
    /* Initialize all packages. */
    nimble_port_init();

    /*Application levels code entry*/
    tls_ble_gap_init();
    tls_bt_util_init();

    /*Initialize the vuart interface and enable controller*/
    ble_hci_vuart_init(0x01);
    
    /* As the last thing, process events from default event queue. */
    tls_nimble_start();
    
    ble_system_state_on = true;

    while(bt_adapter_state != WM_BT_STATE_ON)
    {
        tls_os_time_delay(100);
    }
   
    return OHOS_BT_STATUS_SUCCESS;
}

int DisableBtStack(void) {
    int rc = 0;

    if(!ble_system_state_on)
    {
        return BLE_HS_EALREADY;
    }
    /*Stop hs system*/
    rc = nimble_port_stop();
    assert(rc == 0);
    
    /*Stop controller and free vuart resource */
    rc = ble_hci_vuart_deinit();
    assert(rc == 0);

    /*Free hs system resource*/
    nimble_port_deinit();
    
    /*Free task stack ptr and free hs task*/
    tls_nimble_stop();

    /*Application levels resource cleanup*/
    tls_ble_gap_deinit();
    tls_bt_util_deinit();
    
    ble_system_state_on = false;
  
    return OHOS_BT_STATUS_SUCCESS;
}

int SetDeviceName(const char *name, unsigned int len) {
    int rc;
    
    rc = ble_svc_gap_device_name_set(name);

    if(rc == 0)
    {
        return OHOS_BT_STATUS_SUCCESS;
    }else
    {
        return OHOS_BT_STATUS_FAIL;
    }
}

int BleGattcRegister(BtUuid appUuid) {
    return OHOS_BT_STATUS_SUCCESS;
}

int BleGattcUnRegister(int clientId) {
    return OHOS_BT_STATUS_SUCCESS;
}

int BleSetSecurityIoCap(BleIoCapMode mode) {
    ble_hs_cfg.sm_io_cap = mode;
    BLE_IF_DEBUG("@@@BleSetSecurityIoCap, BleIoCapMode=%d\r\n", mode);
    return OHOS_BT_STATUS_SUCCESS;
}

int BleSetSecurityAuthReq(BleAuthReqMode mode) {
    BLE_IF_DEBUG("@@@BleSetSecurityAuthReq, BleAuthReqMode=%d\r\n", mode);
    ////
    ///typedef enum {
    /// OHOS_BLE_AUTH_NO_BOND = 0x00,
    /// OHOS_BLE_AUTH_BOND,
    /// OHOS_BLE_AUTH_REQ_MITM,
    /// OHOS_BLE_AUTH_REQ_SC_ONLY,
    /// OHOS_BLE_AUTH_REQ_SC_BOND,
    /// OHOS_BLE_AUTH_REQ_SC_MITM,
    /// OHOS_BLE_AUTH_REQ_SC_MITM_BOND
    ///} BleAuthReqMode;
    ///map
    ///
    ///
    ///
    ///
    ///ble_hs_cfg.sm_bonding = MYNEWT_VAL(BLE_SM_BONDING),
    ///ble_hs_cfg.sm_mitm = MYNEWT_VAL(BLE_SM_MITM),
    ///ble_hs_cfg.sm_sc = MYNEWT_VAL(BLE_SM_SC),
    if(mode&OHOS_BLE_AUTH_BOND || mode&OHOS_BLE_AUTH_REQ_SC_BOND || mode&OHOS_BLE_AUTH_REQ_SC_MITM_BOND)
    {
        ble_hs_cfg.sm_bonding = 1;
    }else
    {
        //ble_hs_cfg.sm_bonding = 0;
    }

    if(mode&OHOS_BLE_AUTH_REQ_MITM || mode&OHOS_BLE_AUTH_REQ_SC_MITM || mode&OHOS_BLE_AUTH_REQ_SC_MITM_BOND )
    {
        ble_hs_cfg.sm_mitm = 1;
    }else
    {
        ble_hs_cfg.sm_mitm = 0;
    }

    if(mode&OHOS_BLE_AUTH_REQ_SC_ONLY || mode&OHOS_BLE_AUTH_REQ_SC_BOND || mode&OHOS_BLE_AUTH_REQ_SC_MITM || mode&OHOS_BLE_AUTH_REQ_SC_MITM_BOND)
    {
        ble_hs_cfg.sm_sc = 1;
    }else
    {
        //ble_hs_cfg.sm_sc = 0;
    }
    
    return OHOS_BT_STATUS_SUCCESS;
}

int BleGattSecurityRsp(BdAddr bdAddr, bool accept) {
	BLE_IF_DEBUG("@@@BleGattSecurityRsp, accept=%d\r\n", accept);
    return OHOS_BT_STATUS_SUCCESS;
}

int BleGattsDisconnect(int serverId, BdAddr bdAddr, int connId) {
    int rc;
    
    rc = ble_gap_terminate(connId, BLE_ERR_REM_USER_CONN_TERM);    

    if(!rc)
    {
        return OHOS_BT_STATUS_SUCCESS;
    }else
    {
        return OHOS_BT_STATUS_FAIL;
    }

}

int BleGattsSetEncryption(BdAddr bdAddr, BleSecAct secAct) {
    int rc;
    return OHOS_BT_STATUS_SUCCESS;

    //get conn_handle by address;
    BLE_IF_DEBUG("@@@BleGattsSetEncryption, secAct=%d\r\n", secAct);

    rc = ble_gap_security_initiate(g_conn_handle);  
    
    if(!rc)
    {
        return OHOS_BT_STATUS_SUCCESS;
    }else
    {
        return OHOS_BT_STATUS_FAIL;
    }

}

int BleGattsRegister(BtUuid appUuid) {
    return OHOS_BT_STATUS_SUCCESS;
}

int BleGattsUnRegister(int serverId) {
    return OHOS_BT_STATUS_SUCCESS;
}

int BleGattsAddService(int serverId, BtUuid srvcUuid, bool isPrimary, int number) {
    return OHOS_BT_STATUS_SUCCESS;
}

int BleGattsDeleteService(int serverId, int srvcHandle) {
    return OHOS_BT_STATUS_SUCCESS;
}

int BleGattsAddCharacteristic(int serverId, int srvcHandle, BtUuid characUuid,
                                         int properties, int permissions) {
    return OHOS_BT_STATUS_SUCCESS;
}

int BleGattsAddDescriptor(int serverId, int srvcHandle, BtUuid descUuid, int permissions) {
    return OHOS_BT_STATUS_SUCCESS;
}

int BleGattsStartService(int serverId, int srvcHandle) {
    return OHOS_BT_STATUS_SUCCESS;
}

int BleGattsStopService(int serverId, int srvcHandle) {
    return OHOS_BT_STATUS_SUCCESS;
}

int BleGattsSendResponse(int serverId, GattsSendRspParam *param) {
    return OHOS_BT_STATUS_SUCCESS;
}

#if BLE_IF_DBG

static void 
ble_server_gatt_dump_hexstring(const char *info, uint8_t *p, int length)
{
    int i = 0, j = 0;
    printf("%s\r\n", info);
    for(i = 0; i<length; i++)
    {
        j++;
        printf("%02x ", p[i]);
        if(j == 16)
        {
            printf("\r\n");
            j = 0;
        }
    }
    printf("\r\n");
}
#endif

int BleGattsSendIndication(int serverId, GattsSendIndParam *param) {

    int rc;
    struct os_mbuf *om;
    
    //TLS_BT_APPL_TRACE_DEBUG("### %s len=%d\r\n", __FUNCTION__, data_len);
    BLE_IF_DEBUG("Indicate to app:serverId[%d],g_conn_handle[%d] conn_id[%d],attr_handle[%d],data_length[%d]\r\n", serverId,g_conn_handle, param->connectId,param->attrHandle,param->valueLen);
    if(param->valueLen<=0 || param->value== NULL)
    {
        return BLE_HS_EINVAL;
    }
    
    
    om = ble_hs_mbuf_from_flat(param->value, param->valueLen);
    if (!om) {
        return BLE_HS_ENOMEM;
    }
#if BLE_IF_DBG    
    ble_server_gatt_dump_hexstring("FROM HILINK:", param->value, param->valueLen);
#endif    
    if(param->confirm)
    {
        rc = ble_gattc_indicate_custom(param->connectId, 11, om);
    }else
    {
        rc = ble_gattc_notify_custom(param->connectId, 11, om);
    }
    assert(rc == 0);
    
    return OHOS_BT_STATUS_SUCCESS;
}

int ReadBtMacAddr(unsigned char *mac, unsigned int len) {
    assert(len != 6);
    tls_get_bt_mac_addr(mac);
    return OHOS_BT_STATUS_SUCCESS;
}


int BleGattcRegisterCallbacks(BtGattClientCallbacks *func) {
    return OHOS_BT_STATUS_SUCCESS;
}


int BleGattsRegisterCallbacks(BtGattServerCallbacks *func) {

    gatts_struct_func_ptr_cb = func;
    
    return OHOS_BT_STATUS_SUCCESS;
}


int BleGattRegisterCallbacks(BtGattCallbacks *func) {

    gap_func_ptr_cb = func;
    
    return OHOS_BT_STATUS_SUCCESS;
}

static void
on_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg)
{

    BtUuid btuuid;
    char buf[BLE_UUID_STR_LEN];
    uint16_t server_if;
    uint16_t service_handle;
    switch (ctxt->op) {
        case BLE_GATT_REGISTER_OP_SVC:
            ble_server_retrieve_id_by_uuid(ctxt->svc.svc_def->uuid, &server_if);
            BLE_IF_DEBUG("service,uuid16 %s handle=%d (%04X), server_if=%d\r\n",ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf),ctxt->svc.handle, ctxt->svc.handle, server_if);
            ble_server_update_svc_handle(ctxt->svc.svc_def->uuid, ctxt->svc.handle);
            if(gatts_struct_func_ptr_cb)
            {
                btuuid.uuidLen = ctxt->svc.svc_def->uuid->type;
                ble_uuid_flat(ctxt->svc.svc_def->uuid, buf);  
                btuuid.uuid = buf;
                
                if(gatts_struct_func_ptr_cb->serviceAddCb)
                {
                    gatts_struct_func_ptr_cb->serviceAddCb(0/*Always success*/,server_if/**/,&btuuid,ctxt->svc.handle);
                }
            }
            break;

        case BLE_GATT_REGISTER_OP_CHR:
            service_handle = (uint16_t)*(uint16_t *)ctxt->chr.chr_def->arg;
            ble_server_retrieve_id_by_service_id(service_handle, &server_if);
            BLE_IF_DEBUG("charact,uuid16 %s arg %d def_handle=%d (%04X) val_handle=%d (%04X), svc_handle=%d, server_if=%d, arg=0x%08x\r\n",
                ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf),
                (int)ctxt->chr.chr_def->arg,
                ctxt->chr.def_handle, ctxt->chr.def_handle,
                ctxt->chr.val_handle, ctxt->chr.val_handle, service_handle, server_if, (int)ctxt->chr.chr_def->arg);
            
            if(gatts_struct_func_ptr_cb)
            {
                btuuid.uuidLen = ctxt->chr.chr_def->uuid->type;
                ble_uuid_flat(ctxt->chr.chr_def->uuid, buf);  
                btuuid.uuid = buf;
                if(gatts_struct_func_ptr_cb->characteristicAddCb)
                {
                    gatts_struct_func_ptr_cb->characteristicAddCb(0/*Always success*/, server_if/**/, &btuuid ,/*msg->ser_add_char.service_id*/service_handle, ctxt->chr.val_handle);
                }
            }

            break;

        case BLE_GATT_REGISTER_OP_DSC:
            service_handle = (uint16_t)*(uint16_t *)ctxt->dsc.dsc_def->arg;
            ble_server_retrieve_id_by_service_id(service_handle, &server_if);
            BLE_IF_DEBUG("descrip, uuid16 %s arg %d handle=%d (%04X) svc_handle=%d, server_if=%d\r\n",
                ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf),
                (int)ctxt->dsc.dsc_def->arg,
                ctxt->dsc.handle, ctxt->dsc.handle, service_handle, server_if);
            if(gatts_struct_func_ptr_cb)
            {
                btuuid.uuidLen = ctxt->dsc.dsc_def->uuid->type;
                ble_uuid_flat(ctxt->dsc.dsc_def->uuid, buf);  
                btuuid.uuid = buf;
                if(gatts_struct_func_ptr_cb->descriptorAddCb)
                {
                    gatts_struct_func_ptr_cb->descriptorAddCb(0/*Always success*/, server_if/**/, &btuuid ,/*msg->ser_add_char.service_id*/service_handle, ctxt->dsc.handle);
                }
            }
            break;
    }

    return;

}

static int
gap_event(struct ble_gap_event *event, void *arg)
{
    BLE_IF_DEBUG("%s, event->type=%s(%d)\r\n", __FUNCTION__,tls_bt_gap_evt_2_str(event->type),event->type);
    ///ble_server_gap_event(event, (void*)gatts_struct_func_ptr_cb);
    int rc;
    struct ble_gap_conn_desc desc;
    BdAddr bdaddr;
    if(gatts_struct_func_ptr_cb == NULL) return;   

    switch(event->type)
    {
        case BLE_GAP_EVENT_CONNECT:
            BLE_IF_DEBUG("connected handle=%d, status=%d\r\n",event->connect.conn_handle,event->connect.status);
            if (event->connect.status == 0)
            {
                rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
                assert(rc == 0);
                memcpy(bdaddr.addr, desc.peer_id_addr.val, 6);
                g_conn_handle = event->connect.conn_handle;
                /*see nble_server.c ble_server_gap_event will handle this callback*/
                gatts_struct_func_ptr_cb->connectServerCb(event->connect.conn_handle, 0/*Always 0,nimble does not care server if*/,&bdaddr);
            }
            if (event->connect.status != 0) {
                /* Connection failed; resume advertising. */
                //tls_nimble_gap_adv(WM_BLE_ADV_IND, 0);
                BleStartAdvEx(0, g_adv_raw_data, g_adv_param);
            }
            break;
        case BLE_GAP_EVENT_DISCONNECT:
            BLE_IF_DEBUG("disconnect reason=%d\r\n", event->disconnect.reason);

            memcpy(bdaddr.addr, event->disconnect.conn.peer_id_addr.val, 6);
            /*see nble_server.c ble_server_gap_event will handle this callback*/
            gatts_struct_func_ptr_cb->disconnectServerCb(event->disconnect.conn.conn_handle, 0/*Always 0,nimble does not care server if*/,&bdaddr);
            if(event->disconnect.reason == 534)
            {
                //hci error code:  0x16 + 0x200 = 534; //local host terminate the connection;
            }else
            {
                //tls_nimble_gap_adv(WM_BLE_ADV_IND, 0);
                BleStartAdvEx(0, g_adv_raw_data, g_adv_param);
            }
            break;
        case BLE_GAP_EVENT_NOTIFY_TX:
            if(event->notify_tx.status == BLE_HS_EDONE)
            {
                //ble_server_indication_sent_cb(event->notify_tx.attr_handle, event->notify_tx.status);
                //Note, the first param conn__handle, conn_id???  all servcie share one conn_id, so it is not proper 
                gatts_struct_func_ptr_cb->indicationSentCb(event->notify_tx.conn_handle, /*event->notify_tx.status*/0);
            }else
            {
                /*Application will handle other cases*/
            }
            break;
        case BLE_GAP_EVENT_SUBSCRIBE:
            BLE_IF_DEBUG("subscribe indicate(%d,%d),attr_handle=%d\r\n", event->subscribe.prev_indicate,event->subscribe.cur_indicate,event->subscribe.attr_handle );
  
            break;
        case BLE_GAP_EVENT_MTU:
            BLE_IF_DEBUG("wm ble dm mtu changed to(%d)\r\n", event->mtu.value);
            /*nimBLE config prefered ATT_MTU is 256. here 256-12 = 244. */
            /* preamble(1)+access address(4)+pdu(2~257)+crc*/
            /* ATT_MTU(247):pdu= pdu_header(2)+l2cap_len(2)+l2cap_chn(2)+mic(4)*/
            /* GATT MTU(244): ATT_MTU +opcode+chn*/
            //g_mtu = min(244,event->mtu.value - 12);
            if(gatts_struct_func_ptr_cb && gatts_struct_func_ptr_cb->mtuChangeCb)
            gatts_struct_func_ptr_cb->mtuChangeCb(event->mtu.conn_handle, event->mtu.value);
            break;
        case BLE_GAP_EVENT_REPEAT_PAIRING:
            /* We already have a bond with the peer, but it is attempting to
                     * establish a new secure link.  This app sacrifices security for
                     * convenience: just throw away the old bond and accept the new link.
                     */
            /* Delete the old bond. */
            rc = ble_gap_conn_find(event->repeat_pairing.conn_handle, &desc);
            assert(rc == 0);
            ble_store_util_delete_peer(&desc.peer_id_addr);
            
            BLE_IF_DEBUG("!!!BLE_GAP_EVENT_REPEAT_PAIRING\r\n");
            return BLE_GAP_REPEAT_PAIRING_RETRY;
        
        case BLE_GAP_EVENT_PASSKEY_ACTION:
            BLE_IF_DEBUG(">>>BLE_GAP_EVENT_REPEAT_PAIRING\r\n");
            return 0;
        case BLE_GAP_EVENT_ADV_COMPLETE:
            if(gap_func_ptr_cb)gap_func_ptr_cb->advDisableCb(0, event->adv_complete.reason);
            return 0; 
        case BLE_GAP_EVENT_ENC_CHANGE:
            {
                rc = ble_gap_conn_find(event->enc_change.conn_handle, &desc);
                assert(rc == 0);
                memcpy(bdaddr.addr, desc.peer_id_addr.val, 6);
                if(gap_func_ptr_cb)gap_func_ptr_cb->securityRespondCb(&bdaddr);
                return;
            }
        default:
            break;
    }

    return 0;    
}

int BleStopAdv(int advId)
{
    int rc;
    (void)advId;

    rc = ble_gap_adv_stop();

    return rc;
}

int BleStartAdvEx(int serverId, const StartAdvRawData rawData, BleAdvParams advParam) {
    (void)serverId;
    int rc;
    uint8_t own_addr_type;
    ble_addr_t peer_addr;

    //first back up the adv information;
    g_adv_raw_data = rawData;
    g_adv_param = advParam;
    
    struct ble_gap_adv_params adv_params;
    
    if(rawData.advDataLen)
    {
        rc = ble_gap_adv_set_data(rawData.advData,rawData.advDataLen);
        assert(rc == 0);
    }

    if(rawData.rspDataLen)
    {
        rc = ble_gap_adv_rsp_set_data(rawData.rspData,rawData.rspDataLen);
        assert(rc == 0);
    }   

    
    /* Figure out address to use while advertising (no privacy for now) */
    rc = ble_hs_id_infer_auto(0, &own_addr_type);
    if (rc != 0) {
        BLE_IF_DEBUG("error determining address type; rc=%d\r\n", rc);
        return rc;
    }
    /* set adv parameters */
    memset(&adv_params, 0, sizeof(adv_params));

    adv_params.itvl_max = advParam.maxInterval;
    adv_params.itvl_min = advParam.minInterval;
    adv_params.channel_map = advParam.channelMap;
    adv_params.filter_policy = advParam.advFilterPolicy;
    
    switch(advParam.advType)
    {
        case OHOS_BLE_ADV_IND:
                adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
                adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
            break;
        case OHOS_BLE_ADV_NONCONN_IND:
                adv_params.conn_mode = BLE_GAP_CONN_MODE_NON;
                adv_params.disc_mode = BLE_GAP_DISC_MODE_NON;
            break;
        case OHOS_BLE_ADV_SCAN_IND:
                adv_params.conn_mode = BLE_GAP_CONN_MODE_NON;
                adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN; // LTD same as GEN;
            break;
        case OHOS_BLE_ADV_DIRECT_IND_HIGH:
                adv_params.high_duty_cycle = 1;
                //passthrough;
        case OHOS_BLE_ADV_DIRECT_IND_LOW:
                adv_params.conn_mode = BLE_GAP_CONN_MODE_DIR;
                adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
            break;
    } 

    peer_addr.type = advParam.peerAddrType;
    memcpy(&peer_addr.val[0], &advParam.peerAddr.addr[0], 6);

    BLE_IF_DEBUG("Starting advertising\r\n");
    
    /* As own address type we use hard-coded value, because we generate
          NRPA and by definition it's random */
    /*NOTE: own_addr_type, we actually used, not specified by param->ownaddType*/
    rc = ble_gap_adv_start(own_addr_type, &peer_addr, advParam.duration?advParam.duration:BLE_HS_FOREVER,
                           &adv_params, gap_event, NULL);
    if(rc)
    {
       BLE_IF_PRINTF("Starting advertising failed, rc=%d\r\n", rc); 
    }  

    
    return OHOS_BT_STATUS_SUCCESS;
}

int BleGattsStartServiceEx(int *srvcHandle, BleGattService *srvcInfo) {
    int rc = 0;
    int server_if = 0;
    int adv_active = 0;
    uint16_t service_handle_r = 0;
    
    BLE_IF_DEBUG("BleGattsStartServiceEx\r\n");
    adv_active = ble_gap_adv_active();
    if(adv_active)
    {
        rc = ble_gap_adv_stop();
    }
    
    rc = ble_gatts_reset();
    if(rc != 0)
    {
        BLE_IF_DEBUG("!!!BleGattsStartServiceEx failed!!! rc=%d\r\n", rc);
        return rc;
    }
    
    server_if = ble_server_alloc(srvcInfo);
    *srvcHandle  = server_if;
    
    ble_server_start_service();

    if(adv_active)
    {
        BleStartAdvEx(0, g_adv_raw_data, g_adv_param);
    }
    
    //report servcie startcb
    if(gatts_struct_func_ptr_cb)
    {
        if(gatts_struct_func_ptr_cb->serviceStartCb)
        {
            ble_server_retrieve_service_handle_by_server_id(server_if, &service_handle_r);
            gatts_struct_func_ptr_cb->serviceStartCb(0/*Always success*/, server_if, service_handle_r);
        }
    }

    BLE_IF_DEBUG("ble server alloc return=%d\r\n", server_if);
    return OHOS_BT_STATUS_SUCCESS;
}

int BleGattsStopServiceEx(int srvcHandle) {
    uint16_t service_handle_r = 0;
    
    ble_gap_adv_stop();
    ble_server_free(srvcHandle);

    if(gatts_struct_func_ptr_cb)
    {
        if(gatts_struct_func_ptr_cb->serviceStopCb)
        {
            ble_server_retrieve_service_handle_by_server_id(srvcHandle/*actually, this is server_if*/, &service_handle_r);
            gatts_struct_func_ptr_cb->serviceStopCb(0/*Always success*/, srvcHandle/*server_if*/, service_handle_r);
        }
    }    
    return OHOS_BT_STATUS_SUCCESS;
}


////////////////////test case/////////////////////////////////////
static int test_indicate(void *arg); 


int char_write(uint8_t *data, int length)
{
    int i = 0;
    printf("char1 write:(%d)\r\n", length);
    for(i=0; i<length; i++)
    {
        printf("%02x ", data[i]);
    }
    printf("\r\n");


    tls_bt_async_proc_func(test_indicate, NULL,20);
}
int char_read(uint8_t *data, int *length)
{
    data[0] = '1';
    data[1] = '1';
    *length = 2;
}
int char2_write(uint8_t *data, int length)
{
    int i = 0;
    printf("char2 write:(%d)\r\n", length);
    for(i=0; i<length; i++)
    {
        printf("%02x ", data[i]);
    }
    printf("\r\n");

}
int char2_read(uint8_t *data, int *length)
{
    data[0] = '2';
    data[1] = '2';
    *length = 2;
}
void test_remove_service(void *arg)
{
    printf("Free server if=0\r\n");
    ble_gap_adv_stop();
    
    ble_server_free(0);

    ble_server_start_service();
}

void test_add_service3()
{
    //
    int ret, server_if;
    uint16_t uuid;

    uint8_t uuid_128_service[16]= {0x00,0x91,0x8A,0xEF,0x39,0xDD,0x84,0xA4,0xFC,0x43,0x77,0xA2,0x00,0xE4,0xF1,0x15};
    uint8_t uuid_128_char1[16]=   {0x00,0x91,0x8A,0xEF,0x39,0xDD,0x84,0xA4,0xFC,0x43,0x77,0xA2,0x01,0xE4,0xF1,0x15};   
    
    BleGattService service;
    BleGattOperateFunc func, func1;
    BleGattAttr attr[5];
    attr[0].attrType = OHOS_BLE_ATTRIB_TYPE_SERVICE;
    #if 0
    //attr[0].uuidType = OHOS_UUID_TYPE_16_BIT;
    //uuid = 0xE600;
    //memcpy(attr[0].uuid, &uuid, 2);
    #else
    attr[0].uuidType = OHOS_UUID_TYPE_128_BIT;
    memcpy(attr[0].uuid, uuid_128_service, 16);    
    #endif

    attr[1].attrType = OHOS_BLE_ATTRIB_TYPE_CHAR;
    #if 0
    attr[1].uuidType = OHOS_UUID_TYPE_16_BIT; 
    uuid = 0xE601;
    memcpy(attr[1].uuid, &uuid, 2);
    #else
    attr[1].uuidType = OHOS_UUID_TYPE_128_BIT; 
    memcpy(attr[1].uuid, uuid_128_char1, 16);    
    #endif
    func.write = char_write;
    func.read = char_read;
    attr[1].func = func;
    attr[1].properties = 0x0A;
    attr[1].permission = 0x00;

     attr[2].attrType = OHOS_BLE_ATTRIB_TYPE_CHAR_USER_DESCR;
     attr[2].uuidType = OHOS_UUID_TYPE_16_BIT;
     uuid = 0x2902;
     memcpy(attr[2].uuid, &uuid, 2);
     func.write = char_write;
     func.read = char_read;
     attr[2].func = func; 
     attr[2].permission = 0x11;    

    service.attrNum = 3;
    service.attrList = &attr[0];
     
     ret = BleGattsStartServiceEx(&server_if, &service);
     printf("adding service1, ret=%d, server_if=%d\r\n", ret, server_if);

}
uint8_t indicate_data[128]= {0x01, 0x4F, 0x01, 0x00, 0x00, 0x00, 0x00, 0x11, 0x09, 0x6e, 0x65, 0x74, 0x43, 
    0x66, 0x67,0x56,0x65, 0x72, 0x09, 0x00, 0x7b, 0x22, 0x76, 0x65, 0x72, 0x22, 0x3a, 0x32, 0x7d};

int test_indicate(void *arg) 
{

    int rc;
    (void)arg;
    struct os_mbuf *om;
    indicate_data[0] = 0x01;
    om = ble_hs_mbuf_from_flat(indicate_data, 29);
    if (!om) {
        return BLE_HS_ENOMEM;
    }
#if BLE_IF_DBG    
    ble_server_gatt_dump_hexstring("FROM HILINK:", indicate_data, 29);
#endif  

    rc = ble_gattc_indicate_custom(g_conn_handle, 3, om);

    assert(rc == 0);
    
    return OHOS_BT_STATUS_SUCCESS;
}


void test_add_service()
{
    //
    int ret, server_if;
    uint16_t uuid;

    uint8_t uuid_128_service[16]= {0x00,0x91,0x8A,0xEF,0x39,0xDD,0x84,0xA4,0xFC,0x43,0x77,0xA2,0x00,0xE6,0xF1,0x15};
    uint8_t uuid_128_char1[16]=   {0x00,0x91,0x8A,0xEF,0x39,0xDD,0x84,0xA4,0xFC,0x43,0x77,0xA2,0x01,0xE6,0xF1,0x15};
    uint8_t uuid_128_char2[16]=   {0x00,0x91,0x8A,0xEF,0x39,0xDD,0x84,0xA4,0xFC,0x43,0x77,0xA2,0x02,0xE6,0xF1,0x15};    
    
    BleGattService service;
    BleGattOperateFunc func, func1;
    BleGattAttr attr[5];
    attr[0].attrType = OHOS_BLE_ATTRIB_TYPE_SERVICE;
    #if 0
    //attr[0].uuidType = OHOS_UUID_TYPE_16_BIT;
    //uuid = 0xE600;
    //memcpy(attr[0].uuid, &uuid, 2);
    #else
    attr[0].uuidType = OHOS_UUID_TYPE_128_BIT;
    memcpy(attr[0].uuid, uuid_128_service, 16);    
    #endif

    attr[1].attrType = OHOS_BLE_ATTRIB_TYPE_CHAR;
    #if 0
    attr[1].uuidType = OHOS_UUID_TYPE_16_BIT; 
    uuid = 0xE601;
    memcpy(attr[1].uuid, &uuid, 2);
    #else
    attr[1].uuidType = OHOS_UUID_TYPE_128_BIT; 
    memcpy(attr[1].uuid, uuid_128_char1, 16);    
    #endif
    func.write = char_write;
    func.read = char_read;
    attr[1].func = func;
    attr[1].properties = 0x22;
    attr[1].permission = 0x00;

     attr[2].attrType = OHOS_BLE_ATTRIB_TYPE_CHAR_USER_DESCR;
     attr[2].uuidType = OHOS_UUID_TYPE_16_BIT;
     uuid = 0x2902;
     memcpy(attr[2].uuid, &uuid, 2);
     func.write = char_write;
     func.read = char_read;
     attr[2].func = func; 
     attr[2].permission = 0x11;    

    attr[3].attrType = OHOS_BLE_ATTRIB_TYPE_CHAR;
    #if 0
    attr[3].uuidType = OHOS_UUID_TYPE_16_BIT; 
    uuid = 0xE602;
    memcpy(attr[3].uuid, &uuid, 2);
    #else
    attr[3].uuidType = OHOS_UUID_TYPE_128_BIT; 
    memcpy(attr[3].uuid, uuid_128_char2, 16);     
    #endif
    func.write = char_write;
    func.read = char_read;
    attr[3].func = func;
    attr[3].properties = 0x08;
    attr[3].permission = 0x00;
    
    attr[4].attrType = OHOS_BLE_ATTRIB_TYPE_CHAR_USER_DESCR;
    attr[4].uuidType = OHOS_UUID_TYPE_16_BIT;
    uuid = 0x2902;
    memcpy(attr[4].uuid, &uuid, 2);
    func.write = char2_write;
    func.read = char2_read;
    attr[4].func = func; 
    attr[4].permission = 0x11;


#if 0

     attr[2].attrType = OHOS_BLE_ATTRIB_TYPE_CHAR_USER_DESCR;
     attr[2].uuidType = OHOS_UUID_TYPE_16_BIT;
     uuid = 0x2902;
     memcpy(attr[2].uuid, &uuid, 2);
     func.write = char_write;
     func.read = char_read;
     attr[2].func = func; 
     attr[2].permission = 0x11;

    attr[3].attrType = OHOS_BLE_ATTRIB_TYPE_CHAR;
    attr[3].uuidType = OHOS_UUID_TYPE_16_BIT; 
    uuid = 0x2ab8;
    memcpy(attr[3].uuid, &uuid, 2);
    func.write = char2_write;
    attr[3].func = func;
    attr[3].properties = 0x0A;
    attr[3].permission = 0x00;

    attr[4].attrType = OHOS_BLE_ATTRIB_TYPE_CHAR_USER_DESCR;
    attr[4].uuidType = OHOS_UUID_TYPE_16_BIT;
    uuid = 0x2902;
    memcpy(attr[4].uuid, &uuid, 2);
    func.write = char2_write;
    func.read = char2_read;
    attr[4].func = func; 
    attr[4].permission = 0x11;
#endif
    service.attrNum = 5;
    service.attrList = &attr[0];
     
     ret = BleGattsStartServiceEx(&server_if, &service);
     printf("adding service1, ret=%d, server_if=%d\r\n", ret, server_if);

}

void test_add_service2()
{
    //
    int ret, server_if;
    uint16_t uuid;
    
    BleGattService service;
    BleGattOperateFunc func, func1;
    BleGattAttr attr[5];
    attr[0].attrType = OHOS_BLE_ATTRIB_TYPE_SERVICE;
    attr[0].uuidType = OHOS_UUID_TYPE_16_BIT;
    uuid = 0x1826;
    memcpy(attr[0].uuid, &uuid, 2);

    attr[1].attrType = OHOS_BLE_ATTRIB_TYPE_CHAR;
    attr[1].uuidType = OHOS_UUID_TYPE_16_BIT; 
    uuid = 0x2abc;
    memcpy(attr[1].uuid, &uuid, 2);
    func.write = char2_write;
    func.read = char2_read;
    attr[1].func = func;
    attr[1].properties = 0x28;
    attr[1].permission = 0x00;
#if 0
     attr[2].attrType = OHOS_BLE_ATTRIB_TYPE_CHAR_USER_DESCR;
     attr[2].uuidType = OHOS_UUID_TYPE_16_BIT;
     uuid = 0x2902;
     memcpy(attr[2].uuid, &uuid, 2);
     func.write = char2_write;
     func.read = char2_read;
     attr[2].func = func; 
     attr[2].permission = 0x11;
#endif
    attr[2].attrType = OHOS_BLE_ATTRIB_TYPE_CHAR;
    attr[2].uuidType = OHOS_UUID_TYPE_16_BIT; 
    uuid = 0x2ab8;
    memcpy(attr[2].uuid, &uuid, 2);
    func.write = char2_write;
    attr[2].func = func;
    attr[2].properties = 0x28;
    attr[2].permission = 0x00;
#if 0

    attr[4].attrType = OHOS_BLE_ATTRIB_TYPE_CHAR_USER_DESCR;
    attr[4].uuidType = OHOS_UUID_TYPE_16_BIT;
    uuid = 0x2902;
    memcpy(attr[4].uuid, &uuid, 2);
    func.write = char2_write;
    func.read = char2_read;
    attr[4].func = func; 
    attr[4].permission = 0x11;
#endif
    service.attrNum = 3;
    service.attrList = &attr[0];
     
     ret = BleGattsStartServiceEx(&server_if, &service);
     printf("adding service2, ret=%d, server_if=%d\r\n", ret, server_if);

}


void test_registerServerCallback(int status, int serverId, BtUuid *appUuid)
{
  BLE_IF_DEBUG("%s serverId=%d\r\n",__FUNCTION__, serverId);  
}

void test_connectServerCallback(int connId, int serverId, BdAddr *bdAddr)
{
    BLE_IF_DEBUG("%s serverId=%d\r\n",__FUNCTION__, serverId);

}

void test_disconnectServerCallback(int connId, int serverId, BdAddr *bdAddr)
{
    BLE_IF_DEBUG("%s serverId=%d\r\n",__FUNCTION__, serverId);

}

void test_serviceAddCallback(int status, int serverId, BtUuid *uuid, int srvcHandle)
{
    BLE_IF_DEBUG("%s serverId=%d\r\n",__FUNCTION__, serverId);

}

void test_includeServiceAddCallback(int status, int serverId, int srvcHandle, int includeSrvcHandle)
{
    BLE_IF_DEBUG("%s serverId=%d\r\n",__FUNCTION__, serverId);

}

void test_characteristicAddCallback(int status, int serverId, BtUuid *uuid,
                                                     int srvcHandle, int characteristicHandle)
{
    BLE_IF_DEBUG("%s serverId=%d\r\n",__FUNCTION__, serverId);

}

void test_descriptorAddCallback(int status, int serverId, BtUuid *uuid,
                                               int srvcHandle, int descriptorHandle)
{
    BLE_IF_DEBUG("%s serverId=%d\r\n",__FUNCTION__, serverId);

}

void test_serviceStartCallback(int status, int serverId, int srvcHandle)
{
    BLE_IF_DEBUG("%s serverId=%d\r\n",__FUNCTION__, serverId);

}

void test_serviceStopCallback(int status, int serverId, int srvcHandle)
{
    BLE_IF_DEBUG("%s serverId=%d\r\n",__FUNCTION__, serverId);

}

void test_serviceDeleteCallback(int status, int serverId, int srvcHandle)
{
    BLE_IF_DEBUG("%s serverId=%d\r\n",__FUNCTION__, serverId);

}

void test_requestReadCallback(BtReqReadCbPara readCbPara)
{
    BLE_IF_DEBUG("%s serverId=%d\r\n",__FUNCTION__, 0);

}

void test_requestWriteCallback(BtReqWriteCbPara writeCbPara)
{
    BLE_IF_DEBUG("%s serverId=%d\r\n",__FUNCTION__, 0);

}

void test_responseConfirmationCallback(int status, int handle)
{
    BLE_IF_DEBUG("%s serverId=%d\r\n",__FUNCTION__, 0);

}

void test_indicationSentCallback(int connId, int status)
{
    BLE_IF_DEBUG("%s connId=%d\r\n",__FUNCTION__, connId);

}

void test_mtuChangeCallback(int connId, int mtu)
{
    BLE_IF_DEBUG("%s connId=%d\r\n",__FUNCTION__, connId);

}
static BtGattServerCallbacks scb={

    test_registerServerCallback,
    
    test_connectServerCallback,
    
    test_disconnectServerCallback,
    
    test_serviceAddCallback,
    
    test_includeServiceAddCallback,
    
    test_characteristicAddCallback,
    
    test_descriptorAddCallback,
    
    test_serviceStartCallback,
    
    test_serviceStopCallback,
    
    test_serviceDeleteCallback,
    
    test_requestReadCallback,
    
    test_requestWriteCallback,
    
    test_responseConfirmationCallback,
    
    test_indicationSentCallback,
    
    test_mtuChangeCallback,
};

void test_adv()
{
    StartAdvRawData raw;
    BleAdvParams param;


    memset(&raw, 0, sizeof(raw));
    memset(&param, 0, sizeof(param));
    
    uint8_t adv_data[] = {
            0x02,0x01,0x06,
            0x15,0x16,0xEE,0xFD,0x01,0x01,0x07,0x04,0x00,0x11,0xF8,0x12,0x44,0x30,0x30,0x32,0xFF,0x00,0x04,0x02,0x02,0x01
           }; 
    uint8_t scan_data[] = {
            0x14,0x09,0x48,0x69,0x2D,0x42,0x72,0x6f,0x61,0x53,0x77,0x69,0x74,0x2d,0x31,0x44,0x30,0x30,0x32,0x30,0x30 
            };

    raw.advDataLen = 25;
    raw.advData = adv_data;

    raw.rspDataLen = 21;
    raw.rspData = scan_data;    

    param.advFilterPolicy = 0;
    param.advType = OHOS_BLE_ADV_IND;
    param.channelMap = 0x07;
    param.ownAddrType = 0;
    param.duration = 0; 
    param.maxInterval = 0x60;
    param.minInterval = 0x40;


    BleGattsRegisterCallbacks(&scb);
    
    BLE_IF_DEBUG("Adding service....\r\n");
    
    //test_add_service3();
    //test_add_service2();
    test_add_service();
    BLE_IF_DEBUG("Runnng service....\r\n");

    //tls_bt_async_proc_func(test_remove_service,(void*)1,3000);

    BLE_IF_DEBUG("Starting advertisment...\r\n");
    BleStartAdvEx(1, raw, param);
  
}


