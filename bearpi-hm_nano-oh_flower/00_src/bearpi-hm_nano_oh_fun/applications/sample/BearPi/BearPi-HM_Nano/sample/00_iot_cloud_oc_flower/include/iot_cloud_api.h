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
#ifndef __IOT_CLOUD_API_H__
#define __IOT_CLOUD_API_H__

/**
 * @brief This is the cloud command here
 */
typedef enum {
    CLOUD_COMMAND_SETTHRETHOLD = 0,
    CLOUD_COMMAND_CONTROLMOTOR,
    CLOUD_COMMAND_LAST,
}CLOUD_CommandType;

/**
 * @brief This api will do the cloud initialize
 * @return 0 success while others failed
 */
int CLOUD_Init(void);
/**
 * @brief This api will do the cloud deinitialize
 * @return 0 success while others failed
 */
int CLOUD_Deinit(void);

/**
 * @brief This is the clould call back module, if any message comes, then the function will be called
 */
typedef int (*CLOUD_CommandCallBack)(int command, int value);

/**
 * @brief This api will send a meesage to the iot platform
 * @param airTemperature, the current local temperature, if NULL, will not update
 * @param airHumidity, the current local humiditym, if NULL, will not update
 * @param soilMoisture, the current soil moisture, if NULL, will not update
 * @param motorStatus, the current motor status, if NULL, will not update
 * @return 0 success while others failed
*/
int CLOUD_ReportMsg(int *airTemperature, int *airHumidity, int *soilMoisture, int *motorStatus);

/**
 * @brief This api will connect to the iot platform
 * @param deviceID, the deviceID create in the iot platform
 * @param devicePwd, the corresponding to the deviceID
 * @param serverIP, the ip of the iot platform
 * @param serverPort, the port correspond to the ip
 * @param cmdCallBack, used for the command callback, if any message comes, will call this callback function
 * @return 0 success while others failed
*/
int CLOUD_Connect(const char *deviceID, const char *devicePwd, \
    const char *serverIP, const char *serverPort, \
    CLOUD_CommandCallBack cmdCallBack);
/**
 * @brief This api will disconnect to the iot platform
 * @return 0 success while others failed
*/
int CLOUD_Disconnect(void);
#endif /* __IOT_CLOUD_API_H__ */

