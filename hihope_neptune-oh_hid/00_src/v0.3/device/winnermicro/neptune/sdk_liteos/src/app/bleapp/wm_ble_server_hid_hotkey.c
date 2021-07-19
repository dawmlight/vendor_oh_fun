/*****************************************************************************
**
**  Name:           wm_ble_server_hid_hotkey.c
**
**  Description:    This file contains the  implemention of hotkey: detecting /processing/...
**
*****************************************************************************/
#include <assert.h>
#include <stdint.h>
#include "wm_bt_config.h"

#if (WM_NIMBLE_INCLUDED == CFG_ON)

#include "host/ble_hs.h"
#include "wm_bt_app.h"
#include "wm_bt_util.h"
#include "wm_bt_storage.h"

#include "wm_ble_server_hid_demo.h"

static uint8_t sfn[8] =        {0x04, 0x00, 0x2e, 0x00, 0x00, 0x00, 0x00, 0x00}; //alt+=   adv mode;
static uint8_t sfn_delete[8] = {0x04, 0x00, 0x2d, 0x00, 0x00, 0x00, 0x00, 0x00}; //alt+-  delete the oldest bonding device
static uint8_t sfn1[8] =       {0x04, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00}; //alt+1
static uint8_t sfn2[8] =       {0x04, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00}; //alt+2
static uint8_t sfn3[8] =       {0x04, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00}; //alt+3
static uint8_t sfn4[8] =       {0x04, 0x00, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00}; //alt+4
static uint8_t sfn5[8] =       {0x04, 0x00, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00}; //alt+5
static uint8_t sfn_clear[8] =  {0x04, 0x00, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00}; //alt+0   clear flash;
static uint8_t sfn_dsleep[8] =  {0x04, 0x00, 0x26, 0x00, 0x00, 0x00, 0x00, 0x00}; //alt+9   disable sleep;
static uint8_t sfn_esleep[8] =  {0x04, 0x00, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00}; //alt+8   enable sleep;




typedef enum{
  HID_DEV_KEY_IDLE,
  HID_DEV_KEY_PRESS,
  HID_DEV_KEY_LONG_PRESS,
} hid_key_action_t;

typedef enum{
    HID_DEV_IDLE,
    HID_DEV_ADDING,
    HID_DEV_BONDED_AND_CONNECTED,
    HID_DEV_SWITCHING_TOME,
    HID_DEV_BONDED_AND_DISCONNECTED,
    HID_DEV_DISCONNECTING,
    HID_DEV_REPLACING,
} hid_dev_state_t;

typedef struct{
    uint8_t hot_key;
    uint32_t hot_key_press_ticks;
    hid_dev_state_t state;
    ble_addr_t addr;
} hid_dev_t;

#define MAX_BONDING_DEV_COUNT 5
static hid_dev_t hid_dev_array[MAX_BONDING_DEV_COUNT];


#define LONG_TIME_PRESS_MS_THRESHOLD  2000
/*return  0: no hotkey detected;
              1: normay press key detected
              2: long time press key detected(2s)
              */
int check_hotkey_released(uint8_t  *key_index)
{
    int rc = 0;
    int i = 0;
    uint32_t time_pressed = 0;
    uint32_t time_now = tls_os_get_time();
    uint32_t duration = 0;
    
    for(i=0; i<MAX_BONDING_DEV_COUNT; i++)
    {
        time_pressed = hid_dev_array[i].hot_key_press_ticks;
        hid_dev_array[i].hot_key_press_ticks = 0;  //clear it;
        
        if(time_pressed)
        {
            *key_index = (uint8_t)i;
            
            if(time_now >= time_pressed)
            {
                duration = time_now - time_pressed;
            }else if (time_now < time_pressed)
            {
                duration = (0xFFFFFFFF - time_pressed) + time_now;
            }
            
            if(duration < ble_npl_time_ms_to_ticks32(LONG_TIME_PRESS_MS_THRESHOLD))
            {
                return HID_DEV_KEY_PRESS;
            }else
            {
                return HID_DEV_KEY_LONG_PRESS;
            }
        }
    }

    return rc;
}

int check_hotkey_pressed(uint8_t *key_array, int len)
{
    int rc = 0;
    uint8_t connected = 0;
    ble_addr_t peer_addr;

    if(key_array[0] == 0x04) printf("0x%02x\r\n", key_array[2]);
    
    if(memcmp(key_array, sfn, 4)==0)
    {

        TLS_BT_APPL_TRACE_API("### ALT + = detected\r\n");
        connected = tls_ble_server_demo_hid_connected();
        
        if(connected)
        {
            TLS_BT_APPL_TRACE_API("### hid service connected with remote device!!!\r\n");
            return 0;
        }else
        {
            TLS_BT_APPL_TRACE_API("### hid service start advertising\r\n");
            tls_ble_server_demo_hid_advertise(1, NULL);            
        }  
    }else if(memcmp(key_array, sfn_delete, 4)==0)
    {
        
        rc = ble_store_util_delete_oldest_peer(&peer_addr);
        TLS_BT_APPL_TRACE_API("### hid service delete oldest bonding device[%02x:%02x:%02x:%02x:%02x:%02x], rc=%d\r\n",
            peer_addr.val[0],peer_addr.val[1],peer_addr.val[2],peer_addr.val[3],peer_addr.val[4],peer_addr.val[5], rc);
        if(rc == 0)
        {
            remove_hid_dev(&peer_addr);
        }
        
    }else if(memcmp(key_array, sfn_clear, 4)==0)
    {
        TLS_BT_APPL_TRACE_API("### hid service clear bonding information\r\n");
        tls_param_to_default();
    }else if(memcmp(key_array, sfn_dsleep, 4)==0)
    {
        TLS_BT_APPL_TRACE_API("### hid service disable sleep\r\n");
        tls_bt_ctrl_sleep(false);
    }else if(memcmp(key_array, sfn_esleep, 4)==0)
    {
        TLS_BT_APPL_TRACE_API("### hid service enable sleep\r\n");
        tls_bt_ctrl_sleep(true);
    }else if(memcmp(key_array, sfn1, 4) == 0)
    {
        if(!hid_dev_array[0].hot_key_press_ticks)
        {
            hid_dev_array[0].hot_key_press_ticks = tls_os_get_time();
            
        }
        rc = 1;

    }else if(memcmp(key_array, sfn2, 4) == 0)
    {
        if(!hid_dev_array[1].hot_key_press_ticks)
        {
            hid_dev_array[1].hot_key_press_ticks = tls_os_get_time();
            
        }
        rc = 1;

    }else if(memcmp(key_array, sfn3, 4) == 0)
    {
        if(!hid_dev_array[2].hot_key_press_ticks)
        {
            hid_dev_array[2].hot_key_press_ticks = tls_os_get_time(); 
        }
        rc = 1;
    
    }else if(memcmp(key_array, sfn4, 4) == 0)
    {
        if(!hid_dev_array[3].hot_key_press_ticks)
        {
            hid_dev_array[3].hot_key_press_ticks = tls_os_get_time(); 
        }
        rc = 1;
   
    }else if(memcmp(key_array, sfn5, 4) == 0)
    {
        if(!hid_dev_array[4].hot_key_press_ticks)
        {
            hid_dev_array[4].hot_key_press_ticks = tls_os_get_time();
            
        }
        rc = 1;    
    #if 0
        rc = ble_store_util_get_peer_by_index(4, &peer_addr);
        if(rc == 0)
        {
            TLS_BT_APPL_TRACE_DEBUG("### hid service adding device 5(%02x:%02x:%02x:%02x:%02x:%02x[%d])\r\n", peer_addr.val[0], peer_addr.val[1],
                peer_addr.val[2],peer_addr.val[3],peer_addr.val[4],peer_addr.val[5], peer_addr.type);
            tls_ble_server_demo_hid_advertise(1, (void*)&peer_addr);
        }else
        {
            TLS_BT_APPL_TRACE_WARNING("### hid service adding device 5, but it is not available now\r\n");
        }
    #endif    
    }
 
    return rc;
}

static bool check_hid_dev_array_connected(uint8_t *dev_index)
{
    int i = 0;
    bool found = false;
    
    for(i=0; i< MAX_BONDING_DEV_COUNT; i++)
    {
        if(hid_dev_array[i].state == HID_DEV_BONDED_AND_CONNECTED)
        {
            *dev_index = i;
            found = true;
            break;
        }
    }

    return found;
}
static bool check_hid_dev_array_adding(uint8_t *dev_index)
{
    int i = 0;
    bool found = false;
    
    for(i=0; i< MAX_BONDING_DEV_COUNT; i++)
    {
        if(hid_dev_array[i].state == HID_DEV_ADDING)
        {
            *dev_index = i;
            found = true;
            break;
        }
    }

    return found;
}
static bool check_hid_dev_array_switching(uint8_t *dev_index)
{
    int i = 0;
    bool found = false;
    
    for(i=0; i< MAX_BONDING_DEV_COUNT; i++)
    {
        if(hid_dev_array[i].state == HID_DEV_SWITCHING_TOME)
        {
            *dev_index = i;
            found = true;
            break;
        }
    }

    return found;
}
static bool check_hid_dev_array_matching(ble_addr_t *addr)
{
    int i = 0;
    bool found = false;
    
    for(i=0; i< MAX_BONDING_DEV_COUNT; i++)
    {
        if(hid_dev_array[i].state == HID_DEV_BONDED_AND_DISCONNECTED && (!ble_addr_cmp(&hid_dev_array[i].addr, addr)))
        {
            found = true;
            break;
        }
    }

    return found;
}

/*key_action: 1 normal press, 2 long time press*/
int process_hotkey(uint8_t key_action, uint8_t key_index)
{
    bool found_connected = false;
    bool found_adding = false;
    bool found_switching = false;
    uint8_t dev_index = 0, dev_tmp = 0;
    
    TLS_BT_APPL_TRACE_API("processing hotkey, action=%s, key_index=%d\r\n", key_action==1?"switching":"adding", key_index);
       
    switch(key_action)
    {
        case HID_DEV_KEY_LONG_PRESS:    //long time press processing, adding device;

            found_adding = check_hid_dev_array_adding(&dev_tmp);
            if(found_adding)
            {
               TLS_BT_APPL_TRACE_WARNING("system is busying, device %d is adding\r\n", dev_tmp);
               return -1;
            }
            
            found_switching = check_hid_dev_array_switching(&dev_tmp);
            if(found_switching)
            {
               TLS_BT_APPL_TRACE_WARNING("system is busying, device %d is switching\r\n", dev_tmp);
               return -1;            
            }

            found_connected = check_hid_dev_array_connected(&dev_index);
            
            if(!ble_addr_cmp(&hid_dev_array[key_index].addr,BLE_ADDR_ANY))
            {
                if(found_connected)
                {
                    assert(key_index != dev_index);
                    hid_dev_array[dev_index].state = HID_DEV_DISCONNECTING;
                    hid_dev_array[key_index].state = HID_DEV_ADDING;
                    ///inform controller decline the connect_req all connected device;
                    refresh_hid_dev_black_white_list();
                    
                    tls_ble_server_demo_hid_disconnect(true);
                }else
                {
                    hid_dev_array[key_index].state = HID_DEV_ADDING;

                    refresh_hid_dev_black_white_list();
                    
                    ///inform controller decline the connect_req all connected device;
                    tls_ble_server_demo_hid_advertise(WM_BLE_ADV_IND, NULL);
                }
                
            }else
            {
                if(found_connected)
                {
                    if(dev_index == key_index)
                    {
                        hid_dev_array[key_index].state = HID_DEV_REPLACING; 
                    }else
                    {
                        hid_dev_array[dev_index].state = HID_DEV_DISCONNECTING;
                        hid_dev_array[key_index].state = HID_DEV_ADDING;
                    }
                    ///inform controller decline the connect_req all connected device;
                    refresh_hid_dev_black_white_list();
                    
                    tls_ble_server_demo_hid_disconnect(true);
                }else
                {
                    hid_dev_array[key_index].state = HID_DEV_ADDING;
                    ///inform controller decline the connect_req all connected device;

                    refresh_hid_dev_black_white_list();
                    
                    tls_ble_server_demo_hid_advertise(WM_BLE_ADV_IND, NULL);
                }
                
                //remove bonding peer sec information;
            }
            break;
        case HID_DEV_KEY_PRESS:   //normal press processing, device switching;

            found_switching = check_hid_dev_array_switching(&dev_tmp);
            if(found_switching)
            {
                TLS_BT_APPL_TRACE_WARNING("system is busying, device is switching to %d\r\n", dev_tmp);
                return -1;
            }
            found_adding = check_hid_dev_array_adding(&dev_tmp);
            if(found_adding)
            {
                TLS_BT_APPL_TRACE_WARNING("system is busying, device %d is adding\r\n", dev_tmp);
                return -1;
            }            
            
            if(!ble_addr_cmp(&hid_dev_array[key_index].addr,BLE_ADDR_ANY))
            {
                TLS_BT_APPL_TRACE_WARNING("Invalid action, switch to hid device index=%d, no bonding address\r\n", key_index);
            }else
            {
                found_connected = check_hid_dev_array_connected(&dev_index);

                if(found_connected)
                {
                    if(key_index == dev_index)
                    {
                        TLS_BT_APPL_TRACE_WARNING("Invalid action, switching to myself\r\n");
                    }else
                    {
                        hid_dev_array[dev_index].state = HID_DEV_DISCONNECTING;
                        hid_dev_array[key_index].state = HID_DEV_SWITCHING_TOME;
                        //TODO inform controller accept hid_dev_array[key_index].addr only!!!!
                        //!!!
                        refresh_hid_dev_black_white_list();
                        
                        tls_ble_server_demo_hid_disconnect(true);
                    }
                }else
                {
                    //TODO inform controller accept hid_dev_array[key_index].addr only!!!!
                    //!!!
                    hid_dev_array[key_index].state = HID_DEV_SWITCHING_TOME;

                    refresh_hid_dev_black_white_list();
                    
                    tls_ble_server_demo_hid_advertise(WM_BLE_ADV_IND, NULL);
                }
                
         
                //inform controller only accept the connection with this address;

                //if connected with other device, disconnect it now.

                //enable advertising now
            }
            break;
        default:
            break;
    }
}

static void update_dev_to_nvram(bool force_flush)
{
    int index = 0, i = 0, j = 0;
    uint8_t nvram_value[MAX_BONDING_DEV_COUNT*9] = {0};
    uint8_t offset = 0;
    //map hid_dev_array to nvram_value

    for(index = 0; index< MAX_BONDING_DEV_COUNT; index++)
    {
        nvram_value[offset] = hid_dev_array[index].hot_key;
        offset++;
        nvram_value[offset] = hid_dev_array[index].state;
        offset++;
        nvram_value[offset] = hid_dev_array[index].addr.type;
        offset++;
        memcpy(nvram_value+offset, &hid_dev_array[index].addr.val[0], 6);
        offset += 6;
    }
#if 1   
    TLS_BT_APPL_TRACE_API("Dump nvram information, then write to nvram\r\n");
    for(i = 0; i<45; i++)
    {
        j++;
        printf("%02x ", nvram_value[i]);
        if(j == 9)
        {
            j = 0;
            printf("\r\n");
        }
    }
 #endif
    btif_config_store_key_map(nvram_value, sizeof(nvram_value), force_flush);
    
}

static void load_nvram_to_dev(uint8_t *nvram_info)
{
   int index = 0, i=0, j=0;
   int index_action = -1;
   uint8_t *src = nvram_info;
   uint8_t offset = 0;
   
    TLS_BT_APPL_TRACE_API("Dump nvram information, then load to dev array\r\n");
    #if 0
    for(i = 0; i<45; i++)
    {
        j++;
        printf("%02x ", nvram_info[i]);
        if(j == 9)
        {
            j = 0;
            printf("\r\n");
        }
    }   
   #endif
   for(index = 0; index<MAX_BONDING_DEV_COUNT; index++)
   {
        hid_dev_array[index].hot_key = src[offset++];
        hid_dev_array[index].state = src[offset++];
        if(hid_dev_array[index].state == HID_DEV_BONDED_AND_CONNECTED || hid_dev_array[index].state == HID_DEV_SWITCHING_TOME)
        {
            TLS_BT_APPL_TRACE_DEBUG("Found old state=%d\r\n", hid_dev_array[index].state);
            index_action = index;
            //// convert to this state, one adv enabled, only accept this device
            hid_dev_array[index].state = HID_DEV_SWITCHING_TOME;
        }else if(hid_dev_array[index].state == HID_DEV_ADDING)
        {
            index_action = index;
        }
        hid_dev_array[index].addr.type = src[offset++];
        memcpy(&hid_dev_array[index].addr.val[0], src+offset, 6);
        offset += 6;
        hid_dev_array[index].hot_key_press_ticks = 0;
   }

   if(index_action >= 0)
   {
        TLS_BT_APPL_TRACE_API("After loading nvram, adv started, and accept device (%d) only!!!\r\n", index_action);

        refresh_hid_dev_black_white_list();
        tls_ble_server_demo_hid_advertise(WM_BLE_ADV_IND, NULL);
   }
   
}

int remove_hid_dev(ble_addr_t *addr)
{
    int index = 0;
    bool found = false;
    
    for(index = 0; index < MAX_BONDING_DEV_COUNT; index++)
    {
        if(!ble_addr_cmp(&hid_dev_array[index].addr, addr))
        {
            found = true;
            break;
        }
    }

    if(found)
    {
        TLS_BT_APPL_TRACE_API("Remove hid dev %d, it is the oldest one\r\n", index);
        if(hid_dev_array[index].state == HID_DEV_BONDED_AND_CONNECTED)
        {
            TLS_BT_APPL_TRACE_DEBUG("The oldest one connected, disconnect with it\r\n");
            tls_ble_server_demo_hid_disconnect(false);
        }
        memset(&hid_dev_array[index], 0 , sizeof(hid_dev_t));
        update_dev_to_nvram(true);
    }
    
}

void refresh_hid_dev_black_white_list()
{
    int rc = 0;
    int index = 0;
    int offset = 0;
    ble_addr_t addr_list[MAX_BONDING_DEV_COUNT] = {0};

    for(index = 0; index<MAX_BONDING_DEV_COUNT; index++)
    {
        if(hid_dev_array[index].state == HID_DEV_DISCONNECTING || hid_dev_array[index].state == HID_DEV_BONDED_AND_DISCONNECTED || hid_dev_array[index].state == HID_DEV_REPLACING)
        {
            addr_list[offset].type = 0x04;  ///black list //bit 0 must be zero;
            memcpy(addr_list[offset].val, hid_dev_array[index].addr.val, 6);
            offset ++;
        }else if(hid_dev_array[index].state == HID_DEV_SWITCHING_TOME)
        {
            addr_list[offset].type = 0x02;  ///white list; //bit 0 must be zero;
            memcpy(addr_list[offset].val, hid_dev_array[index].addr.val, 6);
            offset ++;           
        }
    }
    if(offset > 0)
    {
        printf("dump white black list...\r\n");
        for(index = 0; index<offset; index++)
        {
            printf("%d, %02x, [%02x:%02x:%02x:%02x:%02x:%02x]\r\n", index, addr_list[index].type, 
                addr_list[index].val[0],addr_list[index].val[1],addr_list[index].val[2],addr_list[index].val[3],addr_list[index].val[4],addr_list[index].val[5]);
        }
        
        rc = ble_gap_wl_set(&addr_list[0], offset);
        TLS_BT_APPL_TRACE_DEBUG("Adding device list ret=%d\r\n", rc);
    }
}
int update_hid_dev_state(uint8_t disconnected, ble_addr_t *addr)
{
    int index = 0;
    int rc = 0;
    
    if(disconnected)
    {
        for(index = 0; index<MAX_BONDING_DEV_COUNT; index++)
        {
            if(hid_dev_array[index].state == HID_DEV_DISCONNECTING)
            {
                hid_dev_array[index].state = HID_DEV_BONDED_AND_DISCONNECTED;
                TLS_BT_APPL_TRACE_DEBUG("disconnecting, hid_dev_array index=%d, disconnected\r\n", index);
                break;
            }else if(hid_dev_array[index].state == HID_DEV_REPLACING)
            {
                hid_dev_array[index].state = HID_DEV_ADDING;
                TLS_BT_APPL_TRACE_DEBUG("replacing, hid_dev_array index=%d, next step adding\r\n", index);
                break;                
            }else if(hid_dev_array[index].state == HID_DEV_ADDING)
            {
                TLS_BT_APPL_TRACE_DEBUG("reset adding state to idle, hid_dev_array index=%d\r\n", index);
                hid_dev_array[index].state = HID_DEV_IDLE;
                break;
            }
        }
    }else
    {
        for(index = 0; index<MAX_BONDING_DEV_COUNT; index++)
        {
            if(hid_dev_array[index].state == HID_DEV_ADDING)
            {
                hid_dev_array[index].state = HID_DEV_BONDED_AND_CONNECTED;
                memcpy(&hid_dev_array[index].addr, addr, sizeof(ble_addr_t));
                TLS_BT_APPL_TRACE_DEBUG("adding, hid_dev_array index=%d, bonded and connected\r\n", index);
                break;
                
            }else if(hid_dev_array[index].state == HID_DEV_SWITCHING_TOME)
            {
                hid_dev_array[index].state = HID_DEV_BONDED_AND_CONNECTED;
                TLS_BT_APPL_TRACE_DEBUG("switching to,hid_dev_array index=%d, bonded and connected\r\n", index);

                rc = ble_addr_cmp(&hid_dev_array[index].addr, addr);

                assert(rc == 0);
                break;
            }
        }
    }

    if(index == MAX_BONDING_DEV_COUNT)
    {
        TLS_BT_APPL_TRACE_WARNING("No device need to be updated!!!\r\n")
    }else
    {
        TLS_BT_APPL_TRACE_DEBUG("device map flushing...!!!\r\n")
        update_dev_to_nvram(false);
    }

    return index;
    
}

int check_hid_dev_black_white_list(ble_addr_t *addr)
{
    bool found = false;
    uint8_t dev_index;
    int rc;
    
    /*check white list first*/
    found = check_hid_dev_array_switching(&dev_index);

    if(found)
    {
        rc = ble_addr_cmp(&hid_dev_array[dev_index].addr, addr);
        if(rc == 0)
        {
            TLS_BT_APPL_TRACE_DEBUG("Found white list, accept it...\r\n");
            return 0;
        }else
        {
            TLS_BT_APPL_TRACE_DEBUG("Found white list, but addr does not match, decline it now\r\n");
            tls_ble_server_demo_hid_disconnect(true);
            return 1;
        }
    }

    /*check black list now*/
    found = check_hid_dev_array_matching(addr);

    /*in black list, decline it and continue to adv*/
    if(found)
    {
        TLS_BT_APPL_TRACE_DEBUG("Found matching black list!!!\r\n");

        tls_ble_server_demo_hid_disconnect(true);
        
        return 1;
    }

    return -1;
}

int tls_ble_server_hid_hotkey_init()
{
    int rc;
    uint8_t nvram_value[MAX_BONDING_DEV_COUNT*9];
    
    memset(hid_dev_array, 0, sizeof(hid_dev_array));
    memset(nvram_value, 0, sizeof(nvram_value));
    //load hid dev information and restore to hid_dev_array...
    rc = btif_config_load_key_map(nvram_value, sizeof(nvram_value));
    if(rc == 0)
    {
        load_nvram_to_dev(nvram_value);
    }else
    {
        TLS_BT_APPL_TRACE_DEBUG("btif_config_load_key_map, no device info...\r\n");
    }
}
int tls_ble_server_hid_hotkey_deinit()
{
    
}

#endif
