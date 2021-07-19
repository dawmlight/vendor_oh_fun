/*****************************************************************************
**
**  Name:           wm_uart_ble_if.c
**
**  Description:    This file contains the  implemention of interface between the hid service and uart
**
*****************************************************************************/
#include <assert.h>

#include "wm_bt_config.h"

#if (WM_NIMBLE_INCLUDED == CFG_ON)

#include "host/ble_hs.h"
#include "wm_bt_app.h"
#include "wm_bt_util.h"
#include "wm_mem.h"
#include "wm_osal.h"
#include "wm_uart.h"
#include "wm_ble_server_hid_demo.h"
#include "wm_ble_server_hid_hotkey.h"

typedef struct uart_buffer
{
    uint8_t buffer[15];
    uint8_t flag;
} uart_buffer_t;


#define    HID_UART_TASK_STACK_SIZE     384
#define    HID_UART_TASK_PRIO	        33
#define    HID_UART_QUEUE_SIZE          64

#define    HID_UART_IDX                 TLS_UART_1

static tls_os_queue_t 	*hid_uart_msg_queue = NULL;
static void 			*hid_uart_task_stack_ptr;
static uart_buffer_t 	hid_uart_queue[HID_UART_QUEUE_SIZE];
static uint8_t          hid_uart_queue_idx = 0;
static uint8_t          hid_uart_service_init = 0;

#define                 HID_UART_INPUT_MSG_HEADER         4
#define                 HID_UART_INPUT_MSG_KEYBOARD       11
#define                 HID_UART_INPUT_MSG_MOUSE          7
#define                 HID_UART_INPUT_MSG_KEYBOARD_CC    7

static uint8_t          hid_uart_read_size = HID_UART_INPUT_MSG_HEADER;
static uint8_t          hid_request_control_header[4] =  {0x57, 0xAB, 0x82, 0xA3};
static uint8_t          hid_request_keyboard_header[4] = {0x57, 0xAB, 0x88, 0x0B};
static uint8_t          hid_request_mouse_header[4] =    {0x57, 0xAB, 0x88, 0x07};
//static uint8_t          hid_request_keyboard_cc_header[4] = {0x57, 0xAB, 0x88, 0x07};


#define UART_MSG_OUTPUT 0

static int hid_uart_rx_cb(u16 len, void* user_data)
{
    int i = 0;
    int rc;
    uint8_t *ptr_detect = NULL;
    uint8_t drop_byte = 0, drop_count = 0;
    uint8_t header[HID_UART_INPUT_MSG_HEADER];
    
    int16_t payload_len = tls_uart_try_read(HID_UART_IDX,hid_uart_read_size);

    if(payload_len)
    {
        if(hid_uart_read_size <= HID_UART_INPUT_MSG_HEADER)
        {
            tls_uart_read(HID_UART_IDX, header, hid_uart_read_size);
#if UART_MSG_OUTPUT            
            printf("RAW:");
            for(i= 0; i< hid_uart_read_size; i++) printf("%02x ", header[i]);
            printf("\r\n"); 
#endif
            if(memcmp(hid_request_control_header, header, 4) == 0)
            {
                rc = tls_os_queue_send(hid_uart_msg_queue, (void *)0x0F, 0); //send msg, to stop request;
                if(rc != 0) assert(0);

                hid_uart_read_size = 1; //read out 1 byte default;
                drop_count = hid_uart_read_size;
                #if UART_MSG_OUTPUT            
                    printf("RAW:");
                #endif
                while(drop_count){
                    drop_count = tls_uart_read(HID_UART_IDX,&drop_byte, hid_uart_read_size);   //TODO drop all payload, not a good way;
                    #if UART_MSG_OUTPUT
                    printf("%02x ", drop_byte);
                    #endif
                }
                #if UART_MSG_OUTPUT            
                    printf("\r\n");
                #endif
                hid_uart_read_size = HID_UART_INPUT_MSG_HEADER;
                return 0;
                
            }else if(memcmp(hid_request_keyboard_header, header, 4) == 0)
            {
                hid_uart_read_size = HID_UART_INPUT_MSG_KEYBOARD;
            }else if(memcmp(hid_request_mouse_header, header, 4) == 0)
            {
                hid_uart_read_size = HID_UART_INPUT_MSG_MOUSE;
            }else
            {
                hid_uart_read_size = 1; //read out 1 byte default;
                drop_count = hid_uart_read_size;
                #if UART_MSG_OUTPUT            
                    printf("RAW:");
                #endif
                while(drop_count){
                    drop_count = tls_uart_read(HID_UART_IDX,&drop_byte, hid_uart_read_size);   //TODO drop all payload, not a good way;
                    #if UART_MSG_OUTPUT
                    printf("%02x ", drop_byte);
                    #endif
                }
                #if UART_MSG_OUTPUT            
                    printf("\r\n");
                #endif
                hid_uart_read_size = HID_UART_INPUT_MSG_HEADER;
                return 0;
            }
        }

        payload_len = tls_uart_try_read(HID_UART_IDX,hid_uart_read_size);
        
        if(payload_len == HID_UART_INPUT_MSG_MOUSE || payload_len == HID_UART_INPUT_MSG_KEYBOARD)
        {
            tls_uart_read(HID_UART_IDX,hid_uart_queue[hid_uart_queue_idx].buffer, hid_uart_read_size);
#if UART_MSG_OUTPUT  
            printf("PLD:");
            for(i= 0; i< hid_uart_read_size; i++) printf("%02x ", hid_uart_queue[hid_uart_queue_idx].buffer[i]);
            printf("\r\n");
#endif            
            rc = tls_os_queue_send(hid_uart_msg_queue, (void *)hid_uart_queue_idx, 0);
            if(rc != 0)
            {
                TLS_BT_APPL_TRACE_WARNING("hid_uart_msg_queue full\r\n");
            }

            hid_uart_queue_idx ++;
            if(hid_uart_queue_idx == HID_UART_QUEUE_SIZE)
            {
                hid_uart_queue_idx = 0;
            }
            hid_uart_read_size = HID_UART_INPUT_MSG_HEADER;
        }
    }    

    return 0;
}

int parse_cmd_execute(uint8_t *cmd)
{
    int i = 0, rc = 0;
    uint8_t hotkey_index = 0;
    int len = 0;
    //per spec:
    uint8_t src_type = (cmd[0]>>4)&0x03;

#if UART_MSG_OUTPUT     
        printf("QUE:");
        for(i = 0; i< 11; i++) printf("%02x ", cmd[i]);
        printf("\r\n");
#endif

    if(src_type == 0x01)
    {
        len = 11;
        if(0x04 == cmd[1])   //LEFT ALT pressed
        {
            rc = check_hotkey_pressed(cmd+1, 8);

            if(rc){
                tls_ble_server_demo_hid_keyboard_notify(cmd+1, 8);
                return 1;
            }
        }

        {
            rc = check_hotkey_released(&hotkey_index);
            if(rc)
            {
                process_hotkey(rc, hotkey_index);
            }
        }
        tls_ble_server_demo_hid_keyboard_notify(cmd+1, 8);
    }else if(src_type == 0x02)
    {
        len = 11;
        tls_ble_server_demo_hid_mouse_notify(cmd+1, 4);        
    }else if(src_type == 0x03)
    {
        len = 7;
        //57 AB 88 07 30 03 40 00 00 10 53 
        //57 AB 88 07 30 03 00 00 00 11 14
        tls_ble_server_demo_hid_keyboard_cc_notify(cmd+2, 2);
    }
    

    return 0;
}


static void hid_uart_task(void *sdata)
{
    void *msg;
    tls_os_status_t status;
    uint8_t queue_idx = 0;
    ts_uart_init(HID_UART_IDX);
    tls_uart_set_baud_rate(HID_UART_IDX, 115200);
	tls_uart_rx_callback_register(HID_UART_IDX, hid_uart_rx_cb, NULL);

    for(;;)
    {
        status = tls_os_queue_receive(hid_uart_msg_queue, (void **)&msg, 0, 0);
        assert(status == TLS_OS_SUCCESS);
        
        queue_idx = (uint8_t)msg;

        if(queue_idx == 0x0F)
        {
            uint8_t req_stop_payload[11] = {0x57, 0xAB, 0x12, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x80, 0x00, 0x20};
            tls_uart_write_async(HID_UART_IDX,req_stop_payload,sizeof(req_stop_payload));
        }else
        {
            parse_cmd_execute(hid_uart_queue[queue_idx].buffer);
        }
    }
}

int tls_ble_server_hid_uart_init()
{
    tls_os_status_t status;
    int rc;
    
    TLS_BT_APPL_TRACE_DEBUG("tls_ble_server_hid_uart_init \r\n");
    if(hid_uart_service_init)
    {
    	TLS_BT_APPL_TRACE_WARNING("tls_ble_server_hid_uart_init registered\r\n");
        return BLE_HS_EALREADY;
    }

    rc = tls_ble_server_demo_hid_init();
    assert(rc == 0);
    
    status = tls_os_queue_create(&hid_uart_msg_queue, HID_UART_QUEUE_SIZE);
    assert(status == TLS_OS_SUCCESS);
    
    hid_uart_task_stack_ptr = (void*)tls_mem_alloc(HID_UART_TASK_STACK_SIZE * sizeof(u32));
    assert(hid_uart_task_stack_ptr != NULL);
    
    status = tls_os_task_create(NULL, "HIDU",
                       hid_uart_task,
                       NULL,
                       (void *)hid_uart_task_stack_ptr,          /* task's stack start address */
                       HID_UART_TASK_STACK_SIZE * sizeof(uint32_t), /* task's stack size, unit:byte */
                       HID_UART_TASK_PRIO,
                       0);
    
    assert(status == TLS_OS_SUCCESS);

    hid_uart_service_init = 1;
    hid_uart_queue_idx = 0;
    hid_uart_read_size = HID_UART_INPUT_MSG_HEADER;
    
    TLS_BT_APPL_TRACE_DEBUG("tls_ble_server_hid_uart_init registered\r\n");

    tls_ble_server_hid_hotkey_init();

    return status;
}


static void free_hid_uart_task_stack()
{
	if(hid_uart_task_stack_ptr)
	{
		tls_mem_free(hid_uart_task_stack_ptr);
		hid_uart_task_stack_ptr = NULL;
	}
}

int tls_ble_server_hid_uart_deinit()
{
    int rc;
    
    if(hid_uart_service_init == 0) return BLE_HS_EALREADY;

    tls_os_task_del(HID_UART_TASK_PRIO, free_hid_uart_task_stack);

    if(hid_uart_msg_queue)
    {
        tls_os_queue_delete(hid_uart_msg_queue);
        hid_uart_msg_queue = NULL;
    }    

    rc = tls_ble_server_demo_hid_deinit();
    //assert(rc == 0);

    hid_uart_service_init = 0;

    TLS_BT_APPL_TRACE_DEBUG("tls_ble_server_hid_uart_init degistered\r\n");

    return 0;
}

#endif
