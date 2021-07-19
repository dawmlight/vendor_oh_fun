#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "wm_bt_config.h"

#if (WM_NIMBLE_INCLUDED == CFG_ON)

#include "wm_bt_app.h"
#include "wm_bt_util.h"

#include "host/ble_hs.h"
#include "wm_ble_gap.h"
#include "wm_osal.h"
#include "wm_uart.h"
#include "wm_ble_server_hid_codes.h"
#include "wm_ble_server_hid_demo.h"
#include "services/bas/ble_svc_bas.h"
#include "services/dis/ble_svc_dis.h"


/** GATT server. */
#define GATT_SVR_SVC_ALERT_UUID                 0x1811
#define GATT_SVR_CHR_SUP_NEW_ALERT_CAT_UUID     0x2A47
#define GATT_SVR_CHR_NEW_ALERT                  0x2A46
#define GATT_SVR_CHR_SUP_UNR_ALERT_CAT_UUID     0x2A48
#define GATT_SVR_CHR_UNR_ALERT_STAT_UUID        0x2A45
#define GATT_SVR_CHR_ALERT_NOT_CTRL_PT          0x2A44

#define GATT_UUID_HID_SERVICE                   0x1812

#define GATT_UUID_HID_INFORMATION               0x2A4A
#define GATT_UUID_HID_REPORT_MAP                0x2A4B
#define GATT_UUID_HID_CONTROL_POINT             0x2A4C
#define GATT_UUID_HID_REPORT                    0x2A4D
#define GATT_UUID_HID_PROTO_MODE                0x2A4E
#define GATT_UUID_HID_BT_KB_INPUT               0x2A22
#define GATT_UUID_HID_BT_KB_OUTPUT              0x2A32
#define GATT_UUID_HID_BT_MOUSE_INPUT            0x2A33

#define GATT_UUID_BAT_PRESENT_DESCR             0x2904
#define GATT_UUID_EXT_RPT_REF_DESCR             0x2907
#define GATT_UUID_RPT_REF_DESCR                 0x2908

#define BLE_SVC_DIS_MODEL_NUMBER_DEFAULT        "0x0102"
#define BLE_SVC_DIS_SERIAL_NUMBER_DEFAULT       "0x0001"
#define BLE_SVC_DIS_FIRMWARE_REVISION_DEFAULT   "0x1409"
#define BLE_SVC_DIS_HARDWARE_REVISION_DEFAULT   "0x0001"
#define BLE_SVC_DIS_SOFTWARE_REVISION_DEFAULT   "0x1409"
#define BLE_SVC_DIS_MANUFACTURER_NAME_DEFAULT   "winnermicro"
#define BLE_SVC_DIS_SYSTEM_ID_DEFAULT           "W800"
#define BLE_SVC_DIS_PNP_INFO_DEFAULT            {0x00,0x47,0x00,0xff,0xff,0xff,0xff}

#define BLE_SVC_DIS_UUID16					        0x180A
#define BLE_SVC_DIS_CHR_UUID16_SYSTEM_ID			0x2A23
#define BLE_SVC_DIS_CHR_UUID16_MODEL_NUMBER			0x2A24
#define BLE_SVC_DIS_CHR_UUID16_SERIAL_NUMBER		0x2A25
#define BLE_SVC_DIS_CHR_UUID16_FIRMWARE_REVISION 	0x2A26
#define BLE_SVC_DIS_CHR_UUID16_HARDWARE_REVISION 	0x2A27
#define BLE_SVC_DIS_CHR_UUID16_SOFTWARE_REVISION 	0x2A28
#define BLE_SVC_DIS_CHR_UUID16_MANUFACTURER_NAME    0x2A29
#define BLE_SVC_DIS_CHR_UUID16_PNP_INFO             0x2A50

/*
    Defines default permissions for reading characteristics. Can be
    zero to allow read without extra permissions or combination of:
        BLE_GATT_CHR_F_READ_ENC
        BLE_GATT_CHR_F_READ_AUTHEN
        BLE_GATT_CHR_F_READ_AUTHOR
*/

#define BLE_SVC_DIS_MODEL_NUMBER_READ_PERM      0
#define BLE_SVC_DIS_SERIAL_NUMBER_READ_PERM     0
#define BLE_SVC_DIS_HARDWARE_REVISION_READ_PERM 0
#define BLE_SVC_DIS_FIRMWARE_REVISION_READ_PERM 0
#define BLE_SVC_DIS_SOFTWARE_REVISION_READ_PERM 0
#define BLE_SVC_DIS_MANUFACTURER_NAME_READ_PERM 0
#define BLE_SVC_DIS_SYSTEM_ID_READ_PERM         0

/* Attribute value lengths */
#define HID_PROTOCOL_MODE_LEN           1         // HID Protocol Mode
#define HID_INFORMATION_LEN             4         // HID Information
#define HID_REPORT_REF_LEN              2         // HID Report Reference Descriptor
#define HID_EXT_REPORT_REF_LEN          2         // External Report Reference Descriptor

// HID Report IDs for the service
#define HID_RPT_ID_KB_OUT               0   // LED output report ID from report map
#define HID_RPT_ID_MOUSE_IN             1   // Mouse input report ID from report map
#define HID_RPT_ID_KB_IN                2   // Keyboard input report ID from report map
#define HID_RPT_ID_CC_IN                3   // Consumer Control input report ID from report map
#define HID_RPT_ID_FEATURE              4   // Feature report ID from report map

// boot report cb_access args
#define HID_BOOT_KB_IN                  6   // Keyboard input report ID
#define HID_BOOT_KB_OUT                 7   // LED output report ID
#define HID_BOOT_MOUSE_IN               8   // Mouse input report ID

// HID Report types
#define HID_REPORT_TYPE_INPUT           1
#define HID_REPORT_TYPE_OUTPUT          2
#define HID_REPORT_TYPE_FEATURE         3

#define HID_CC_RPT_MUTE                 1
#define HID_CC_RPT_POWER                2
#define HID_CC_RPT_LAST                 3
#define HID_CC_RPT_ASSIGN_SEL           4
#define HID_CC_RPT_PLAY                 5
#define HID_CC_RPT_PAUSE                6
#define HID_CC_RPT_RECORD               7
#define HID_CC_RPT_FAST_FWD             8
#define HID_CC_RPT_REWIND               9
#define HID_CC_RPT_SCAN_NEXT_TRK        10
#define HID_CC_RPT_SCAN_PREV_TRK        11
#define HID_CC_RPT_STOP                 12

#define HID_CC_RPT_CHANNEL_UP           0x01
#define HID_CC_RPT_CHANNEL_DOWN         0x03
#define HID_CC_RPT_VOLUME_UP            0x40
#define HID_CC_RPT_VOLUME_DOWN          0x80

// HID Consumer Control report bitmasks
#define HID_CC_RPT_NUMERIC_BITS         0xF0
#define HID_CC_RPT_CHANNEL_BITS         0xCF
#define HID_CC_RPT_VOLUME_BITS          0x3F
#define HID_CC_RPT_BUTTON_BITS          0xF0
#define HID_CC_RPT_SELECTION_BITS       0xCF


// Macros for the HID Consumer Control 2-byte report
#define HID_CC_RPT_SET_NUMERIC(s, x)    (s)[0] &= HID_CC_RPT_NUMERIC_BITS;   \
                                        (s)[0] |= (x)
#define HID_CC_RPT_SET_CHANNEL(s, x)    (s)[0] &= HID_CC_RPT_CHANNEL_BITS;   \
                                        (s)[0] |= ((x) & 0x03) << 4
#define HID_CC_RPT_SET_VOLUME(s, x)     (s)[0] &= HID_CC_RPT_VOLUME_BITS;    \
                                        (s)[0] |= (x)
#define HID_CC_RPT_SET_BUTTON(s, x)     (s)[1] &= HID_CC_RPT_BUTTON_BITS;    \
                                        (s)[1] |= (x)
#define HID_CC_RPT_SET_SELECTION(s, x)  (s)[1] &= HID_CC_RPT_SELECTION_BITS; \
                                        (s)[1] |= ((x) & 0x03) << 4

// Keyboard report size
#define HIDD_LE_REPORT_KB_IN_SIZE       (8)

// Mouse report size
#define HIDD_LE_REPORT_MOUSE_SIZE       (4)

// LEDS report size
#define HIDD_LE_REPORT_KB_OUT_SIZE      (1)

// Consumer control report size
#define HIDD_LE_REPORT_CC_SIZE          (2)

// battery level data size
#define HIDD_LE_BATTERY_LEVEL_SIZE      (1)

// feature data size
#define HIDD_LE_REPORT_FEATURE          (6)

/* HID information flags */
#define HID_FLAGS_REMOTE_WAKE           0x01      // RemoteWake
#define HID_FLAGS_NORMALLY_CONNECTABLE  0x02      // NormallyConnectable

/* Control point commands */
#define HID_CMD_SUSPEND                 0x00      // Suspend
#define HID_CMD_EXIT_SUSPEND            0x01      // Exit Suspend

/* HID protocol mode values */
#define HID_PROTOCOL_MODE_BOOT          0x00      // Boot Protocol Mode
#define HID_PROTOCOL_MODE_REPORT        0x01      // Report Protocol Mode

// HID feature flags
//#define HID_KBD_FLAGS                   HID_FLAGS_NORMALLY_CONNECTABLE
#define HID_KBD_FLAGS                   HID_FLAGS_REMOTE_WAKE

#define HI_UINT16(a) (((a) >> 8) & 0xFF)
#define LO_UINT16(a) ((a) & 0xFF)

#define DEFAULT_MIN_KEY_SIZE            0

#define NVS_IO_CAP_NUM "io_cap_number"
#define DEFAULT_IO_CAP BLE_HS_IO_NO_INPUT_OUTPUT

#define HID_KEYBOARD_APPEARENCE         0x03c1 // appearance field in advertising packet

enum attr_handles {
    HANDLE_BATTERY_LEVEL,           //  0
    HANDLE_DIS_MODEL_NUMBER,            //  1
    HANDLE_DIS_SERIAL_NUMBER,           //  2
    HANDLE_DIS_HARDWARE_REVISION,       //  3
    HANDLE_DIS_FIRMWARE_REVISION,       //  4
    HANDLE_DIS_SOFWARE_REVISION,        //  5
    HANDLE_DIS_MANUFACTURER_NAME,       //  6
    HANDLE_DIS_SYSTEM_ID,               //  7
    HANDLE_DIS_PNP_INFO,                //  8

    // HID SERVICE
    HANDLE_HID_INFORMATION,             //  9
    HANDLE_HID_CONTROL_POINT,           // 10
    HANDLE_HID_REPORT_MAP,              // 11
    HANDLE_HID_PROTO_MODE,              // 12
    HANDLE_HID_MOUSE_REPORT,            // 13
    HANDLE_HID_KB_IN_REPORT,            // 14
    HANDLE_HID_KB_OUT_REPORT,           // 15
    HANDLE_HID_CC_REPORT,               // 16
    HANDLE_HID_BOOT_KB_IN_REPORT,       // 17
    HANDLE_HID_BOOT_KB_OUT_REPORT,      // 18
    HANDLE_HID_BOOT_MOUSE_REPORT,       // 19
    HANDLE_HID_FEATURE_REPORT,          // 20
    HANDLE_HID_COUNT                    // 21
};
// Battery level presentation descriptor value format
struct prf_char_pres_fmt
{
    /// Format
    uint8_t format;
    /// Exponent
    uint8_t exponent;
    /// Unit (The Unit is a UUID)
    uint16_t unit;
    /// Name space
    uint8_t name_space;
    /// Description
    uint16_t description;
} __attribute__((packed));

struct report_reference_table {
    int id;
    uint8_t hidReportRef[HID_REPORT_REF_LEN];
} __attribute__((packed));


#define SUPPORT_REPORT_VENDOR false

// HID Report Map characteristic value
// Keyboard report descriptor (using format for Boot interface descriptor)
const uint8_t Hid_report_map[] = {
    /*** MOUSE REPORT ***/
    0x05, 0x01,  // Usage Page (Generic Desktop)
    0x09, 0x02,  // Usage (Mouse)
    0xA1, 0x01,  // Collection (Application)
    0x85, 0x01,  // Report Id (1)
    0x09, 0x01,  //   Usage (Pointer)
    0xA1, 0x00,  //   Collection (Physical)
    0x05, 0x09,  //     Usage Page (Buttons)
    0x19, 0x01,  //     Usage Minimum (01) - Button 1
    0x29, 0x03,  //     Usage Maximum (03) - Button 3
    0x15, 0x00,  //     Logical Minimum (0)
    0x25, 0x01,  //     Logical Maximum (1)
    0x75, 0x01,  //     Report Size (1)
    0x95, 0x03,  //     Report Count (3)
    0x81, 0x02,  //     Input (Data, Variable, Absolute) - Button states
    0x75, 0x05,  //     Report Size (5)
    0x95, 0x01,  //     Report Count (1)
    0x81, 0x01,  //     Input (Constant) - Padding or Reserved bits
    0x05, 0x01,  //     Usage Page (Generic Desktop)
    0x09, 0x30,  //     Usage (X)
    0x09, 0x31,  //     Usage (Y)
    0x09, 0x38,  //     Usage (Wheel)
    0x15, 0x81,  //     Logical Minimum (-127)
    0x25, 0x7F,  //     Logical Maximum (127)
    0x75, 0x08,  //     Report Size (8)
    0x95, 0x03,  //     Report Count (3)
    0x81, 0x06,  //     Input (Data, Variable, Relative) - X coordinate, Y coordinate, wheel
    0xC0,        //   End Collection
    0xC0,        // End Collection

    /*** KEYBOARD REPORT ***/
    0x05, 0x01,  // Usage Pg (Generic Desktop)
    0x09, 0x06,  // Usage (Keyboard)
    0xA1, 0x01,  // Collection: (Application)
    0x85, 0x02,  // Report Id (2)
    //
    0x05, 0x07,  //   Usage Pg (Key Codes)
    0x19, 0xE0,  //   Usage Min (224)
    0x29, 0xE7,  //   Usage Max (231)
    0x15, 0x00,  //   Log Min (0)
    0x25, 0x01,  //   Log Max (1)
    //
    //   Modifier byte
    0x75, 0x01,  //   Report Size (1)
    0x95, 0x08,  //   Report Count (8)
    0x81, 0x02,  //   Input: (Data, Variable, Absolute)
    //
    //   Reserved byte
    0x95, 0x01,  //   Report Count (1)
    0x75, 0x08,  //   Report Size (8)
    0x81, 0x01,  //   Input: (Constant)
    //
    //   LED report
    0x95, 0x05,  //   Report Count (5)
    0x75, 0x01,  //   Report Size (1)
    0x05, 0x08,  //   Usage Pg (LEDs)
    0x19, 0x01,  //   Usage Min (1)
    0x29, 0x05,  //   Usage Max (5)
    0x91, 0x02,  //   Output: (Data, Variable, Absolute)
    //
    //   LED report padding
    0x95, 0x01,  //   Report Count (1)
    0x75, 0x03,  //   Report Size (3)
    0x91, 0x01,  //   Output: (Constant)
    //
    //   Key arrays (6 bytes)
    0x95, 0x06,  //   Report Count (6)
    0x75, 0x08,  //   Report Size (8)
    0x15, 0x00,  //   Log Min (0)
    0x25, 0x65,  //   Log Max (101)
    0x05, 0x07,  //   Usage Pg (Key Codes)
    0x19, 0x00,  //   Usage Min (0)
    0x29, 0x65,  //   Usage Max (101)
    0x81, 0x00,  //   Input: (Data, Array)
    //
    0xC0,        // End Collection
    //
    /*** CONSUMER DEVICE REPORT ***/
    0x05, 0x0C,   // Usage Pg (Consumer Devices)
    0x09, 0x01,   // Usage (Consumer Control)
    0xA1, 0x01,   // Collection (Application)
    0x85, 0x03,   // Report Id (3)
    0x09, 0x02,   //   Usage (Numeric Key Pad)
    0xA1, 0x02,   //   Collection (Logical)
    0x05, 0x09,   //     Usage Pg (Button)
    0x19, 0x01,   //     Usage Min (Button 1)
    0x29, 0x0A,   //     Usage Max (Button 10)
    0x15, 0x01,   //     Logical Min (1)
    0x25, 0x0A,   //     Logical Max (10)
    0x75, 0x04,   //     Report Size (4)
    0x95, 0x01,   //     Report Count (1)
    0x81, 0x00,   //     Input (Data, Ary, Abs)
    0xC0,         //   End Collection
    0x05, 0x0C,   //   Usage Pg (Consumer Devices)
    0x09, 0x86,   //   Usage (Channel)
    0x15, 0xFF,   //   Logical Min (-1)
    0x25, 0x01,   //   Logical Max (1)
    0x75, 0x02,   //   Report Size (2)
    0x95, 0x01,   //   Report Count (1)
    0x81, 0x46,   //   Input (Data, Var, Rel, Null)
    0x09, 0xE9,   //   Usage (Volume Up)
    0x09, 0xEA,   //   Usage (Volume Down)
    0x15, 0x00,   //   Logical Min (0)
    0x75, 0x01,   //   Report Size (1)
    0x95, 0x02,   //   Report Count (2)
    0x81, 0x02,   //   Input (Data, Var, Abs)
    0x09, 0xE2,   //   Usage (Mute)
    0x09, 0x30,   //   Usage (Power)
    0x09, 0x83,   //   Usage (Recall Last)
    0x09, 0x81,   //   Usage (Assign Selection)
    0x09, 0xB0,   //   Usage (Play)
    0x09, 0xB1,   //   Usage (Pause)
    0x09, 0xB2,   //   Usage (Record)
    0x09, 0xB3,   //   Usage (Fast Forward)
    0x09, 0xB4,   //   Usage (Rewind)
    0x09, 0xB5,   //   Usage (Scan Next)
    0x09, 0xB6,   //   Usage (Scan Prev)
    0x09, 0xB7,   //   Usage (Stop)
    0x15, 0x01,   //   Logical Min (1)
    0x25, 0x0C,   //   Logical Max (12)
    0x75, 0x04,   //   Report Size (4)
    0x95, 0x01,   //   Report Count (1)
    0x81, 0x00,   //   Input (Data, Ary, Abs)
    0x09, 0x80,   //   Usage (Selection)
    0xA1, 0x02,   //   Collection (Logical)
    0x05, 0x09,   //     Usage Pg (Button)
    0x19, 0x01,   //     Usage Min (Button 1)
    0x29, 0x03,   //     Usage Max (Button 3)
    0x15, 0x01,   //     Logical Min (1)
    0x25, 0x03,   //     Logical Max (3)
    0x75, 0x02,   //     Report Size (2)
    0x81, 0x00,   //     Input (Data, Ary, Abs)
    0xC0,         //   End Collection
    0x81, 0x03,   //   Input (Const, Var, Abs)
    0xC0,         // End Collection
};

size_t Hid_report_map_size = sizeof(Hid_report_map);



/*
10 ms is enough time for writing operation, and
a very large amount of time im terms of ble operations*/
#define HID_DEV_BUF_MUTEX_WAIT 10
#define BATTERY_DEFAULT_LEVEL 77

/* notify data buffers */
static uint8_t
    /* mouse: byte 0: bit 0 Button 1, bit 1 Button 2, bit 2 Button 3, bits 4 to 7 zero
    byte 1 : X displacement, byte 2: Y displacement, byte 3: wheel    */
    Mouse_buffer[HIDD_LE_REPORT_MOUSE_SIZE],
    /* keyboard
    byte 0: modifiers: bit 0 LEFT CTRL, 1 LEFT SHIFT, 2 LEFT ALT, 3 LEFT GUI
                           4 RIGHT CTRL, 5 RIGHT SHIFT, 6 RIGHT ALT, 7 RIGHT GUI
    byte 1: reserved (zeroes),
    bytes 2 to 7: keyboard scan codes from 4 to 221     */
    Keyboard_buffer[HIDD_LE_REPORT_KB_IN_SIZE],
    /* consumer control buffer
    byte 0: bits 0-3 num key pad, 4-5 - channel (+1 -1), 6 - volume up, 7 - volume down
    byte 1: 0-3 - buttons, 4-5 - selection buttons, 6-7 - zero    */
    CC_buffer[HIDD_LE_REPORT_CC_SIZE],
    /* Keyboard out report keeps data for leds in one byte
    LEDS: bit 0 NUM LOCK, 1 CAPS LOCK, 2 SCROLL LOCK, 3 COMPOSE, 4 KANA, 5 to 7 RESERVED (zeroes) */
    Leds_buffer[HIDD_LE_REPORT_KB_OUT_SIZE],
    /* battery level */
    Battery_level[] = { BATTERY_DEFAULT_LEVEL, },
    /* Feature data - custom data for this device */
    Feature_buffer[] = "olegos";

static struct hid_notify_data {
    const char *name;
    int handle_num;        // handle index from Svc_char_handles
    int handle_boot_num;   // handle num in boot mode
    uint8_t *buffer;            // data to send
    size_t buffer_size;
    bool can_indicate;          // preffered method, because central will response to it
    bool can_notify;
} Notify_data_reports[] =
{
    {   .name = "mouse",
        .handle_num = HANDLE_HID_MOUSE_REPORT,
        .handle_boot_num = HANDLE_HID_BOOT_MOUSE_REPORT,
        .buffer = Mouse_buffer,
        .buffer_size = HIDD_LE_REPORT_MOUSE_SIZE,
        .can_indicate = false, .can_notify= false
    },
    {   .name = "keyboard",
        .handle_num = HANDLE_HID_KB_IN_REPORT,
        .handle_boot_num = HANDLE_HID_BOOT_KB_IN_REPORT,
        .buffer = Keyboard_buffer,
        .buffer_size = HIDD_LE_REPORT_KB_IN_SIZE,
        .can_indicate = false, .can_notify= false
    },
    {   .name = "leds",
        .handle_num = HANDLE_HID_KB_OUT_REPORT,
        .handle_boot_num = HANDLE_HID_BOOT_KB_OUT_REPORT,
        .buffer = Leds_buffer,
        .buffer_size = HIDD_LE_REPORT_KB_OUT_SIZE,
        .can_indicate = false, .can_notify= false
    },
    {   .name = "consumer control",
        .handle_num = HANDLE_HID_CC_REPORT,
        .handle_boot_num = HANDLE_HID_CC_REPORT,
        .buffer = CC_buffer,
        .buffer_size = HIDD_LE_REPORT_CC_SIZE,
        .can_indicate = false, .can_notify= false
    },
    {   .name = "battery level",
        .handle_num = HANDLE_BATTERY_LEVEL,
        .handle_boot_num = HANDLE_BATTERY_LEVEL,
        .buffer = Battery_level,
        .buffer_size = HIDD_LE_BATTERY_LEVEL_SIZE,
        .can_indicate = false, .can_notify= false
    },
    {   .name = "feature",
        .handle_num = HANDLE_HID_FEATURE_REPORT,
        .handle_boot_num = HANDLE_HID_FEATURE_REPORT,
        .buffer = Feature_buffer,
        .buffer_size = HIDD_LE_REPORT_FEATURE,
        .can_indicate = false, .can_notify= false
    },
};

static struct hid_device_data {
    /* Mutex semaphore for access to this struct */
    //xSemaphoreHandle *semaphore;
    tls_os_mutex_t *semaphore;
    bool suspended_state;
    bool report_mode_boot;
    bool connected;
    uint16_t conn_handle;
} wm_hid_dev = {
    .semaphore = 0,
    .connected = false,
    .suspended_state = false,
    .report_mode_boot = false,
};


#define NO_MINKEYSIZE     .min_key_size = DEFAULT_MIN_KEY_SIZE
#define NO_ARG_MINKEYSIZE .arg = NULL, NO_MINKEYSIZE
#define NO_ARG_DESCR_MKS  .descriptors = NULL, NO_ARG_MINKEYSIZE
#define NO_DESCR_MKS      .descriptors = NULL, NO_MINKEYSIZE

#define MY_NOTIFY_FLAGS (BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY | BLE_GATT_CHR_F_INDICATE)

static uint8_t g_ble_demo_hid_inited = 0; 
static uint8_t g_ble_demo_hid_connected = 0;
static uint16_t g_ble_demo_conn_handle = 0 ;
static ble_addr_t g_ble_remote_addr;
static bool g_ble_disconnect_and_continue_to_adv = false;
extern int update_hid_dev_state(uint8_t disconnected, ble_addr_t *addr);
extern int check_hid_dev_black_white_list(ble_addr_t *addr);

static int
hid_svr_chr_access(uint16_t conn_handle, uint16_t attr_handle,
                             struct ble_gatt_access_ctxt *ctxt,
                             void *arg);
static int
ble_svc_report_access(uint16_t conn_handle, uint16_t attr_handle,
                             struct ble_gatt_access_ctxt *ctxt,
                             void *arg);
static int
ble_svc_battery_access(uint16_t conn_handle, uint16_t attr_handle,
                          struct ble_gatt_access_ctxt *ctxt,
                          void *arg);

static int
ble_svc_dis_access(uint16_t conn_handle, uint16_t attr_handle,
                   struct ble_gatt_access_ctxt *ctxt, void *arg);

/* handles for all characteristics in GATT services */
uint16_t Svc_char_handles[HANDLE_HID_COUNT];


const struct ble_gatt_svc_def Gatt_svr_included_services[] = {
    {
        /*** Battery Service. */
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(BLE_SVC_BAS_UUID16),
        .includes = NULL,
        .characteristics = (struct ble_gatt_chr_def[]) { {
        /*** Battery level characteristic */
            .uuid = BLE_UUID16_DECLARE(BLE_SVC_BAS_CHR_UUID16_BATTERY_LEVEL),
            .access_cb = ble_svc_battery_access,
            .arg = (void *) HANDLE_BATTERY_LEVEL,
            .val_handle = &Svc_char_handles[HANDLE_BATTERY_LEVEL],
            .flags = MY_NOTIFY_FLAGS,
            NO_MINKEYSIZE,
            .descriptors = (struct ble_gatt_dsc_def[]) { {
                .uuid = BLE_UUID16_DECLARE(GATT_UUID_BAT_PRESENT_DESCR),
                .att_flags = BLE_ATT_F_READ | BLE_ATT_F_READ_ENC,
                .access_cb = ble_svc_battery_access,
                NO_ARG_MINKEYSIZE,
            }, {
                0, /* No more descriptors in this characteristic. */
            } },
        }, {
            0, /* No more characteristics in this service. */
        } },
    },

    { /*** Service: Device Information Service (DIS). */
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(BLE_SVC_DIS_UUID16),
        .includes = NULL,
        .characteristics = (struct ble_gatt_chr_def[]) { {
        /*** Characteristic: Model Number String */
            .uuid = BLE_UUID16_DECLARE(BLE_SVC_DIS_CHR_UUID16_MODEL_NUMBER),
            .access_cb = ble_svc_dis_access,
            .val_handle = &Svc_char_handles[HANDLE_DIS_MODEL_NUMBER],
            .flags = BLE_GATT_CHR_F_READ | (BLE_SVC_DIS_MODEL_NUMBER_READ_PERM),
            NO_ARG_DESCR_MKS,
        }, {
        /*** Characteristic: Serial Number String */
            .uuid = BLE_UUID16_DECLARE(BLE_SVC_DIS_CHR_UUID16_SERIAL_NUMBER),
            .access_cb = ble_svc_dis_access,
            .val_handle = &Svc_char_handles[HANDLE_DIS_SERIAL_NUMBER],
            .flags = BLE_GATT_CHR_F_READ | (BLE_SVC_DIS_SERIAL_NUMBER_READ_PERM),
            NO_ARG_DESCR_MKS,
        }, {
        /*** Characteristic: Hardware Revision String */
            .uuid = BLE_UUID16_DECLARE(BLE_SVC_DIS_CHR_UUID16_HARDWARE_REVISION),
            .access_cb = ble_svc_dis_access,
            .val_handle = &Svc_char_handles[HANDLE_DIS_HARDWARE_REVISION],
            .flags = BLE_GATT_CHR_F_READ | (BLE_SVC_DIS_HARDWARE_REVISION_READ_PERM),
            NO_ARG_DESCR_MKS,
        }, {
        /*** Characteristic: Firmware Revision String */
            .uuid = BLE_UUID16_DECLARE(BLE_SVC_DIS_CHR_UUID16_FIRMWARE_REVISION),
            .access_cb = ble_svc_dis_access,
            .val_handle = &Svc_char_handles[HANDLE_DIS_FIRMWARE_REVISION],
            .flags = BLE_GATT_CHR_F_READ | (BLE_SVC_DIS_FIRMWARE_REVISION_READ_PERM),
            NO_ARG_DESCR_MKS,
        }, {
        /*** Characteristic: Software Revision String */
            .uuid = BLE_UUID16_DECLARE(BLE_SVC_DIS_CHR_UUID16_SOFTWARE_REVISION),
            .access_cb = ble_svc_dis_access,
            .val_handle = &Svc_char_handles[HANDLE_DIS_SOFWARE_REVISION],
            .flags = BLE_GATT_CHR_F_READ | (BLE_SVC_DIS_SOFTWARE_REVISION_READ_PERM),
            NO_ARG_DESCR_MKS,
        }, {
        /*** Characteristic: Manufacturer Name */
            .uuid = BLE_UUID16_DECLARE(BLE_SVC_DIS_CHR_UUID16_MANUFACTURER_NAME),
            .access_cb = ble_svc_dis_access,
            .val_handle = &Svc_char_handles[HANDLE_DIS_MANUFACTURER_NAME],
            .flags = BLE_GATT_CHR_F_READ | (BLE_SVC_DIS_MANUFACTURER_NAME_READ_PERM),
            NO_ARG_DESCR_MKS,
        }, {
      /*** Characteristic: System Id */
            .uuid = BLE_UUID16_DECLARE(BLE_SVC_DIS_CHR_UUID16_SYSTEM_ID),
            .access_cb = ble_svc_dis_access,
            .val_handle = &Svc_char_handles[HANDLE_DIS_SYSTEM_ID],
            .flags = BLE_GATT_CHR_F_READ | (BLE_SVC_DIS_SYSTEM_ID_READ_PERM),
            NO_ARG_DESCR_MKS,
        }, {
      /*** Characteristic: System Id */
            .uuid = BLE_UUID16_DECLARE(BLE_SVC_DIS_CHR_UUID16_PNP_INFO),
            .access_cb = ble_svc_dis_access,
            .val_handle = &Svc_char_handles[HANDLE_DIS_PNP_INFO],
            .flags = BLE_GATT_CHR_F_READ,
            NO_ARG_DESCR_MKS,
        }, {
            0, /* No more characteristics in this service */
        }, }
    },

    {
        0, /* No more services. */
    },
};

const struct ble_gatt_svc_def *Inc_svcs[] = {
    &Gatt_svr_included_services[0],
    NULL,
};

const struct ble_gatt_svc_def Gatt_svr_svcs[] = {
    {
        /*** HID Service */
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(GATT_UUID_HID_SERVICE),
        .includes = Inc_svcs,
        .characteristics = (struct ble_gatt_chr_def[])
        {
            {
            /*** HID INFO characteristic */
                .uuid = BLE_UUID16_DECLARE(GATT_UUID_HID_INFORMATION),
                .access_cb = hid_svr_chr_access,
                .val_handle = &Svc_char_handles[HANDLE_HID_INFORMATION],
                .flags = BLE_GATT_CHR_F_READ, // | BLE_GATT_CHR_F_READ_ENC,
                NO_ARG_DESCR_MKS,
            }, {
            /*** HID Control Point */
                .uuid = BLE_UUID16_DECLARE(GATT_UUID_HID_CONTROL_POINT),
                .access_cb = hid_svr_chr_access,
                .val_handle = &Svc_char_handles[HANDLE_HID_CONTROL_POINT],
                .flags = BLE_GATT_CHR_F_WRITE, // | BLE_GATT_CHR_F_WRITE_ENC,
                NO_ARG_DESCR_MKS,
            }, {
            /*** Report Map */
                .uuid = BLE_UUID16_DECLARE(GATT_UUID_HID_REPORT_MAP),
                .access_cb = hid_svr_chr_access,
                .val_handle = &Svc_char_handles[HANDLE_HID_REPORT_MAP],
                .flags = BLE_GATT_CHR_F_READ,
                NO_ARG_MINKEYSIZE,
                .descriptors = (struct ble_gatt_dsc_def[]) { {
                    /*** External Report Reference Descriptor */
                    .uuid = BLE_UUID16_DECLARE(GATT_UUID_EXT_RPT_REF_DESCR),
                    .att_flags = BLE_ATT_F_READ,
                    .access_cb = hid_svr_chr_access,
                    NO_ARG_MINKEYSIZE,
                }, {
                    0, /* No more descriptors in this characteristic. */
                } },
            }, {
            /*** Protocol Mode Characteristic */
                .uuid = BLE_UUID16_DECLARE(GATT_UUID_HID_PROTO_MODE),
                .access_cb = hid_svr_chr_access,
                .val_handle = &Svc_char_handles[HANDLE_HID_PROTO_MODE],
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
                NO_ARG_DESCR_MKS,
            }, {
            /*** Mouse hid report */
                .uuid = BLE_UUID16_DECLARE(GATT_UUID_HID_REPORT),
                .access_cb = ble_svc_report_access,
                .arg = (void *)HANDLE_HID_MOUSE_REPORT,
                .val_handle = &Svc_char_handles[HANDLE_HID_MOUSE_REPORT],
                .flags = MY_NOTIFY_FLAGS,
                .min_key_size = DEFAULT_MIN_KEY_SIZE,
                .descriptors = (struct ble_gatt_dsc_def[]) { {
                    /* Report Reference Descriptor */
                    .uuid = BLE_UUID16_DECLARE(GATT_UUID_RPT_REF_DESCR),
                    .att_flags = BLE_ATT_F_READ,
                    .access_cb = ble_svc_report_access,
                    .arg = (void *)HANDLE_HID_MOUSE_REPORT,
                    .min_key_size = DEFAULT_MIN_KEY_SIZE,
                }, {
                    0, /* No more descriptors in this characteristic. */
                } },
            }, {
            /*** Keyboard hid report */
                .uuid = BLE_UUID16_DECLARE(GATT_UUID_HID_REPORT),
                .access_cb = ble_svc_report_access,
                .arg = (void *)HANDLE_HID_KB_IN_REPORT,
                .val_handle = &Svc_char_handles[HANDLE_HID_KB_IN_REPORT],
                .flags = MY_NOTIFY_FLAGS,
                .min_key_size = DEFAULT_MIN_KEY_SIZE,
                .descriptors = (struct ble_gatt_dsc_def[]) { {
                    /* Report Reference Descriptor */
                    .uuid = BLE_UUID16_DECLARE(GATT_UUID_RPT_REF_DESCR),
                    .att_flags = BLE_ATT_F_READ,
                    .access_cb = ble_svc_report_access,
                    .arg = (void *)HANDLE_HID_KB_IN_REPORT,
                    .min_key_size = DEFAULT_MIN_KEY_SIZE,
                }, {
                    0, /* No more descriptors in this characteristic. */
                } },
            }, {
            /*** Keyboard hid out (LED IN/OUT) report */
                .uuid = BLE_UUID16_DECLARE(GATT_UUID_HID_REPORT),
                .access_cb = ble_svc_report_access,
                .arg = (void *)HANDLE_HID_KB_OUT_REPORT,
                .val_handle = &Svc_char_handles[HANDLE_HID_KB_OUT_REPORT],
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_NO_RSP,
                .min_key_size = DEFAULT_MIN_KEY_SIZE,
                .descriptors = (struct ble_gatt_dsc_def[]) { {
                    /* Report Reference Descriptor */
                    .uuid = BLE_UUID16_DECLARE(GATT_UUID_RPT_REF_DESCR),
                    .att_flags = BLE_ATT_F_READ,
                    .access_cb = ble_svc_report_access,
                    .arg = (void *)HANDLE_HID_KB_OUT_REPORT,
                    .min_key_size = DEFAULT_MIN_KEY_SIZE,
                }, {
                    0, /* No more descriptors in this characteristic. */
                } },
            }, {
            /*** Consumer control hid report */
                .uuid = BLE_UUID16_DECLARE(GATT_UUID_HID_REPORT),
                .access_cb = ble_svc_report_access,
                .arg = (void *)HANDLE_HID_CC_REPORT,
                .val_handle = &Svc_char_handles[HANDLE_HID_CC_REPORT],
                .flags = MY_NOTIFY_FLAGS,
                .min_key_size = DEFAULT_MIN_KEY_SIZE,
                .descriptors = (struct ble_gatt_dsc_def[]) { {
                    /* Report Reference Descriptor */
                    .uuid = BLE_UUID16_DECLARE(GATT_UUID_RPT_REF_DESCR),
                    .att_flags = BLE_ATT_F_READ,
                    .access_cb = ble_svc_report_access,
                    .arg = (void *)HANDLE_HID_CC_REPORT,
                    .min_key_size = DEFAULT_MIN_KEY_SIZE,
                }, {
                    0, /* No more descriptors in this characteristic. */
                } },
            }, {
            /*** Keyboard input boot hid report */
                .uuid = BLE_UUID16_DECLARE(GATT_UUID_HID_BT_KB_INPUT),
                .access_cb = ble_svc_report_access,
                .arg = (void *)HANDLE_HID_BOOT_KB_IN_REPORT,
                .val_handle = &Svc_char_handles[HANDLE_HID_BOOT_KB_IN_REPORT],
                .flags = MY_NOTIFY_FLAGS,
                NO_DESCR_MKS,
            }, {
            /*** Keyboard output boot hid report */
                .uuid = BLE_UUID16_DECLARE(GATT_UUID_HID_BT_KB_OUTPUT),
                .access_cb = ble_svc_report_access,
                .arg = (void *)HANDLE_HID_BOOT_KB_OUT_REPORT,
                .val_handle = &Svc_char_handles[HANDLE_HID_BOOT_KB_OUT_REPORT],
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_NO_RSP,
                NO_DESCR_MKS,
            }, {
            /*** Mouse input boot hid report */
                .uuid = BLE_UUID16_DECLARE(GATT_UUID_HID_BT_MOUSE_INPUT),
                .access_cb = ble_svc_report_access,
                .arg = (void *)HANDLE_HID_BOOT_MOUSE_REPORT,
                .val_handle = &Svc_char_handles[HANDLE_HID_BOOT_MOUSE_REPORT],
                .flags = MY_NOTIFY_FLAGS,
                NO_DESCR_MKS,
            }, {
            /*** Feature hid report */
                .uuid = BLE_UUID16_DECLARE(GATT_UUID_HID_REPORT),
                .access_cb = ble_svc_report_access,
                .arg = (void *)HANDLE_HID_FEATURE_REPORT,
                .val_handle = &Svc_char_handles[HANDLE_HID_FEATURE_REPORT],
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
                .min_key_size = DEFAULT_MIN_KEY_SIZE,
                .descriptors = (struct ble_gatt_dsc_def[]) { {
                    /* Report Reference Descriptor */
                    .uuid = BLE_UUID16_DECLARE(GATT_UUID_RPT_REF_DESCR),
                    .att_flags = BLE_ATT_F_READ,
                    .access_cb = ble_svc_report_access,
                    .arg = (void *)HANDLE_HID_FEATURE_REPORT,
                    .min_key_size = DEFAULT_MIN_KEY_SIZE,
                }, {
                    0, /* No more descriptors in this characteristic. */
                } },
            }, {
                0, /* No more characteristics in this service. */
            }
        },
    },

    {
        0, /* No more services. */
    },
};



#define BCDHID_DATA 0x0111

const uint8_t HidInfo[HID_INFORMATION_LEN] = {
    LO_UINT16(BCDHID_DATA), HI_UINT16(BCDHID_DATA),   // bcdHID (USB HID version)
    0x00,                                             // bCountryCode
    HID_KBD_FLAGS                                     // Flags
};

// HID External Report Reference Descriptor
uint16_t HidExtReportRefDesc = BLE_SVC_BAS_CHR_UUID16_BATTERY_LEVEL;
uint8_t  HidProtocolMode = HID_PROTOCOL_MODE_REPORT;

/* Report reference table, byte 0 - report id from report map, byte 1 - report type (in,out,feature)*/
struct report_reference_table Hid_report_ref_data[] = {
    { .id = HANDLE_HID_MOUSE_REPORT,    .hidReportRef = { HID_RPT_ID_MOUSE_IN,  HID_REPORT_TYPE_INPUT   }},
    { .id = HANDLE_HID_KB_IN_REPORT,    .hidReportRef = { HID_RPT_ID_KB_IN,     HID_REPORT_TYPE_INPUT   }},
    { .id = HANDLE_HID_KB_OUT_REPORT,   .hidReportRef = { HID_RPT_ID_KB_IN,     HID_REPORT_TYPE_OUTPUT  }},
    { .id = HANDLE_HID_CC_REPORT,       .hidReportRef = { HID_RPT_ID_CC_IN,     HID_REPORT_TYPE_INPUT   }},
    { .id = HANDLE_HID_FEATURE_REPORT,  .hidReportRef = { HID_RPT_ID_FEATURE,   HID_REPORT_TYPE_FEATURE }},
};
size_t Hid_report_ref_data_count = sizeof(Hid_report_ref_data)/sizeof(Hid_report_ref_data[0]);

// battery level unit - percents
struct prf_char_pres_fmt Battery_level_units = {
    .format = 4,      // Unsigned 8-bit
    .exponent = 0,
    .unit = 0x27AD,   // percentage
    .name_space = 1,  // BLUETOOTH SIG
    .description = 0
};

/* Device information
*/
struct ble_svc_dis_data Hid_dis_data = {
    .model_number      = BLE_SVC_DIS_MODEL_NUMBER_DEFAULT,
    .serial_number     = BLE_SVC_DIS_SERIAL_NUMBER_DEFAULT,
    .firmware_revision = BLE_SVC_DIS_FIRMWARE_REVISION_DEFAULT,
    .hardware_revision = BLE_SVC_DIS_HARDWARE_REVISION_DEFAULT,
    .software_revision = BLE_SVC_DIS_SOFTWARE_REVISION_DEFAULT,
    .manufacturer_name = BLE_SVC_DIS_MANUFACTURER_NAME_DEFAULT,
    .system_id         = BLE_SVC_DIS_SYSTEM_ID_DEFAULT,
};







/* mark report for indicate/notify when central subscribes to service charachetric with report */
static void
hid_set_notify(uint16_t attr_handle, uint8_t cur_notify, uint8_t cur_indicate)
{
    int report_idx = -1;

    /* find hid_notify_data struct index of reports array for given atribute handle */
    for (int i = 0; i < sizeof(Notify_data_reports)/sizeof(Notify_data_reports[0]); ++i) {
        uint16_t current_handle_idx = wm_hid_dev.report_mode_boot ?
            Notify_data_reports[i].handle_boot_num :
            Notify_data_reports[i].handle_num;
        if (attr_handle == Svc_char_handles[current_handle_idx]) {
            report_idx = i;
            break;
        }
    }
    if (report_idx == -1) {
        TLS_BT_APPL_TRACE_WARNING( "%s: attr_handle 0x%04X not found in reports\r\n", __FUNCTION__, attr_handle);
    } else {
        Notify_data_reports[report_idx].can_indicate = cur_indicate;
        Notify_data_reports[report_idx].can_notify = cur_notify;

        TLS_BT_APPL_TRACE_API( "%s: service %s, attr_handle %d, notify %d, indicate %d\r\n",
                    __FUNCTION__, Notify_data_reports[report_idx].name, attr_handle, cur_notify, cur_indicate);
    }
}

/* lock report buffers with mutex semaphore */
static int
lock_hid_data()
{
    tls_os_status_t status;
    
    if (!wm_hid_dev.semaphore) {
        TLS_BT_APPL_TRACE_WARNING( "%s semaphore is NULL\r\n", __FUNCTION__);
        return 1;
    }

    status = tls_os_mutex_acquire(wm_hid_dev.semaphore, ble_npl_time_ms_to_ticks32(HID_DEV_BUF_MUTEX_WAIT));
    //if (xSemaphoreTake( wm_hid_dev.semaphore, ble_npl_time_ms_to_ticks32(HID_DEV_BUF_MUTEX_WAIT) ) == pdTRUE) {
    if(status == TLS_OS_SUCCESS){
        return 0;
    } else {
        TLS_BT_APPL_TRACE_WARNING( "%s: can't lock\r\n", __FUNCTION__);
    }

    return 2;

}

/* unlock report buffers locked with mutex semaphore */
static int
unlock_hid_data()
{
    tls_os_status_t status;
    
    if (!wm_hid_dev.semaphore) {
        TLS_BT_APPL_TRACE_WARNING( "%s semaphore is NULL\r\n", __FUNCTION__);
        return 1;
    }
    status = tls_os_mutex_release(wm_hid_dev.semaphore);

    if(status ==  TLS_OS_SUCCESS)
    {
        return 0;
    }
    //if (xSemaphoreGive( wm_hid_dev.semaphore ) == pdTRUE) {
    //    return 0;
    //}
    return 2;

}

/* zero all fields on new connection */
void
hid_clean_vars()
{
    // save semaphore for new connection use
    tls_os_mutex_t *semaphore_saved = wm_hid_dev.semaphore;
    int rc = -1;

    if (semaphore_saved) {
        rc = lock_hid_data();
    }

    memset(&wm_hid_dev, 0, sizeof(struct hid_device_data));

    for (int i = 0; i < sizeof(Notify_data_reports)/sizeof(Notify_data_reports[0]); ++i) {
        Notify_data_reports[i].can_indicate = false;
        Notify_data_reports[i].can_notify = false;
        switch (Notify_data_reports[i].handle_num) {
            case HANDLE_HID_MOUSE_REPORT:
            case HANDLE_HID_KB_IN_REPORT:
            case HANDLE_HID_KB_OUT_REPORT:
            case HANDLE_HID_CC_REPORT:
                memset(Notify_data_reports[i].buffer, 0, Notify_data_reports[i].buffer_size);
        }
    }

    if (semaphore_saved) {
        wm_hid_dev.semaphore = semaphore_saved;
    } else {
        //wm_hid_dev.semaphore = xSemaphoreCreateMutex();
        tls_os_mutex_create(0, &wm_hid_dev.semaphore);
        assert(wm_hid_dev.semaphore != NULL);
    }

    //wm_hid_dev.conn_handle = desc->conn_handle;
    //wm_hid_dev.connected = true;

    if (!rc) {
        unlock_hid_data();
    }
}


int
hid_read_buffer(struct os_mbuf *buf, int handle_num)
{
    int rc = 0;
    int rep_idx = -1;

    for (int i = 0; i < sizeof(Notify_data_reports)/sizeof(Notify_data_reports[0]); ++i) {
        if (Notify_data_reports[i].handle_num == handle_num ||
            Notify_data_reports[i].handle_boot_num == handle_num) {
            rep_idx = i;
            break;
        }
    }

    if (rep_idx != -1 && lock_hid_data() == 0) {
        rc = os_mbuf_append(buf,
            Notify_data_reports[rep_idx].buffer,
            Notify_data_reports[rep_idx].buffer_size);
        unlock_hid_data();

    } else {
        if (rep_idx == -1) TLS_BT_APPL_TRACE_WARNING( "%s: handle_num %d not found\r\n", __FUNCTION__, handle_num);
        return 2;
    }

    return rc;
}

int
hid_write_buffer(struct os_mbuf *buf, int handle_num)
{
    int rc = 0;

    int rep_idx = -1;

    for (int i = 0; i < sizeof(Notify_data_reports)/sizeof(Notify_data_reports[0]); ++i) {
        if (Notify_data_reports[i].handle_num == handle_num ||
            Notify_data_reports[i].handle_boot_num == handle_num) {
            rep_idx = i;
            break;
        }
    }
    if (rep_idx != -1 && lock_hid_data() == 0) {
        if (OS_MBUF_PKTLEN(buf) == Notify_data_reports[rep_idx].buffer_size) {
            rc = ble_hs_mbuf_to_flat(buf,
                Notify_data_reports[rep_idx].buffer,
                OS_MBUF_PKTLEN(buf),
                NULL);
        } else {
            rc = 4;
        }
        unlock_hid_data();
        if (rc == 0) {
            if (handle_num == HANDLE_HID_KB_OUT_REPORT) {
                // change LEDs level when Keyboard out report received
                // set_leds(Leds_buffer[0]);
                uint8_t req_led_payload[11] = {0x57, 0xAB, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xac, 0x20};
                req_led_payload[7] = Leds_buffer[0];
                tls_uart_write_async(TLS_UART_1,req_led_payload,sizeof(req_led_payload));
                TLS_BT_APPL_TRACE_WARNING("KB LED:%02x\r\n", Leds_buffer[0]);
            }
        }
    } else {
        return 2;
    }

    return rc;
}

/*  send report to central using different ways
    0 - using ble_gattc_indicate_custom     using custom buffer
    1 - using ble_gattc_indicate            to only one connection
    2 - using ble_gatts_chr_updated         to all connected centrals
*/
#define SEND_METHOD_CUSTOM  0
#define SEND_METHOD_STD     1
#define SEND_METHOD_ALL     2

#define NOTIFY_METHOD SEND_METHOD_STD

/* send report data to central using notify/indicate */
int
hid_send_report(int report_handle_num)
{
    /* check semaphore, connection and suspend state */
    if ( !wm_hid_dev.semaphore || !wm_hid_dev.connected || wm_hid_dev.suspended_state) {
        TLS_BT_APPL_TRACE_API( "%s semaphore %p %d %d\r\n", __FUNCTION__,
            wm_hid_dev.semaphore, wm_hid_dev.connected, wm_hid_dev.suspended_state);
        return 1;
    }

    int report_idx = -1;

    for (int i = 0; i < sizeof(Notify_data_reports)/sizeof(Notify_data_reports[0]); ++i) {
        if (report_handle_num == Notify_data_reports[i].handle_num) {
            report_idx = i;
            break;
        }
    }

    if (report_idx == -1) {
        TLS_BT_APPL_TRACE_WARNING( "%s: Unknown report_handle_num %d\r\n", __FUNCTION__, report_handle_num);
        return 2;
    }

    uint16_t send_handle;
    int rc = 0;

    if (wm_hid_dev.report_mode_boot) {
        send_handle = Svc_char_handles[Notify_data_reports[report_idx].handle_boot_num];
    } else {
        send_handle = Svc_char_handles[Notify_data_reports[report_idx].handle_num];
    }

    switch (NOTIFY_METHOD) {
        case SEND_METHOD_CUSTOM: {
            if (lock_hid_data() == 0) {
                struct os_mbuf *om = ble_hs_mbuf_from_flat(
                    Notify_data_reports[report_idx].buffer,
                    Notify_data_reports[report_idx].buffer_size);
                unlock_hid_data();

                if (Notify_data_reports[report_idx].can_indicate) {
                    rc = ble_gattc_indicate_custom(wm_hid_dev.conn_handle, send_handle, om);
                } else if (Notify_data_reports[report_idx].can_notify) {
                    rc = ble_gattc_notify_custom(wm_hid_dev.conn_handle, send_handle, om);
                }
            }
            break;
        }

        case SEND_METHOD_STD:
            if (Notify_data_reports[report_idx].can_indicate) {
                rc = ble_gattc_indicate(wm_hid_dev.conn_handle, send_handle);
            } else if (Notify_data_reports[report_idx].can_notify) {
                rc = ble_gattc_notify(wm_hid_dev.conn_handle, send_handle);
            }
            break;

        case SEND_METHOD_ALL:
            ble_gatts_chr_updated(send_handle);
            rc = 0;
            break;
    }
    if (rc) {
        TLS_BT_APPL_TRACE_ERROR( "%s: Notify error in function\r\n", __FUNCTION__);
    }

    return 0;
}

uint8_t
hid_battery_level_get(void)
{
    return Battery_level[0];
}

int
hid_battery_level_set(uint8_t level)
{
    int rc = 0;

    if (lock_hid_data() == 0) {
        if (Battery_level[0] != level) {
            Battery_level[0] = level;
        }
        unlock_hid_data();

        rc = hid_send_report(HANDLE_BATTERY_LEVEL);
    } else {
        return -2;
    }

    return rc;
}

int
hid_mouse_change_key(int cmd, int8_t move_x, int8_t move_y, bool pressed)
{
    int rc = 0;

    if (lock_hid_data() == 0) {

        switch (cmd) {
            case HID_MOUSE_LEFT:
            case HID_MOUSE_MIDDLE:
            case HID_MOUSE_RIGHT:
                if (pressed) {
                    Mouse_buffer[0] |= 1 << (cmd - HID_MOUSE_LEFT);
                } else {
                    Mouse_buffer[0] &= ~(1 << (cmd - HID_MOUSE_LEFT));
                }
                break;
            case HID_MOUSE_WHEEL_UP:
                Mouse_buffer[3] = 1;
                break;
            case HID_MOUSE_WHEEL_DOWN:
                Mouse_buffer[3] = -1;
                break;
            default:
                rc = 1;
                TLS_BT_APPL_TRACE_API( "Unknown mouse cmd %d!\r\n", cmd);
        }

        Mouse_buffer[1] = move_x;
        Mouse_buffer[2] = move_y;

        unlock_hid_data();

        if (rc == 0 || move_x || move_y) {
            rc = hid_send_report(HANDLE_HID_MOUSE_REPORT);
        }
    } else {
        rc = 1;
    }

    return rc;
}

int
hid_cc_build_report(uint8_t *buffer, consumer_cmd_t cmd, bool pressed)
{
    if (!buffer) {
        TLS_BT_APPL_TRACE_ERROR( "%s(), the buffer is NULL.\r\n", __func__);
        return 1;
    }

    int rc = 0;

    switch (cmd) {
        case HID_CONSUMER_CHANNEL_UP:
            HID_CC_RPT_SET_CHANNEL(buffer, pressed ? HID_CC_RPT_CHANNEL_UP : 0);
            break;

        case HID_CONSUMER_CHANNEL_DOWN:
            HID_CC_RPT_SET_CHANNEL(buffer, pressed ? HID_CC_RPT_CHANNEL_DOWN : 0);
            break;

        case HID_CONSUMER_VOLUME_UP:
            HID_CC_RPT_SET_VOLUME(buffer, pressed ? HID_CC_RPT_VOLUME_UP : 0);
            break;

        case HID_CONSUMER_VOLUME_DOWN:
            HID_CC_RPT_SET_VOLUME(buffer, pressed ? HID_CC_RPT_VOLUME_DOWN : 0);
            break;

        case HID_CONSUMER_MUTE:
            HID_CC_RPT_SET_BUTTON(buffer, pressed ? HID_CC_RPT_MUTE : 0);
            break;

        case HID_CONSUMER_POWER:
            HID_CC_RPT_SET_BUTTON(buffer, pressed ? HID_CC_RPT_POWER : 0);
            break;

        case HID_CONSUMER_RECALL_LAST:
            HID_CC_RPT_SET_BUTTON(buffer, pressed ? HID_CC_RPT_LAST : 0);
            break;

        case HID_CONSUMER_ASSIGN_SEL:
            HID_CC_RPT_SET_BUTTON(buffer, pressed ? HID_CC_RPT_ASSIGN_SEL : 0);
            break;

        case HID_CONSUMER_PLAY:
            HID_CC_RPT_SET_BUTTON(buffer, pressed ? HID_CC_RPT_PLAY : 0);
            break;

        case HID_CONSUMER_PAUSE:
            HID_CC_RPT_SET_BUTTON(buffer, pressed ? HID_CC_RPT_PAUSE : 0);
            break;

        case HID_CONSUMER_RECORD:
            HID_CC_RPT_SET_BUTTON(buffer, pressed ? HID_CC_RPT_RECORD : 0);
            break;

        case HID_CONSUMER_FAST_FORWARD:
            HID_CC_RPT_SET_BUTTON(buffer, pressed ? HID_CC_RPT_FAST_FWD : 0);
            break;

        case HID_CONSUMER_REWIND:
            HID_CC_RPT_SET_BUTTON(buffer, pressed ? HID_CC_RPT_REWIND : 0);
            break;

        case HID_CONSUMER_SCAN_NEXT_TRK:
            HID_CC_RPT_SET_BUTTON(buffer, pressed ? HID_CC_RPT_SCAN_NEXT_TRK : 0);
            break;

        case HID_CONSUMER_SCAN_PREV_TRK:
            HID_CC_RPT_SET_BUTTON(buffer, pressed ? HID_CC_RPT_SCAN_PREV_TRK : 0);
            break;

        case HID_CONSUMER_STOP:
            HID_CC_RPT_SET_BUTTON(buffer, pressed ? HID_CC_RPT_STOP : 0);
            break;

        default:
            rc = 2;
            break;
    }

    return rc;
}

int
hid_cc_change_key(int key, bool pressed)
{
    int rc = 0;

    if (lock_hid_data() == 0) {

        rc = hid_cc_build_report(CC_buffer, (consumer_cmd_t) key, pressed);

        unlock_hid_data();

        if (rc == 0) {
            rc = hid_send_report(HANDLE_HID_CC_REPORT);
        }
    } else {
        rc = 2;
    }

    return rc;
}

int
hid_keyboard_change_key(uint8_t key, bool pressed)
{
    int rc = 0;

    if (lock_hid_data() == 0) {

        if (key >= HID_KEY_LEFT_CTRL && key <= HID_KEY_RIGHT_GUI) {
            // it is modifier (Ctrl Shift Alt or Winkey)
            if (pressed) {
                Keyboard_buffer[0] |= 1 << ( key - HID_KEY_LEFT_CTRL );
            } else {
                Keyboard_buffer[0] &= ~( 1 << ( key - HID_KEY_LEFT_CTRL ));
            }
        } else {
            // ordinary key
            bool found = false;
            if (pressed) {
                // if pressed, adding key to buffer
                for (int i = 2; i < HIDD_LE_REPORT_KB_IN_SIZE; ++i) {
                    if (Keyboard_buffer[i] == 0) {
                        Keyboard_buffer[i] = key;
                        found = true;
                        break;
                    }
                }
            } else {
                // if key is released then delete key from buffer
                for (int i = 2; i < HIDD_LE_REPORT_KB_IN_SIZE; ++i) {
                    if (!found) {
                        if (Keyboard_buffer[i] == key) {
                            Keyboard_buffer[i] = 0;
                            found = true;
                        }
                    } else {
                        if (Keyboard_buffer[i] == 0) {
                            break;
                        }
                        // shift other keys to the left
                        Keyboard_buffer[i-1] = Keyboard_buffer[i];
                        Keyboard_buffer[i] = 0;
                    }
                }
            }
            if (!found) {
                rc = 1; // no room for new key or key not found
            }
        }

        unlock_hid_data();

        if (rc == 0) {
            rc = hid_send_report(HANDLE_HID_KB_IN_REPORT);
        }
    } else {
        rc = 2;
    }

    return rc;
}

int
gatt_svr_chr_write(struct os_mbuf *om, uint16_t min_len, uint16_t max_len,
                   void *dst, uint16_t *len)
{
    uint16_t om_len;
    int rc;

    om_len = OS_MBUF_PKTLEN(om);
    if (om_len < min_len || om_len > max_len) {
        return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
    }

    rc = ble_hs_mbuf_to_flat(om, dst, max_len, len);
    if (rc != 0) {
        return BLE_ATT_ERR_UNLIKELY;
    }

    return 0;
}

static int
hid_svr_chr_access(uint16_t conn_handle, uint16_t attr_handle,
                             struct ble_gatt_access_ctxt *ctxt,
                             void *arg)
{
    uint16_t uuid16 = ble_uuid_u16(ctxt->chr->uuid);
    int rc;

    TLS_BT_APPL_TRACE_DEBUG("%s: UUID %04X attr %04X arg %d op %d\r\n", __FUNCTION__,
        uuid16, attr_handle, (int)arg, ctxt->op);

    switch (uuid16) {

    case GATT_UUID_HID_INFORMATION:
        if (ctxt->op != BLE_GATT_ACCESS_OP_READ_CHR) {
            TLS_BT_APPL_TRACE_WARNING("invalid op %d\r\n", ctxt->op);
            break;
        }
        rc = os_mbuf_append(ctxt->om, HidInfo,
                            HID_INFORMATION_LEN);
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;

    case GATT_UUID_HID_CONTROL_POINT: {
        if (ctxt->op != BLE_GATT_ACCESS_OP_WRITE_CHR) {
            TLS_BT_APPL_TRACE_WARNING("invalid op %d\r\n", ctxt->op);
            break;
        }

        uint8_t new_suspend_state;

        rc = gatt_svr_chr_write(ctxt->om, 1, 1, &new_suspend_state, NULL);
        if (!rc) {
            bool old_state = wm_hid_dev.suspended_state;
            wm_hid_dev.suspended_state = (bool) new_suspend_state;

            TLS_BT_APPL_TRACE_WARNING("HID_CONTROL_POINT received new suspend state: %d, old state is: %d\r\n",
                (int)new_suspend_state, (int)old_state);
        }
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }

    case GATT_UUID_HID_REPORT_MAP: {
        if (ctxt->op != BLE_GATT_ACCESS_OP_READ_CHR) {
            TLS_BT_APPL_TRACE_WARNING("invalid op %d\r\n", ctxt->op);
            break;
        }

        rc = os_mbuf_append(ctxt->om, Hid_report_map, Hid_report_map_size);
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }

    case GATT_UUID_EXT_RPT_REF_DESCR: {
        if (ctxt->op != BLE_GATT_ACCESS_OP_READ_DSC) {
            TLS_BT_APPL_TRACE_WARNING("invalid op %d\r\n", ctxt->op);
            break;
        }

        rc = os_mbuf_append(ctxt->om, (uint8_t *)&HidExtReportRefDesc,
            sizeof(HidExtReportRefDesc));
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }

    case GATT_UUID_HID_PROTO_MODE: {

        if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {

            rc = os_mbuf_append(ctxt->om, &HidProtocolMode,
                                sizeof(HidProtocolMode));
            return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;

        } else if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {

            uint8_t new_protocol_mode;

            rc = gatt_svr_chr_write(ctxt->om, 1, sizeof(new_protocol_mode),
                &new_protocol_mode, NULL);
            if (!rc) {
                HidProtocolMode = new_protocol_mode;
                // send true if new mode is boot mode, else guess
                wm_hid_dev.report_mode_boot = (HidProtocolMode == HID_PROTOCOL_MODE_BOOT);
                TLS_BT_APPL_TRACE_DEBUG("Received new protocol mode: %d\r\n",
                    (int)new_protocol_mode);
            }

            return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
        } else {
            TLS_BT_APPL_TRACE_WARNING("invalid op %d\r\n", ctxt->op);
        }
        return BLE_ATT_ERR_INSUFFICIENT_RES;
    }

    default:
        TLS_BT_APPL_TRACE_WARNING("invalid UUID %02X\r\n", uuid16);
        break;
    }
    return BLE_ATT_ERR_UNLIKELY;
}

/* Report access function for all reports */

static int
ble_svc_report_access(uint16_t conn_handle, uint16_t attr_handle,
                             struct ble_gatt_access_ctxt *ctxt,
                             void *arg)
{
    uint16_t uuid16 = ble_uuid_u16(ctxt->chr->uuid);
    int handle_num = (int) arg;
    int rc = BLE_ATT_ERR_UNLIKELY;

    TLS_BT_APPL_TRACE_DEBUG( "%s: UUID %04X attr %04X arg %d op %d\r\n",
         __FUNCTION__, uuid16, attr_handle, (int)arg, ctxt->op);


    do {
        // Report reference descriptors
        if (uuid16 == GATT_UUID_RPT_REF_DESCR) {
            if (ctxt->op != BLE_GATT_ACCESS_OP_READ_DSC) {
                TLS_BT_APPL_TRACE_API( "invalid op %d\r\n", ctxt->op);
                break;
            }
            int rpt_ind = -1;

            for (int i = 0; i < Hid_report_ref_data_count; ++i) {
                if ((int)Hid_report_ref_data[i].id == handle_num) {
                    rpt_ind = i;
                    break;
                }
            }
            if (rpt_ind != -1) {
                rc = os_mbuf_append(ctxt->om,
                    (uint8_t *)Hid_report_ref_data[rpt_ind].hidReportRef,
                    HID_REPORT_REF_LEN);
                if (rc) {
                    rc = BLE_ATT_ERR_INSUFFICIENT_RES;
                }
            } else {
                rc = BLE_ATT_ERR_UNLIKELY;
            }

            break;
        }

        /* reports read */
        if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR && (
                (uuid16 == GATT_UUID_HID_REPORT)            ||
                (uuid16 == GATT_UUID_HID_BT_MOUSE_INPUT)    ||
                (uuid16 == GATT_UUID_HID_BT_KB_INPUT)       ||
                (uuid16 == GATT_UUID_HID_BT_KB_OUTPUT)      )) {
            rc = hid_read_buffer(ctxt->om, handle_num);
            if (rc) {
                rc = BLE_ATT_ERR_INSUFFICIENT_RES;
            }
            break;
        }

        // keyboard out report (leds in/out)
        if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
            switch (handle_num) {
                case HANDLE_HID_KB_OUT_REPORT:
                case HANDLE_HID_FEATURE_REPORT:
                    rc = hid_write_buffer(ctxt->om, handle_num);
                    if (rc) {
                        rc = BLE_ATT_ERR_INSUFFICIENT_RES;
                    }
                    break;
                default:
                    rc = BLE_ATT_ERR_INSUFFICIENT_RES;
            }
            break;
        }

    } while (0);

    return rc;
}

/**
 * BAS access function
 */
static int
ble_svc_battery_access(uint16_t conn_handle, uint16_t attr_handle,
                          struct ble_gatt_access_ctxt *ctxt,
                          void *arg)
{
    uint16_t uuid16 = ble_uuid_u16(ctxt->chr->uuid);
    int rc = 0;

    TLS_BT_APPL_TRACE_DEBUG( "%s: UUID %04X attr %04X arg %d op %d\r\n",
         __FUNCTION__, uuid16, attr_handle, (int) arg, ctxt->op);

    switch (uuid16) {
        case BLE_SVC_BAS_CHR_UUID16_BATTERY_LEVEL:
            if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
                rc = hid_read_buffer(ctxt->om, (int) arg);
                // rc = hid_battery_level_get(ctxt->om);
                if (rc) {
                    TLS_BT_APPL_TRACE_WARNING( "Error reading battery buffer, rc = %d\r\n", rc);
                    rc = BLE_ATT_ERR_INSUFFICIENT_RES;
                }
            } else {
                rc = BLE_ATT_ERR_UNLIKELY;
            }
            break;

        case GATT_UUID_BAT_PRESENT_DESCR:
            if (ctxt->op == BLE_GATT_ACCESS_OP_READ_DSC) {
                TLS_BT_APPL_TRACE_API( "battery character presentation descriptor read, op: %d\r\n", ctxt->op);
                rc = os_mbuf_append(ctxt->om, &Battery_level_units,
                                    sizeof Battery_level_units);

                if (rc) {
                    rc = BLE_ATT_ERR_INSUFFICIENT_RES;
                }
            }
            break;

        default:
            rc = BLE_ATT_ERR_UNLIKELY;
    }

    return rc;
}

/**
 * Simple read access callback for the device information service
 * characteristic.
 */
static int
ble_svc_dis_access(uint16_t conn_handle, uint16_t attr_handle,
                   struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    uint16_t uuid    = ble_uuid_u16(ctxt->chr->uuid);
    const char *info = NULL;
    int rc = 0;
    int data_len = 0;

    TLS_BT_APPL_TRACE_DEBUG( "%s: UUID %04X attr %04X arg %d op %d\r\n",
         __FUNCTION__, uuid, attr_handle, (int)arg, ctxt->op);

    switch(uuid) {
    case BLE_SVC_DIS_CHR_UUID16_MODEL_NUMBER:
        info = Hid_dis_data.model_number;
        if (info == NULL) {
            info = (BLE_SVC_DIS_MODEL_NUMBER_DEFAULT);
        }
        break;
    case BLE_SVC_DIS_CHR_UUID16_SERIAL_NUMBER:
        info = Hid_dis_data.serial_number;
        if (info == NULL) {
            info = (BLE_SVC_DIS_SERIAL_NUMBER_DEFAULT);
        }
        break;
    case BLE_SVC_DIS_CHR_UUID16_FIRMWARE_REVISION:
        info = Hid_dis_data.firmware_revision;
        if (info == NULL) {
            info = (BLE_SVC_DIS_FIRMWARE_REVISION_DEFAULT);
        }
        break;
    case BLE_SVC_DIS_CHR_UUID16_HARDWARE_REVISION:
        info = Hid_dis_data.hardware_revision;
        if (info == NULL) {
            info = (BLE_SVC_DIS_HARDWARE_REVISION_DEFAULT);
        }
        break;
    case BLE_SVC_DIS_CHR_UUID16_SOFTWARE_REVISION:
        info = Hid_dis_data.software_revision;
        if (info == NULL) {
            info = (BLE_SVC_DIS_SOFTWARE_REVISION_DEFAULT);
        }
        break;
    case BLE_SVC_DIS_CHR_UUID16_MANUFACTURER_NAME:
        info = Hid_dis_data.manufacturer_name;
        if (info == NULL) {
            info = (BLE_SVC_DIS_MANUFACTURER_NAME_DEFAULT);
        }
        break;
    case BLE_SVC_DIS_CHR_UUID16_SYSTEM_ID:
        info = Hid_dis_data.system_id;
        if (info == NULL) {
            info = (BLE_SVC_DIS_SYSTEM_ID_DEFAULT);
        }
        break;
    case BLE_SVC_DIS_CHR_UUID16_PNP_INFO:

        info = (char *)Hid_dis_data.pnp_info;
        data_len = sizeof(Hid_dis_data.pnp_info);
        if (info == NULL) {
            info = BLE_SVC_DIS_SYSTEM_ID_DEFAULT;
        }
        break;
    default:
        rc = BLE_ATT_ERR_UNLIKELY;
    }

    if (!data_len) {
        data_len =  strlen(info);
    }

    if (info != NULL) {
        rc = os_mbuf_append(ctxt->om, info, data_len);
        if (rc) {
            rc = BLE_ATT_ERR_INSUFFICIENT_RES;
        }
    }

    return rc;
}


#define BREAK_IF_NOT_ZERO(EXPR2TEST) if ((EXPR2TEST)!= 0) break

static int
gatt_svr_init()
{
    int rc = 0;

    //ble_svc_gap_init();
    //ble_svc_gatt_init();

    memset(&Svc_char_handles, 0, sizeof(Svc_char_handles[0]) * HANDLE_HID_COUNT);

    do {
        BREAK_IF_NOT_ZERO( rc = ble_gatts_count_cfg(Gatt_svr_included_services) );

        BREAK_IF_NOT_ZERO( rc = ble_gatts_add_svcs(Gatt_svr_included_services) );

        TLS_BT_APPL_TRACE_API( "GATT included services added\r\n");

        BREAK_IF_NOT_ZERO( rc = ble_gatts_count_cfg(Gatt_svr_svcs) );

        BREAK_IF_NOT_ZERO( rc = ble_gatts_add_svcs(Gatt_svr_svcs) );

        TLS_BT_APPL_TRACE_API( "GATT root services added\r\n");

    } while (0);

    return rc;
}

/**
 * Enables advertising with the following parameters:
 *     o General discoverable mode.
 *     o Undirected connectable mode.
 */
static void
hid_advertise(ble_addr_t *direct_addr)
{
    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;
    //const char *name;
    int rc;

    /**
     *  Set the advertisement data included in our advertisements:
     *     o Flags (indicates advertisement type and other general info).
     *     o Advertising tx power.
     *     o Device name.
     *     o 16-bit service UUIDs (alert notifications).
     */

    memset(&fields, 0, sizeof fields);

    /* Advertise two flags:
     *     o Discoverability in forthcoming advertisement (general)
     *     o BLE-only (BR/EDR unsupported).
     */
    fields.flags = BLE_HS_ADV_F_DISC_GEN |
                   BLE_HS_ADV_F_BREDR_UNSUP;

    /* Indicate that the TX power level field should be included; have the
     * stack fill this value automatically.  This is done by assigning the
     * special value BLE_HS_ADV_TX_PWR_LVL_AUTO.
     */
    fields.tx_pwr_lvl_is_present = 1;
    fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

    fields.adv_itvl_is_present = 1;
    fields.adv_itvl = 40;
#if 0
    name = ble_svc_gap_device_name();
    fields.name = (uint8_t *)name;
    fields.name_len = strlen(name);
    fields.name_is_complete = 1;
#endif
    fields.appearance = HID_KEYBOARD_APPEARENCE;
    fields.appearance_is_present = 1;

    fields.uuids16 = (ble_uuid16_t[]) {
        BLE_UUID16_INIT(GATT_UUID_HID_SERVICE)
    };
    fields.num_uuids16 = 1;
    fields.uuids16_is_complete = 1;

    uint8_t buf[50], buf_sz;
    rc = ble_hs_adv_set_fields(&fields, buf, &buf_sz, 50);

    if (rc != 0) {
        TLS_BT_APPL_TRACE_ERROR( "error setting advertisement data to buf; rc=%d\r\n", rc);
        return;
    }
    if (buf_sz > BLE_HS_ADV_MAX_SZ) {
        TLS_BT_APPL_TRACE_ERROR( "Too long advertising data: appearance %x, uuid16 %x, advsize = %d\r\n",
            fields.appearance, GATT_UUID_HID_SERVICE, buf_sz);
        //ble_hs_adv_parse(buf, buf_sz, user_parse, NULL);
        return;
    }

    rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0) {
        TLS_BT_APPL_TRACE_ERROR( "error setting advertisement data; rc=%d\r\n", rc);
        return;
    }

    /*Device Name*/
	uint8_t bt_mac[6];
    uint8_t adv_rsp_data[31] = {30,0x09, 0x00};
    extern int tls_get_bt_mac_addr(uint8_t *mac);
    
    tls_get_bt_mac_addr(bt_mac);
    rc = sprintf(adv_rsp_data+2,"openHarmony_Keyboard-%02X:%02X:%02X",bt_mac[3], bt_mac[4], bt_mac[5]);
    
    rc = tls_ble_gap_set_data(WM_BLE_ADV_RSP_DATA, adv_rsp_data, 31);

    /* Begin advertising. */
    if(direct_addr == NULL)
    {
        rc = tls_nimble_gap_adv(WM_BLE_ADV_IND, NULL);
    }else
    {
        rc = tls_nimble_gap_adv(WM_BLE_ADV_DIRECT_IND_HDC, direct_addr);
    }

    if (rc != 0) {
        TLS_BT_APPL_TRACE_ERROR( "error enabling advertisement; rc=%d\r\n", rc);
        return;
    }
}

static void hid_conn_param_update_cb(uint16_t conn_handle, int status, void *arg)
{
	TLS_BT_APPL_TRACE_DEBUG("conn param update complete; conn_handle=%d status=%d\n",
		       conn_handle, status);
}

static void wm_ble_server_hid_conn_param_update_slave()
{
    return 0;
	int rc;
	struct ble_l2cap_sig_update_params params;

	params.itvl_min = 0x0010;
	params.itvl_max = 0x0012;
	params.slave_latency = 0;
	params.timeout_multiplier = 0x02d0;

	rc = ble_l2cap_sig_update(g_ble_demo_conn_handle, &params,
		hid_conn_param_update_cb, NULL);
	assert(rc == 0);
}

static void wm_ble_server_hid_check_permission(void *arg)
{
    int rc = -1;
    ble_addr_t *addr = (ble_addr_t *)arg;

    TLS_BT_APPL_TRACE_DEBUG("verify addr: type=%d, %02x:%02x:%02x:%02x:%02x:%02x\r\n", addr->type, addr->val[0], addr->val[1],addr->val[2],addr->val[3],addr->val[4],addr->val[5]);
    rc = check_hid_dev_black_white_list(addr);

    if(rc != 1)
    {
        if(addr->type == 0)update_hid_dev_state(BLE_GAP_EVENT_CONNECT, addr);
    }
            
    TLS_BT_APPL_TRACE_DEBUG("black white list check result=%s(%d)\r\n", rc==1?"decline":"accept", rc);
}
/**
 * The nimble host executes this callback when a GAP event occurs.  The
 * application associates a GAP event callback with each connection that forms.
 * bleprph uses the same callback for all connections.
 *
 * @param event                 The type of event being signalled.
 * @param arg                   Application-specified argument; unused by
 *                                  bleprph.
 *
 * @return                      0 if the application successfully handled the
 *                                  event; nonzero on failure.  The semantics
 *                                  of the return code is specific to the
 *                                  particular GAP event being signalled.
 */


static int
hid_gap_evt_cb(struct ble_gap_event *event, void *arg)
{
    struct ble_gap_conn_desc desc;
    int rc;

    switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:
        /* A new connection was established or a connection attempt failed. */
        TLS_BT_APPL_TRACE_API( "connection %s; status=%d \r\n",
                    event->connect.status == 0 ? "established" : "failed",
                    event->connect.status);
        if (event->connect.status == 0) {
            rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
            assert(rc == 0);
            //bleprph_print_conn_desc(&desc);
            printf("our id addr(%d): 0x%02x:%02x:%02x:%02x:%02x:%02x\r\n", desc.our_id_addr.type, desc.our_id_addr.val[0],desc.our_id_addr.val[1],desc.our_id_addr.val[2],desc.our_id_addr.val[3],desc.our_id_addr.val[4],desc.our_id_addr.val[5]);
            printf("our oa addr(%d): 0x%02x:%02x:%02x:%02x:%02x:%02x\r\n", desc.our_ota_addr.type, desc.our_ota_addr.val[0],desc.our_ota_addr.val[1],desc.our_ota_addr.val[2],desc.our_ota_addr.val[3],desc.our_ota_addr.val[4],desc.our_ota_addr.val[5]);
            printf("per id addr(%d): 0x%02x:%02x:%02x:%02x:%02x:%02x\r\n", desc.peer_id_addr.type, desc.peer_id_addr.val[0],desc.peer_id_addr.val[1],desc.peer_id_addr.val[2],desc.peer_id_addr.val[3],desc.peer_id_addr.val[4],desc.peer_id_addr.val[5]);
            printf("per oa addr(%d): 0x%02x:%02x:%02x:%02x:%02x:%02x\r\n", desc.peer_ota_addr.type, desc.peer_ota_addr.val[0],desc.peer_ota_addr.val[1],desc.peer_ota_addr.val[2],desc.peer_ota_addr.val[3],desc.peer_ota_addr.val[4],desc.peer_ota_addr.val[5]);

            g_ble_demo_conn_handle = event->connect.conn_handle;
            wm_hid_dev.conn_handle = event->connect.conn_handle;
            wm_hid_dev.connected = true;
            g_ble_demo_hid_connected = 1;
            //hid_clean_vars(&desc);
            if(desc.peer_id_addr.type == 0)update_hid_dev_state(BLE_GAP_EVENT_CONNECT, &desc.peer_id_addr);
            //tls_bt_async_proc_func(wm_ble_server_hid_conn_param_update_slave, NULL, 20);
            //memcpy(&g_ble_remote_addr, &desc.peer_id_addr, sizeof(ble_addr_t));
            //tls_bt_async_proc_func(wm_ble_server_hid_check_permission, (void*)&g_ble_remote_addr, 20);
        } else {
            /* Connection failed; resume advertising. */
            hid_advertise(NULL);
            g_ble_demo_hid_connected = 0;
        }
        return 0;

    case BLE_GAP_EVENT_DISCONNECT:
        TLS_BT_APPL_TRACE_API( "disconnect; reason=%d \r\n", event->disconnect.reason);
        wm_hid_dev.connected = false;
        g_ble_demo_hid_connected = 0;

        

        /* Connection terminated; resume advertising. */
        /*if remote deviced disconnect with us normally, we do not start advertising, excecpt ctrl+shift++ detected!!!*/
        //if(event->disconnect.reason != 0x213  && event->disconnect.reason != 0x216)
        {
            //hid_advertise(NULL);
        }

        if(event->disconnect.reason != 0x216)
        {
            hid_advertise(NULL);
        }else
        {
            update_hid_dev_state(BLE_GAP_EVENT_DISCONNECT, &event->disconnect.conn.peer_id_addr); 
        }
        
        if(g_ble_disconnect_and_continue_to_adv)
        {
            g_ble_disconnect_and_continue_to_adv = false;
            hid_advertise(NULL); 
        }
        return 0;

    case BLE_GAP_EVENT_CONN_UPDATE_REQ:
        /* The central requested the connection parameters update. */
        TLS_BT_APPL_TRACE_API( "connection update request\r\n");
        return 0;

    case BLE_GAP_EVENT_CONN_UPDATE:
        /* The central has updated the connection parameters. */
        TLS_BT_APPL_TRACE_API( "connection updated; status=%d \r\n",
                    event->conn_update.status);
        return 0;

    case BLE_GAP_EVENT_ADV_COMPLETE:
        TLS_BT_APPL_TRACE_API( "advertise complete; reason=%d\r\n",
                    event->adv_complete.reason);
        //hid_advertise(NULL);
        return 0;
    case BLE_GAP_EVENT_IDENTITY_RESOLVED:
        TLS_BT_APPL_TRACE_API( "identity address resolved; handle=%d\r\n",
                    event->identity_resolved.conn_handle);
        rc = ble_gap_conn_find(event->identity_resolved.conn_handle, &desc);
        assert(rc == 0);
        //bleprph_print_conn_desc(&desc);
        printf("our id addr(%d): 0x%02x:%02x:%02x:%02x:%02x:%02x\r\n", desc.our_id_addr.type, desc.our_id_addr.val[0],desc.our_id_addr.val[1],desc.our_id_addr.val[2],desc.our_id_addr.val[3],desc.our_id_addr.val[4],desc.our_id_addr.val[5]);
        printf("our oa addr(%d): 0x%02x:%02x:%02x:%02x:%02x:%02x\r\n", desc.our_ota_addr.type, desc.our_ota_addr.val[0],desc.our_ota_addr.val[1],desc.our_ota_addr.val[2],desc.our_ota_addr.val[3],desc.our_ota_addr.val[4],desc.our_ota_addr.val[5]);
        printf("per id addr(%d): 0x%02x:%02x:%02x:%02x:%02x:%02x\r\n", desc.peer_id_addr.type, desc.peer_id_addr.val[0],desc.peer_id_addr.val[1],desc.peer_id_addr.val[2],desc.peer_id_addr.val[3],desc.peer_id_addr.val[4],desc.peer_id_addr.val[5]);
        printf("per oa addr(%d): 0x%02x:%02x:%02x:%02x:%02x:%02x\r\n", desc.peer_ota_addr.type, desc.peer_ota_addr.val[0],desc.peer_ota_addr.val[1],desc.peer_ota_addr.val[2],desc.peer_ota_addr.val[3],desc.peer_ota_addr.val[4],desc.peer_ota_addr.val[5]);
        //memcpy(&g_ble_remote_addr, &desc.peer_id_addr, sizeof(ble_addr_t));
        //tls_bt_async_proc_func(wm_ble_server_hid_check_permission, (void*)&g_ble_remote_addr, 20);
        if(desc.peer_id_addr.type == 0)update_hid_dev_state(BLE_GAP_EVENT_CONNECT, &desc.peer_id_addr);
        return 0;
    case BLE_GAP_EVENT_ENC_CHANGE:
        /* Encryption has been enabled or disabled for this connection. */
        TLS_BT_APPL_TRACE_API( "encryption change event; status=%d \r\n",
                    event->enc_change.status);
        rc = ble_gap_conn_find(event->enc_change.conn_handle, &desc);
        assert(rc == 0);
        //bleprph_print_conn_desc(&desc);       
        return 0;

    case BLE_GAP_EVENT_SUBSCRIBE:
        TLS_BT_APPL_TRACE_API( "subscribe event; conn_handle=%d attr_handle=%04X "
                    "reason=%d prev_notify=%d cur_notify=%d prev_indicate=%d cur_indicate=%d\r\n",
                    event->subscribe.conn_handle,
                    event->subscribe.attr_handle,
                    event->subscribe.reason,
                    event->subscribe.prev_notify,
                    event->subscribe.cur_notify,
                    event->subscribe.prev_indicate,
                    event->subscribe.cur_indicate);

        hid_set_notify(event->subscribe.attr_handle,
            event->subscribe.cur_notify,
            event->subscribe.cur_indicate);
        return 0;

    case BLE_GAP_EVENT_NOTIFY_TX:
        TLS_BT_APPL_TRACE_API( "notify event; status=%d conn_handle=%d attr_handle=%04X type=%s\r\n",
                    event->notify_tx.status,
                    event->notify_tx.conn_handle,
                    event->notify_tx.attr_handle,
                    event->notify_tx.indication?"indicate":"notify");
        return 0;

    case BLE_GAP_EVENT_MTU:
        TLS_BT_APPL_TRACE_API( "mtu update event; conn_handle=%d cid=%d mtu=%d\r\n",
                    event->mtu.conn_handle,
                    event->mtu.channel_id,
                    event->mtu.value);
        return 0;

    case BLE_GAP_EVENT_REPEAT_PAIRING:
            /* We already have a bond with the peer, but it is attempting to
            * establish a new secure link.  This app sacrifices security for
            * convenience: just throw away the old bond and accept the new link.
            */
         /* Delete the old bond. */
        rc = ble_gap_conn_find(event->repeat_pairing.conn_handle, &desc);
        assert(rc == 0);
        ble_store_util_delete_peer(&desc.peer_id_addr);
            
        TLS_BT_APPL_TRACE_DEBUG("!!!BLE_GAP_EVENT_REPEAT_PAIRING\r\n");

        return BLE_GAP_REPEAT_PAIRING_RETRY;

    case BLE_GAP_EVENT_PASSKEY_ACTION:
        TLS_BT_APPL_TRACE_DEBUG(">>>BLE_GAP_EVENT_REPEAT_PAIRING\r\n");
        return 0;

    default:
        TLS_BT_APPL_TRACE_API( "Unknown GAP event: %d\r\n", event->type);
    }

    return 0;
}

/*
 * EXPORTED FUNCTION DEFINITIONS
 ****************************************************************************************
 */
int tls_ble_server_demo_hid_advertise(uint8_t state , void *direct_addr)
{
    if(state)
    {
        hid_advertise((ble_addr_t *)direct_addr);
    }else
    {
       tls_nimble_gap_adv(WM_BLE_ADV_STOP,NULL); 
    }
}
uint8_t tls_ble_server_demo_hid_connected()
{
    return g_ble_demo_hid_connected;
}
int tls_ble_server_demo_hid_disconnect(bool continue_to_adv)
{
     g_ble_disconnect_and_continue_to_adv = continue_to_adv;
     if(g_ble_demo_hid_connected)
     {
        TLS_BT_APPL_TRACE_DEBUG("### hid service disconnect with %02x:%02x:%02x:%02x:%02x:%02x\r\n",g_ble_remote_addr.val[0],
            g_ble_remote_addr.val[1],g_ble_remote_addr.val[2],g_ble_remote_addr.val[3],g_ble_remote_addr.val[4],g_ble_remote_addr.val[5]);
        ble_gap_terminate(g_ble_demo_conn_handle, BLE_ERR_REM_USER_CONN_TERM);
     }
}

int tls_ble_server_demo_hid_mouse_notify(uint8_t *mouse_array, int length)
{
    int rc = 0;
    assert(length == HIDD_LE_REPORT_MOUSE_SIZE);
    if(g_ble_demo_hid_connected == 0) return rc;

    if (lock_hid_data() == 0) {
        
        memcpy(Mouse_buffer, mouse_array, length);
        unlock_hid_data();
        
        if (rc == 0) {
            rc = hid_send_report(HANDLE_HID_MOUSE_REPORT);
        }
    } else {
        rc = 1;
    }

    return rc;
}
int tls_ble_server_demo_hid_keyboard_cc_notify(uint8_t *key_array, int length)
{
    int rc = 0;
    assert(length == HIDD_LE_REPORT_CC_SIZE);
    if(g_ble_demo_hid_connected == 0) return rc;

    if (lock_hid_data() == 0) {
        memcpy(CC_buffer, key_array, length);
        unlock_hid_data();

        if (rc == 0) {
            rc = hid_send_report(HANDLE_HID_CC_REPORT);
        }
    } else {
        rc = 2;
    }

    return rc;
}



int tls_ble_server_demo_hid_keyboard_notify(uint8_t *key_array, int length)
{
    int i = 0;
    int rc = 0;
    assert(length == HIDD_LE_REPORT_KB_IN_SIZE);
    if(g_ble_demo_hid_connected == 0) return rc;

    if (lock_hid_data() == 0) {
        memcpy(Keyboard_buffer, key_array, length);
        unlock_hid_data();
        if (rc == 0) {
            rc = hid_send_report(HANDLE_HID_KB_IN_REPORT);
        }
    } else {
        rc = 2;
    }

    return rc;
}


int tls_ble_server_demo_hid_init()
{
    int rc = 0;
    TLS_BT_APPL_TRACE_DEBUG("### tls_ble_server_demo_hid_init \r\n");

    if(bt_adapter_state == WM_BT_STATE_OFF)
    {
        TLS_BT_APPL_TRACE_ERROR("%s failed rc=%s\r\n", __FUNCTION__, tls_bt_rc_2_str(BLE_HS_EDISABLED));
        return BLE_HS_EDISABLED;
    }
   
    if(g_ble_demo_hid_inited == 0)
    {
        g_ble_demo_hid_connected = 0;

        hid_clean_vars();

        //step 0: reset other services. Note 
        rc = ble_gatts_reset();
        if(rc != 0)
        {
            TLS_BT_APPL_TRACE_ERROR("tls_ble_server_demo_hid_init failed rc=%d\r\n", rc);
            return rc;
        }
        TLS_BT_APPL_TRACE_DEBUG("### gatt_svr_init\r\n");

        //step 1: config/adding  the services
        rc = gatt_svr_init();
        TLS_BT_APPL_TRACE_DEBUG("### gatt_svr_inited\r\n");

		if(rc == 0)
		{	
		    tls_ble_register_gap_evt(0xFFFFFFFF, hid_gap_evt_cb);
			TLS_BT_APPL_TRACE_DEBUG("### tls_ble_server_demo_hid_inited \r\n");
            
            /*step 2: start the service*/
            rc = ble_gatts_start();
            assert(rc == 0);

            /*step 3: start advertisement*/
            //hid_advertise(NULL); 
            
            g_ble_demo_hid_inited = 1;
		}else
		{
			TLS_BT_APPL_TRACE_ERROR("### tls_ble_server_demo_hid_init failed(rc=%d)\r\n", rc);
		}
    }
    else
    {
    	TLS_BT_APPL_TRACE_WARNING("tls_ble_server_demo_hid_init registered\r\n");
        rc = BLE_HS_EALREADY;
    }
	
	return rc;
}
int tls_ble_server_demo_hid_deinit()
{
	int rc = 0;

    if(g_ble_demo_hid_inited)
    {
       if(g_ble_demo_hid_connected)
       {
            
            rc = ble_gap_terminate(g_ble_demo_conn_handle, BLE_ERR_REM_USER_CONN_TERM);
       }else
       {
            rc = tls_nimble_gap_adv(WM_BLE_ADV_STOP, NULL);
       }

       g_ble_demo_hid_inited = 0;

    }else
    {
        rc = BLE_HS_EALREADY;
    }
    
    return rc;
}


#endif
