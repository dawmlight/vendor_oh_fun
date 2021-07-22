/*
 * Copyright (c) 2020 Nanjing Xiaoxiongpai Intelligent Technology Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "iot_cloud_api.h"
#include "iot_hardware_api.h"
#include "ohos_init.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <cmsis_os2.h>

// #######ATTENSIONS########
// DEFAULT PARAM, AND MUST BE MODIFIED TO THE REAL PARAMS
#define CONFIG_WIFI_DEFAULT_SSID "Hold"                                     // replace it with the real ssid
#define CONFIG_WIFI_DEFAULT_PWD "0987654321"                                // replace it with the real pwd
#define CONFIG_CLOUD_DEFAULT_SERVERIP "121.36.42.100"                       // replace it with iotplatform server ip
#define CONFIG_CLOUD_DEFAULT_SERVERPORT "1883"                              // replace it with iotplatform server  port
#define CONFIG_CLOUD_DEFAULT_DEVICEID "60a26c6606dc9602c02f721c_2132132131" // replace it with real deviceID
#define CONFIG_CLOUD_DEFAULT_DEVICEPWD "123456789"                          // replace it with real pwd
#define CONFIG_MOTOR_TIMELIMIT 4000                                         // unit:minisecond
#define CONFIG_SENSOR_SAMPLE_CYCLE 3000                                     // unit:minisecond
#define CONFIG_LED_FLASH_BASECYCLE 1000                                     // unit:minisecond
#define CONFIG_TASK_DEFAULT_STACKSIZE 0x1000                                // unit:bytes
#define CONFIG_TASK_DEFAULT_PRIOR 25                                        // task priority
#define CONFIG_TASK_MAIN_STACKSIZE 0x4000                                   // main task stacksize must be bigger
#define CN_MINISECONDS_IN_SECOND 1000                                       // mini seconds in second
#define CN_LED_FLASH_STATUSNUM 2                                            // LED flash:on/off
static osTimerId_t g_motorMonitorTimer;                                     // Make a timer to monitor the motor status
static osEventFlagsId_t g_sensorTaskEvent;                                  // if you want to the task
#define CN_SENSOREVENT_MOTORSTOP (1<<0)                                     // report the motor status
/**
 * @brief change miniseconds to ticks
 * @param ms, the mimiseconds to translate
 * @return the corresponding ticks of the time
 */
static uint32_t Time2Tick(uint32_t ms)
{
    uint64_t ret;
    ret = ((uint64_t)ms * osKernelGetTickFreq()) / CN_MINISECONDS_IN_SECOND;
    return (uint32_t)ret;
}

typedef enum {
    IOTMAIN_STATUS_CONNECTWIFI = 1,
    IOTMAIN_STATUS_CONNECTCLOUD = 2,
    IOTMAIN_STATUS_WORKSERR = 4,
    IOTMAIN_STATUS_WORKSOK = 16,
}IoTMainStatus;

static int g_iotMainStatus = IOTMAIN_STATUS_CONNECTWIFI;

/**
 * @brief this is the LED FLASH task entry
 *
 */
static void LedTaskEntry(void *arg)
{
    (void *)arg;
    uint32_t baseTicks;
    uint32_t delayTicks;
    int status = CN_BOARD_SWITCH_ON;

    baseTicks = Time2Tick(CONFIG_LED_FLASH_BASECYCLE);
    while (1) {
        delayTicks = g_iotMainStatus * baseTicks / CN_LED_FLASH_STATUSNUM;
        Board_SetLedStatus(status);
        osDelay(delayTicks);
        status = !status;
    }
    return;
}

/**
 * @brief This is the Sensor task entry, sample the data and report to the cloud
 *
 */
static void SensorTaskEntry(void *arg)
{
    int ret, temperature, humidity, moisture, motorStatus;
    int *temperatureBuf = NULL;
    int *humidityBuf = NULL;
    int *moistureBuf = NULL;
    int *motorStatusBuf = NULL;
    uint32_t sensorEvent;
    (void *)arg;
    /*
    1, sample the humidity and temperature
    2, sample the soil moisture
    3, report to the cloud
    */
    while (1) {
        temperatureBuf = NULL;
        humidityBuf = NULL;
        moistureBuf = NULL;
        motorStatusBuf = NULL;

        sensorEvent = osEventFlagsWait(g_sensorTaskEvent, CN_SENSOREVENT_MOTORSTOP, osFlagsWaitAny, Time2Tick(CONFIG_SENSOR_SAMPLE_CYCLE));
        if (sensorEvent & CN_SENSOREVENT_MOTORSTOP) {
            motorStatus = CN_BOARD_SWITCH_OFF;
            motorStatusBuf = &motorStatus;
        }

        ret = Board_GetAirStatus(&temperature, &humidity);
        if (ret == 0) {
            temperatureBuf = &temperature;
            humidityBuf = &humidity;
            printf("AIRSENSOR:temperature:%d humidity:%d \r\n", temperature, humidity);
        } else {
            printf("AIRSENSOR:sample failed \r\n");
        }

        ret = Board_GetSoilStatus(&moisture);
        if (ret == 0) {
            moistureBuf = &moisture;
            printf("SOILSENSOR:moisture:%d \r\n", moisture);
        } else {
            printf("SOILSENSOR:sample failed \r\n");
        }

        ret = Cloud_ReportMsg(temperatureBuf, humidityBuf, moistureBuf, motorStatusBuf);
        if (ret != 0) {
            printf("CLOUD:sensor report failed");
        }
    }

    return;
}

/**
 * @brief This function used to deal with cloud command
 *
 * @param command, defines as Cloud_CommandType
 * @param value, corresponding to command
 * @return 0 success while -1 failed as defines
*/
static int CommandCallBack(int command, int value)
{
    int ret = -1;
    Cloud_CommandType cmd = (Cloud_CommandType)command;
    switch (cmd) {
        case CLOUD_COMMAND_CONTROLMOTOR:
            if (value) {
                osTimerStart(g_motorMonitorTimer, Time2Tick(CONFIG_MOTOR_TIMELIMIT));
            }
            ret = Board_SetMotorStatus(value);
            break;
        case CLOUD_COMMAND_SETTHRETHOLD:
            break;
        default:
            break;
    }

    return ret;
}

/**
 * @brief This is the motor monitor, when timeout we must set the motor off and report
 *
 */

void MotorMonitorTimerCallBack(void *arg)
{
    int status = CN_BOARD_SWITCH_OFF;

    (void *)arg;
    osTimerStop (g_motorMonitorTimer);
    Board_SetMotorStatus(status);
    osEventFlagsSet(g_sensorTaskEvent, CN_SENSOREVENT_MOTORSTOP);
}

/**
 * @brief this is the main task entry
 *
 */
static void IotMainTaskEntry(void *arg)
{
    int ret;
    osThreadId_t ledThreadId, sensorThreadId;
    const char *wifiSSID = CONFIG_WIFI_DEFAULT_SSID;
    const char *wifiPWD = CONFIG_WIFI_DEFAULT_PWD;
    const char *deviceID = CONFIG_CLOUD_DEFAULT_DEVICEID;
    const char *devicePWD = CONFIG_CLOUD_DEFAULT_DEVICEPWD;
    const char *serverPORT = CONFIG_CLOUD_DEFAULT_SERVERPORT;
    const char *serverIP = CONFIG_CLOUD_DEFAULT_SERVERIP;

    (void *)arg;

    printf("=======================================\r\n");
    printf("************IoT Flower Main************\r\n");
    printf("=======================================\r\n");
    /*
     * 1, init the necessary hardware and create the neccessary resources
     * 2, get the info from the nfc, if not success, we will use the default
     * 3, create the led task, this task will represent the status of the mainTask
     * 4, connect the wifi
     * 5, connect the iot platform
     * 6, create the  sensor task
    */
    printf("%s:Initialize the hardwares\r\n", __FUNCTION__);
    Board_InitNfc();
    Board_InitLed();
    Board_InitMotor();
    Board_InitAirSensor();
    Board_InitSoilSensor();
    Board_InitWifi();

    // create the resource
    g_motorMonitorTimer = osTimerNew(MotorMonitorTimerCallBack, osTimerPeriodic, NULL, NULL);
    if (g_motorMonitorTimer == NULL) {
        goto ERR_MOTORTIMER;
    }
    g_sensorTaskEvent = osEventFlagsNew(NULL);
    if (g_sensorTaskEvent == NULL) {
        goto ERR_SENSOREVENT;
    }

    // read the configuration from the nfc card
    NfcInfo info;
    ret = Board_GetNfcInfo(&info);
    if (ret == 0) {
        wifiSSID = info.wifiSSID;
        wifiPWD = info.wifiPWD;
        deviceID = info.deviceID;
        devicePWD = info.devicePWD;
    }

    printf("%s:DeviceConfig:wifiSSID:%s wifiPWD:%s deviceID:%s devicePWD:%s\r\n", \
           __FUNCTION__, wifiSSID, wifiPWD, deviceID, devicePWD);
    // create the led task, this task works like the print to notify the status
    osThreadAttr_t attr;
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = CONFIG_TASK_DEFAULT_STACKSIZE;
    attr.priority = CONFIG_TASK_DEFAULT_PRIOR;
    attr.name = "LedFlash";
    if ((ledThreadId = osThreadNew(LedTaskEntry, NULL, &attr)) == NULL) {
        goto ERR_CREATELEDFLASH;
    }

    g_iotMainStatus = IOTMAIN_STATUS_CONNECTWIFI;
    ret = Board_ConnectWifi(wifiSSID, wifiPWD);
    if (ret != 0) {
        printf("%s:wifi connect err \r\n", __FUNCTION__);
        goto ERR_WIFICONNECT;
    }

    g_iotMainStatus = IOTMAIN_STATUS_CONNECTCLOUD;
    ret = Cloud_Init();
    if (ret != 0) {
        printf("%s:cloud sdk init err \r\n", __FUNCTION__);
        goto ERR_CLOUDINIT;
    }
    ret = Cloud_Connect(deviceID, devicePWD, serverIP, serverPORT, CommandCallBack);
    if (ret != 0) {
        printf("%s:cloud connect err \r\n", __FUNCTION__);
        goto ERR_CLOUDCONNECT;
    }

    attr.name = "SensorSample";
    if ((sensorThreadId = osThreadNew(SensorTaskEntry, NULL, &attr)) == NULL) {
        goto ERR_CREATESENSORSAMPLE;
    }
    g_iotMainStatus = IOTMAIN_STATUS_WORKSOK;
    return;

ERR_CREATESENSORSAMPLE:
    Cloud_DisConnect();
ERR_CLOUDCONNECT:
    Cloud_DeInit();
ERR_CLOUDINIT:
    Board_DisConnectWifi();
ERR_WIFICONNECT:
    osThreadTerminate(ledThreadId);
ERR_CREATELEDFLASH:
    osEventFlagsDelete(g_sensorTaskEvent);
    g_sensorTaskEvent = NULL;
ERR_SENSOREVENT:
    osTimerDelete(g_motorMonitorTimer);
    g_motorMonitorTimer = NULL;
ERR_MOTORTIMER:
    g_iotMainStatus = IOTMAIN_STATUS_WORKSERR;
    return;
}

/**
 * @brief this is the main Entry of the IoTFlower project
 *
 */
static void IotMainEntry(void)
{
    osThreadAttr_t attr;
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = CONFIG_TASK_MAIN_STACKSIZE;
    attr.priority = CONFIG_TASK_DEFAULT_PRIOR;

    attr.name = "IoTMain";
    if ((osThreadNew(IotMainTaskEntry, NULL, &attr)) == NULL) {
        printf("Create the IoTMainTask failed\r\n");
    } else {
        printf("Create the IoTMainTask Success\r\n");
    }
    return;
}
// add the IotMainEntry to the system loader
APP_FEATURE_INIT(IotMainEntry);