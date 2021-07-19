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
#include "wm_pmu.h"
#include "wm_ble_server_hid_demo.h"
#include "wm_ble_server_hid_hotkey.h"

typedef struct uart_buffer
{
    uint8_t buffer[15];
    uint8_t flag;
} uart_buffer_t;


#define    HID_UART_TASK_STACK_SIZE     384
#define    HID_UART_TASK_PRIO	        33
#define    HID_UART_QUEUE_SIZE          10

#define    HID_UART_IDX                 TLS_UART_1

#define SYS_STANDBY_ENABLE 1

static tls_os_queue_t 	*hid_uart_msg_queue = NULL;
static void 			*hid_uart_task_stack_ptr;
static uart_buffer_t 	hid_uart_queue[HID_UART_QUEUE_SIZE];
static uint8_t          hid_uart_queue_idx = 0;
static uint8_t          hid_uart_service_init = 0;
static uint32_t         hid_uart_last_active_ticks = 0;
static uint8_t          hid_uart_indicate_pmu_stop = 0;
static tls_os_task_t    task_hid_uart_hdl = NULL;

#define                 HID_UART_INPUT_MSG_HEADER         3
#define                 HID_UART_INPUT_MSG_KEYBOARD       8
#define                 HID_UART_INPUT_MSG_MOUSE          4
#define                 HID_UART_INPUT_MSG_KEYBOARD_CC    7

static uint8_t          hid_uart_read_size = HID_UART_INPUT_MSG_HEADER;
static uint8_t          hid_request_control_header[4] =  {0x57, 0xAB, 0x82, 0xA3};
static uint8_t          hid_request_keyboard_header[4] = {0x57, 0xAB, 0x01, 0x0B};
static uint8_t          hid_request_mouse_header[4] =    {0x57, 0xAB, 0x02, 0x07};
//static uint8_t          hid_request_keyboard_cc_header[4] = {0x57, 0xAB, 0x88, 0x07};

static volatile uint8_t led_status = 0xFF;
static volatile uint32_t led_ctrl_ticks = 0;
static volatile bool    led_ctrl_pending = false;


#define UART_MSG_OUTPUT 1

/*Forward declaration*/
#if SYS_STANDBY_ENABLE
static void bt_pmu_enable();
#endif

static void write_led_status(void *arg);

static int hid_uart_rx_cb(u16 len, void* user_data)
{
    tls_os_queue_send(hid_uart_msg_queue, (void *)0x0E, 0); //send msg, to handle uart rx;
#if 0
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
#endif    
}

static int hid_uart_rx_handler()
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
            printf("RCV:");
            for(i= 0; i< hid_uart_read_size; i++) printf("%02x ", header[i]);
            printf("\r\n"); 
#endif
            if(memcmp(hid_request_control_header, header, HID_UART_INPUT_MSG_HEADER) == 0)
            {
                rc = tls_os_queue_send(hid_uart_msg_queue, (void *)0x0F, 0); //send msg, to stop request;
                if(rc != 0) assert(0);

                hid_uart_read_size = 1; //read out 1 byte default;
                drop_count = hid_uart_read_size;
                #if UART_MSG_OUTPUT            
                    printf("RCV:");
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
                
            }else if(memcmp(hid_request_keyboard_header, header, HID_UART_INPUT_MSG_HEADER) == 0)
            {
                hid_uart_read_size = HID_UART_INPUT_MSG_KEYBOARD;
            }else if(memcmp(hid_request_mouse_header, header, HID_UART_INPUT_MSG_HEADER) == 0)
            {
                hid_uart_read_size = HID_UART_INPUT_MSG_MOUSE;
            }else
            {
                hid_uart_read_size = 1; //read out 1 byte default;
                drop_count = hid_uart_read_size;
                #if UART_MSG_OUTPUT            
                    printf("RCV:");
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
            tls_uart_read(HID_UART_IDX,hid_uart_queue[hid_uart_queue_idx].buffer+1, hid_uart_read_size);
            if(payload_len == HID_UART_INPUT_MSG_KEYBOARD)
            {
                hid_uart_queue[hid_uart_queue_idx].buffer[0] = 0x01;
            }else if(payload_len == HID_UART_INPUT_MSG_MOUSE)
            {
                hid_uart_queue[hid_uart_queue_idx].buffer[0] = 0x02;
            }
#if UART_MSG_OUTPUT  
            printf("RCV:");
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
    //uint8_t src_type = (cmd[0]>>4)&0x03;
    uint8_t src_type = (cmd[0])&0x03;

#if UART_MSG_OUTPUT     
        printf("QUE:");
        for(i = 0; i< 11; i++) printf("%02x ", cmd[i]);
        printf("\r\n");
#endif

    if(src_type == 0x01)
    {
        len = 11;

        if(0x00 == cmd[1] && 0x00 == cmd[3] && 0x00 == cmd[4])
        {
            led_ctrl_ticks = tls_os_get_time();
            if(led_ctrl_pending)
            {
                TLS_BT_APPL_TRACE_ERROR("release blocking...\r\n");
                //tls_bt_async_proc_func(write_led_status,(void*)led_ctrl_ticks,50);
            }
        }
        
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
                rc = 0;
            }
        }
        rc = tls_ble_server_demo_hid_keyboard_notify(cmd+1, 8);
    }else if(src_type == 0x02)
    {
        len = 11;
        rc = tls_ble_server_demo_hid_mouse_notify(cmd+1, 4);        
    }else if(src_type == 0x03)
    {
        len = 7;
        //57 AB 88 07 30 03 40 00 00 10 53 
        //57 AB 88 07 30 03 00 00 00 11 14
        rc = tls_ble_server_demo_hid_keyboard_cc_notify(cmd+2, 2);
    }
    
    return 0;
}

static uint32_t ticks_elapsed(uint32_t ticks)
{
    uint32_t diff_ticks = 0;
    
    uint32_t curr_ticks = tls_os_get_time();

    if(curr_ticks > ticks)
    {
        diff_ticks = curr_ticks - ticks;
    }else
    {
        diff_ticks = curr_ticks + (0xFFFFFFFF - ticks);
    }

    return diff_ticks;
}

static void hid_uart_task(void *sdata)
{
    int rc, i;
    void *msg = NULL;
    tls_os_status_t status;
    uint8_t queue_idx = 0;
    uint8_t advertising_state, connected_state;
    tls_uart_port_init(HID_UART_IDX, NULL, 0);
    tls_uart_set_baud_rate(HID_UART_IDX, 115200);
	tls_uart_rx_callback_register(HID_UART_IDX, hid_uart_rx_cb, NULL);

    for(;;)
    {
        status = tls_os_queue_receive(hid_uart_msg_queue, (void **)&msg, 0, HZ);
        if(status != TLS_OS_SUCCESS)
        {
            rc = tls_ble_server_demo_hid_conn_param_update_low_duty_slave();
            if(!rc)TLS_BT_APPL_TRACE_DEBUG("### low duty ble_l2cap_sig_update rc=%d \r\n", rc);
#if SYS_STANDBY_ENABLE            
            advertising_state = tls_ble_server_demo_hid_advertising();
            connected_state = tls_ble_server_demo_hid_connected();
            /*idle state and last time(5000ticks = 10s), enable pmu mode*/
            if(connected_state == 0 && advertising_state == 0 && (ticks_elapsed(hid_uart_last_active_ticks)> 5000))
            {
                bt_pmu_enable();
            }
#endif            
            continue;
        }

        
        tls_bt_init(0xFF);  //only works:  1, system is sleeping, now wake up by keyboard input.
        
        queue_idx = (uint8_t)(uint32_t)msg;
        switch(queue_idx)
        {
            case 0x0E:
                TLS_BT_APPL_TRACE_DEBUG("!!!check pmu_stop:%d\r\n", hid_uart_indicate_pmu_stop);
                hid_uart_rx_handler();
                break;
            case 0x0F:
            {
                uint8_t req_stop_payload[11] = {0x57, 0xAB, 0x12, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x80, 0x00, 0x20};
                tls_uart_write_async(HID_UART_IDX,(char*)req_stop_payload,sizeof(req_stop_payload));
                #if UART_MSG_OUTPUT
                printf("SND:");
                for(i=0; i<11; i++) printf("%02x ", req_stop_payload[i]);
                printf("\r\n");
                #endif
                hid_uart_indicate_pmu_stop = 1;
                hid_uart_last_active_ticks = tls_os_get_time();  
                tls_ble_server_demo_hid_conn_param_update_high_duty_slave();
            }
                break;
            default:
                parse_cmd_execute(hid_uart_queue[queue_idx].buffer);
                hid_uart_indicate_pmu_stop = 1;
                hid_uart_last_active_ticks = tls_os_get_time();
                tls_ble_server_demo_hid_conn_param_update_high_duty_slave();
                break;
        }        
        
    }
}

static void write_led_status(void *arg)
{
    int i = 0, rc = -1;
    uint8_t status = 0; //(uint8_t)arg;
    uint32_t ctrl_ticks = (uint32_t)arg;
    uint32_t curr_ticks = tls_os_get_time();
    
    uint8_t req_led_payload[11] = {0x57, 0xAB, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xac, 0x20};

    req_led_payload[7] = led_status;
    //rc = tls_uart_write(TLS_UART_1,req_led_payload,sizeof(req_led_payload));
    

    TLS_BT_APPL_TRACE_ERROR("***LED SET STATUS trying arg=%d, act=%d\r\n", ctrl_ticks, led_ctrl_ticks);

    if(led_ctrl_ticks == ctrl_ticks)
    {
        TLS_BT_APPL_TRACE_ERROR("led ctrl...\r\n");
        led_ctrl_pending = false;
        rc = tls_uart_write(TLS_UART_1,req_led_payload,sizeof(req_led_payload));

#if UART_MSG_OUTPUT
        TLS_BT_APPL_TRACE_ERROR("!!!LED SET STATUS status=%d,ret=%d\r\n", status, rc);
        printf("SND:");
        for(i=0; i<11; i++) printf("%02x ", req_led_payload[i]);
        printf("\r\n");
#endif 
    }

   
}



void tls_ble_server_hid_uart_set_led_status(uint8_t status)
{
    int rc = -1;
    int i = 0;
    
    uint8_t req_led_payload[11] = {0x57, 0xAB, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xac, 0x20};

    if(led_status != status)
    {
        led_status = status;
        led_ctrl_ticks = tls_os_get_time();
        led_ctrl_pending = true;
    #if 1
        req_led_payload[7] = status;
        rc = tls_uart_write_async(TLS_UART_1,req_led_payload,sizeof(req_led_payload)); 
        #if UART_MSG_OUTPUT
        TLS_BT_APPL_TRACE_ERROR("!!!LED SET STATUS status=%d,ret=%d\r\n", status, rc);
        printf("SND:");
        for(i=0; i<11; i++) printf("%02x ", req_led_payload[i]);
        printf("\r\n");
        #endif
    #else
        TLS_BT_APPL_TRACE_ERROR("mark ticks for control\r\n");
        tls_bt_async_proc_func(write_led_status,(void*)led_ctrl_ticks,100);
    #endif
        
    }
   
}

void tls_ble_server_hid_uart_sync_led_status(uint8_t status)
{
#define NUM_LOCK   0x01
#define CAPS_LOCK  0x02
#define SCOLL_LOCK 0x04
    uint8_t notify_buffer[8] = {0x00, 0x00, 0x53, 0x00, 0x00, 0x00, 0x00, 0x00};

    if(status & NUM_LOCK)
    {
        notify_buffer[2] = 0x53;
        tls_ble_server_demo_hid_keyboard_notify(notify_buffer, 8);
    }

    if(status & CAPS_LOCK)
    {
        notify_buffer[2] = 0x39;
        tls_ble_server_demo_hid_keyboard_notify(notify_buffer, 8);        
    }

    if(status & SCOLL_LOCK)
    {
        notify_buffer[2] = 0x47;
        tls_ble_server_demo_hid_keyboard_notify(notify_buffer, 8);       
    }     
}
#if SYS_STANDBY_ENABLE
static void bt_pmu_timer1_irq(uint8_t *arg)
{
    tls_pmu_timer0_stop();

    //check condition to enable standby

    //everything is quiet, enable
    //if()
    //{
    //     tls_pmu_standby_start()
    //}else
    //{
    //   tls_pmu_standby_stop()
    //}
    //tls_pmu_timer1_start(500);
    //
    TLS_BT_APPL_TRACE_DEBUG("system wake up...\r\n");

    //rc = tls_os_queue_send(hid_uart_msg_queue, (void *)0x0E, 0); //send msg, to enable bt;
    //if(rc != 0) assert(0);
    //tls_pmu_clk_select(1);
}

static void bt_pmu_enable()
{
    tls_bt_deinit();
    TLS_BT_APPL_TRACE_DEBUG("system sleep...\r\n");
    tls_pmu_timer1_isr_register((tls_pmu_irq_callback)bt_pmu_timer1_irq, NULL);
    tls_pmu_timer1_start(900);
    tls_pmu_sleep_start();
}
#endif


int tls_ble_server_hid_uart_init()
{
    tls_os_status_t status;
    
    TLS_BT_APPL_TRACE_DEBUG("tls_ble_server_hid_uart_init \r\n");
    if(hid_uart_service_init)
    {
    	TLS_BT_APPL_TRACE_WARNING("tls_ble_server_hid_uart_init registered\r\n");
        return BLE_HS_EALREADY;
    }
    
    status = tls_os_queue_create(&hid_uart_msg_queue, HID_UART_QUEUE_SIZE);
    assert(status == TLS_OS_SUCCESS);

#if TLS_OS_LITEOS
    status = tls_os_task_create(&task_hid_uart_hdl, "HIDU",
                       hid_uart_task,
                       NULL,
                       (void *)NULL,            /* LITE OS alloc the task stack by himself*/
                       HID_UART_TASK_STACK_SIZE * sizeof(uint32_t), /* task's stack size, unit:byte */
                       HID_UART_TASK_PRIO,
                       0);

    assert(status == TLS_OS_SUCCESS);

#else
    hid_uart_task_stack_ptr = (void*)tls_mem_alloc(HID_UART_TASK_STACK_SIZE * sizeof(u32));
    assert(hid_uart_task_stack_ptr != NULL);
    
    status = tls_os_task_create(&task_hid_uart_hdl, "HIDU",
                       hid_uart_task,
                       NULL,
                       (void *)hid_uart_task_stack_ptr,          /* task's stack start address */
                       HID_UART_TASK_STACK_SIZE * sizeof(uint32_t), /* task's stack size, unit:byte */
                       HID_UART_TASK_PRIO,
                       0);
    
    assert(status == TLS_OS_SUCCESS);
#endif    

    hid_uart_service_init = 1;
    hid_uart_queue_idx = 0;
    hid_uart_read_size = HID_UART_INPUT_MSG_HEADER;
    
    TLS_BT_APPL_TRACE_DEBUG("tls_ble_server_hid_uart_init registered\r\n");
    return status;
}


static void free_hid_uart_task_stack()
{
#if TLS_OS_LITEOS
#else
	if(hid_uart_task_stack_ptr)
	{
		tls_mem_free(hid_uart_task_stack_ptr);
		hid_uart_task_stack_ptr = NULL;
	}
#endif    
}

int tls_ble_server_hid_uart_deinit()
{
    int rc;
    
    if(hid_uart_service_init == 0) return BLE_HS_EALREADY;

    if(task_hid_uart_hdl)
    {
        tls_os_task_del_by_task_handle(task_hid_uart_hdl, free_hid_uart_task_stack);
    }

    if(hid_uart_msg_queue)
    {
        tls_os_queue_delete(hid_uart_msg_queue);
        hid_uart_msg_queue = NULL;
    }    

    rc = tls_ble_server_demo_hid_deinit();

    hid_uart_service_init = 0;

    TLS_BT_APPL_TRACE_DEBUG("tls_ble_server_hid_uart_init degistered\r\n");

    return rc;
}

#endif
