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
#ifndef __IOT_HARDWARE_API_H__
#define __IOT_HARDWARE_API_H__

#define CN_BOARD_SWITCH_ON    1
#define CN_BOARD_SWITCH_OFF   0

/**
 * @brief init the board wifi
 * @return 0 success while others failed
 */
int BOARD_InitWifi(void);

/**
 * @brief connect the wifi
 * @param wifiSSID, the ssid of the ap
 * @param wifiPWD, the pwd of the ap
 */
int BOARD_ConnectWifi(const char *wifiSSID, const char *wifiPWD);

/**
 * @brief disconnect the wifi
 * @return 0 success while others failed
 */
int BOARD_DisconnectWifi(void);

/**
 * @brief init the board motor
 * @return 0 success while others failed
 */
int BOARD_InitMotor(void);

/**
 * @brief control the motor
 * @param status, has the value of CN_BOARD_SWICT_ON/OFF
 * @return 0 success while others failed
 */
int BOARD_SetMotorStatus(int status);

/**
 * @brief get the motor status
 * @return the status of the motor, 0 means on others off
 */
int BOARD_GetMotorStatus(void);

/**
 * @brief init the board air sensor
 * @return 0 success while others failed
 */
int BOARD_InitAirSensor(void);

/**
 * @brief get the air status
 * @return 0 success while others failed
 */
int BOARD_GetAirStatus(int *temperatur, int *humidity);

/**
 * @brief init the board soil sensor
 * @return 0 success while others failed
 */
int BOARD_InitSoilSensor(void);

/**
 * @brief get the air status
 * @param moisture, store the data
 * @return 0 success while others failed
 */
int BOARD_GetSoilStatus(int *moisture);

/**
 * @brief init the board NFC
 * @return 0 success while others failed
 */
int BOARD_InitNfc(void);

/**
 * @brief defins the nfc information
 *
 */
typedef struct {
    const char *deviceID;
    const char *devicePWD;
    const char *wifiSSID;
    const char *wifiPWD;
}NfcInfo;

/**
 * @brief get the nfc info
 * @param info,if success, the corresponding member will point to the real info, and must not free
 * @return 0 success while others failed
 */
int BOARD_GetNfcInfo(NfcInfo *info);

/**
 * @brief use this function to initialize the led
 *
 */
int BOARD_InitLed(void);

/**
 * @brief use this function to set the led on/off
 * @param status, has the value of CN_BOARD_SWICT_ON/OFF
 * @return 0 success while others failed
*/
int BOARD_SetLedStatus(int status);

#endif /* __IOT_HARDWARE_API_H__ */

