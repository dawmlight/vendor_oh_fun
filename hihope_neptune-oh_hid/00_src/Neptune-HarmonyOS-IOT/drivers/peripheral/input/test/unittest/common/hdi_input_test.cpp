/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <gtest/gtest.h>
#include <securec.h>
#include "osal_time.h"
#include "hdf_log.h"
#include "input_manager.h"
#include "hdi_input_test.h"

using namespace testing::ext;

IInputInterface *g_inputInterface;
InputReportEventCb g_callback;

class HdiInputTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void HdiInputTest::SetUpTestCase()
{
    int32_t ret = GetInputInterface(&g_inputInterface);
    if (ret != INPUT_SUCCESS) {
        HDF_LOGE("%s: get input hdi failed, ret %d", __func__, ret);
    }
}

void HdiInputTest::TearDownTestCase()
{
    free(g_inputInterface);
    g_inputInterface = NULL;
}

void HdiInputTest::SetUp()
{
}

void HdiInputTest::TearDown()
{
}

#define INPUT_CHECK_NULL_POINTER(pointer, ret) do { \
    if ((pointer) == NULL) { \
        HDF_LOGE("%s: null pointer", __func__); \
        ASSERT_EQ ((ret), INPUT_SUCCESS); \
    } \
} while (0)

void ReportEventPkgCallback(const EventPackage **pkgs, uint32_t count)
{
    if (pkgs == NULL) {
        return;
    }
    for (uint32_t i = 0; i < count; i++) {
        HDF_LOGI("%s: pkgs[%d] = 0x%x, 0x%x, %d", __func__, i, pkgs[i]->type, pkgs[i]->code, pkgs[i]->value);
    }
}

/**
  * @tc.name: OpenInputDev001
  * @tc.desc: open input device test
  * @tc.type: FUNC
  * @tc.require: AR000F867R, AR000F8QNL
  */
HWTEST_F(HdiInputTest, OpenInputDev001, TestSize.Level0)
{
    HDF_LOGI("%s: [Input] OpenInputDev001 enter", __func__);
    INPUT_CHECK_NULL_POINTER(g_inputInterface, INPUT_NULL_PTR);
    INPUT_CHECK_NULL_POINTER(g_inputInterface->iInputManager, INPUT_NULL_PTR);
    int32_t ret = g_inputInterface->iInputManager->OpenInputDevice(TOUCH_INDEX);
    if (ret) {
        HDF_LOGE("%s: open device1 failed, ret %d", __func__, ret);
    }
    EXPECT_EQ(ret, INPUT_SUCCESS);
}

/**
  * @tc.name: OpenInputDevice002
  * @tc.desc: open input device test
  * @tc.type: FUNC
  * @tc.require: AR000F867R
  */
HWTEST_F(HdiInputTest, OpenInputDevice002, TestSize.Level0)
{
    HDF_LOGI("%s: [Input] OpenInputDev002 enter", __func__);
    INPUT_CHECK_NULL_POINTER(g_inputInterface, INPUT_NULL_PTR);
    INPUT_CHECK_NULL_POINTER(g_inputInterface->iInputManager, INPUT_NULL_PTR);
    /* Device "5" is used for testing nonexistent device node */
    int32_t ret = g_inputInterface->iInputManager->OpenInputDevice(INVALID_INDEX);
    if (ret) {
        HDF_LOGE("%s: device5 dose not exist, can't open it, ret %d", __func__, ret);
    }
    EXPECT_NE(ret, INPUT_SUCCESS);
}

/**
  * @tc.name: CloseInputDevice001
  * @tc.desc: close input device test
  * @tc.type: FUNC
  * @tc.require: AR000F867T, AR000F8QNL
  */
HWTEST_F(HdiInputTest, CloseInputDevice001, TestSize.Level0)
{
    HDF_LOGI("%s: [Input] CloseInputDev001 enter", __func__);
    INPUT_CHECK_NULL_POINTER(g_inputInterface, INPUT_NULL_PTR);
    INPUT_CHECK_NULL_POINTER(g_inputInterface->iInputManager, INPUT_NULL_PTR);
    int32_t ret = g_inputInterface->iInputManager->CloseInputDevice(TOUCH_INDEX);
    if (ret) {
        HDF_LOGE("%s: close device1 failed, ret %d", __func__, ret);
    }
    EXPECT_EQ(ret, INPUT_SUCCESS);
}

/**
  * @tc.name: CloseInputDevice002
  * @tc.desc: close input device test
  * @tc.type: FUNC
  * @tc.require: AR000F867T
  */
HWTEST_F(HdiInputTest, CloseInputDevice002, TestSize.Level0)
{
    HDF_LOGI("%s: [Input] CloseInputDev002 enter", __func__);
    INPUT_CHECK_NULL_POINTER(g_inputInterface, INPUT_NULL_PTR);
    INPUT_CHECK_NULL_POINTER(g_inputInterface->iInputManager, INPUT_NULL_PTR);
    /* Device "5" is used for testing nonexistent device node */
    int32_t ret = g_inputInterface->iInputManager->CloseInputDevice(INVALID_INDEX);
    if (ret) {
        HDF_LOGE("%s: device5 doesn't exist, can't close it, ret %d", __func__, ret);
    }
    EXPECT_NE(ret, INPUT_SUCCESS);
}

/**
  * @tc.name: GetInputDevice001
  * @tc.desc: get input device info test
  * @tc.type: FUNC
  * @tc.require: AR000F867S
  */
HWTEST_F(HdiInputTest, GetInputDevice001, TestSize.Level0)
{
    HDF_LOGI("%s: [Input] GetInputDevice001 enter", __func__);
    DeviceInfo *dev = NULL;
    INPUT_CHECK_NULL_POINTER(g_inputInterface, INPUT_NULL_PTR);
    INPUT_CHECK_NULL_POINTER(g_inputInterface->iInputManager, INPUT_NULL_PTR);

    int32_t ret = g_inputInterface->iInputManager->OpenInputDevice(TOUCH_INDEX);
    if (ret) {
        HDF_LOGE("%s: open device1 failed, ret %d", __func__, ret);
    }
    ASSERT_EQ(ret, INPUT_SUCCESS);

    ret = g_inputInterface->iInputManager->GetInputDevice(TOUCH_INDEX, &dev);
    if (ret) {
        HDF_LOGE("%s: get device1 failed, ret %d", __func__, ret);
    }

    HDF_LOGI("%s: devindex = %u, fd = %d, devType = %u", __func__, dev->devIndex,
           dev->fd, dev->devType);
    HDF_LOGI("%s: chipInfo = %s, vendorName = %s, chipName = %s",
        __func__, dev->chipInfo, dev->vendorName, dev->chipName);
    HDF_LOGI("%s: powerStatus = %u, solutionX = %u, solutionY = %u", __func__,
        dev->powerStatus, dev->solutionX, dev->solutionY);
    EXPECT_EQ(ret, INPUT_SUCCESS);
}

/**
  * @tc.name: GetInputDeviceList001
  * @tc.desc: get input device list info test
  * @tc.type: FUNC
  * @tc.require: AR000F8680
  */
HWTEST_F(HdiInputTest, GetInputDeviceList001, TestSize.Level0)
{
    HDF_LOGI("%s: [Input] GetInputDeviceList001 enter", __func__);
    int32_t ret;
    uint32_t num = 0;
    DeviceInfo *dev[MAX_INPUT_DEV_NUM] = {0};

    INPUT_CHECK_NULL_POINTER(g_inputInterface, INPUT_NULL_PTR);
    INPUT_CHECK_NULL_POINTER(g_inputInterface->iInputManager, INPUT_NULL_PTR);
    ret = g_inputInterface->iInputManager->GetInputDeviceList(&num, dev, MAX_INPUT_DEV_NUM);
    if (ret) {
        HDF_LOGE("%s: get device list failed, ret %d", __func__, ret);
    }
    ASSERT_LE(num, MAX_INPUT_DEV_NUM);  /* num <= MAX_INPUT_DEV_NUM return true */

    for (uint32_t i = 0; i < num; i++) {
        HDF_LOGI("%s: num = %u, device[%d]'s info is:", __func__, num, i);
        HDF_LOGI("%s: index = %u, fd = %d, devType = %u", __func__, dev[i]->devIndex,
               dev[i]->fd, dev[i]->devType);
        HDF_LOGI("%s: chipInfo = %s, vendorName = %s, chipName = %s",
            __func__, dev[i]->chipInfo, dev[i]->vendorName, dev[i]->chipName);
        HDF_LOGI("%s: powerStatus = %u, solutionX = %u, solutionY = %u", __func__,
            dev[i]->powerStatus, dev[i]->solutionX, dev[i]->solutionY);
    }
    EXPECT_EQ(ret, INPUT_SUCCESS);
}

/**
  * @tc.name: GetDeviceType001
  * @tc.desc: get input device type test
  * @tc.type: FUNC
  * @tc.require: AR000F8681, AR000F8QNL
  */
HWTEST_F(HdiInputTest, GetDeviceType001, TestSize.Level0)
{
    HDF_LOGI("%s: [Input] GetDeviceType001 enter", __func__);
    int32_t ret;
    uint32_t devType = INIT_DEFAULT_VALUE;

    INPUT_CHECK_NULL_POINTER(g_inputInterface, INPUT_NULL_PTR);
    INPUT_CHECK_NULL_POINTER(g_inputInterface->iInputController, INPUT_NULL_PTR);
    ret = g_inputInterface->iInputController->GetDeviceType(TOUCH_INDEX, &devType);
    if (ret) {
        HDF_LOGE("%s: get device1's type failed, ret %d", __func__, ret);
    }
    HDF_LOGI("%s: device1's type is %u", __func__, devType);
    EXPECT_EQ(ret, INPUT_SUCCESS);
}

/**
  * @tc.name: GetChipInfo001
  * @tc.desc: get input device chip info test
  * @tc.type: FUNC
  * @tc.require: AR000F8682
  */
HWTEST_F(HdiInputTest, GetChipInfo001, TestSize.Level0)
{
    HDF_LOGI("%s: [Input] GetChipInfo001 enter", __func__);
    int32_t ret;
    char chipInfo[CHIP_INFO_LEN] = {0};

    INPUT_CHECK_NULL_POINTER(g_inputInterface, INPUT_NULL_PTR);
    INPUT_CHECK_NULL_POINTER(g_inputInterface->iInputController, INPUT_NULL_PTR);
    ret = g_inputInterface->iInputController->GetChipInfo(TOUCH_INDEX, chipInfo, CHIP_INFO_LEN);
    if (ret) {
        HDF_LOGE("%s: get device1's chip info failed, ret %d", __func__, ret);
    }
    HDF_LOGI("%s: device1's chip info is %s", __func__, chipInfo);
    EXPECT_EQ(ret, INPUT_SUCCESS);
}

/**
  * @tc.name: GetInputDevice002
  * @tc.desc: get input device chip info test
  * @tc.type: FUNC
  * @tc.require: AR000F8683
  */
HWTEST_F(HdiInputTest, GetInputDevice002, TestSize.Level0)
{
    HDF_LOGI("%s: [Input] GetInputDevice002 enter", __func__);
    int32_t ret;
    DeviceInfo *dev = NULL;

    INPUT_CHECK_NULL_POINTER(g_inputInterface, INPUT_NULL_PTR);
    INPUT_CHECK_NULL_POINTER(g_inputInterface->iInputManager, INPUT_NULL_PTR);
    ret = g_inputInterface->iInputManager->GetInputDevice(TOUCH_INDEX, &dev);
    if (ret) {
        HDF_LOGE("%s: get device1 failed, ret %d", __func__, ret);
    }

    HDF_LOGI("%s: After fill the info, new device0's info is:", __func__);
    HDF_LOGI("%s: new devindex = %u, fd = %d, devType = %u", __func__, dev->devIndex,
           dev->fd, dev->devType);
    HDF_LOGI("%s: new chipInfo = %s, vendorName = %s, chipName = %s",
        __func__, dev->chipInfo, dev->vendorName, dev->chipName);
    HDF_LOGI("%s: new powerStatus = %u, solutionX = %u, solutionY = %u", __func__,
        dev->powerStatus, dev->solutionX, dev->solutionY, dev->callback);
    EXPECT_EQ(ret, INPUT_SUCCESS);
}

/**
  * @tc.name: RegisterCallback001
  * @tc.desc: get input device chip info test
  * @tc.type: FUNC
  * @tc.require: AR000F8684, AR000F8QNL
  */
HWTEST_F(HdiInputTest, RegisterCallback001, TestSize.Level0)
{
    HDF_LOGI("%s: [Input] RegisterCallbac001 enter", __func__);
    int32_t ret;
    g_callback.ReportEventPkgCallback = ReportEventPkgCallback;

    INPUT_CHECK_NULL_POINTER(g_inputInterface, INPUT_NULL_PTR);
    INPUT_CHECK_NULL_POINTER(g_inputInterface->iInputReporter, INPUT_NULL_PTR);
    /* Device "5" is used for testing nonexistent device node */
    ret  = g_inputInterface->iInputReporter->RegisterReportCallback(INVALID_INDEX, &g_callback);
    if (ret) {
        HDF_LOGE("%s: device2 dose not exist, can't register callback to it, ret %d", __func__, ret);
    }
    EXPECT_NE(ret, INPUT_SUCCESS);
}

/**
  * @tc.name: SetPowerStatus001
  * @tc.desc: set devcie power status test
  * @tc.type: FUNC
  * @tc.require: AR000F867T
  */
HWTEST_F(HdiInputTest, SetPowerStatus001, TestSize.Level0)
{
    HDF_LOGI("%s: [Input] SetPowerStatus001 enter", __func__);
    int32_t ret;
    uint32_t setStatus = INPUT_LOW_POWER;

    INPUT_CHECK_NULL_POINTER(g_inputInterface, INPUT_NULL_PTR);
    INPUT_CHECK_NULL_POINTER(g_inputInterface->iInputController, INPUT_NULL_PTR);
    ret = g_inputInterface->iInputController->SetPowerStatus(TOUCH_INDEX, setStatus);
    if (ret) {
        HDF_LOGE("%s: set device1's power status failed, ret %d", __func__, ret);
    }
    EXPECT_EQ(ret, INPUT_SUCCESS);
}

/**
  * @tc.name: SetPowerStatus002
  * @tc.desc: set devcie power status test
  * @tc.type: FUNC
  * @tc.require: AR000F867T
  */
HWTEST_F(HdiInputTest, SetPowerStatus002, TestSize.Level0)
{
    HDF_LOGI("%s: [Input] SetPowerStatus002 enter", __func__);
    int32_t ret;
    uint32_t setStatus = INPUT_LOW_POWER;

    INPUT_CHECK_NULL_POINTER(g_inputInterface, INPUT_NULL_PTR);
    INPUT_CHECK_NULL_POINTER(g_inputInterface->iInputController, INPUT_NULL_PTR);
    /* Device "5" is used for testing nonexistent device node */
    ret = g_inputInterface->iInputController->SetPowerStatus(INVALID_INDEX, setStatus);
    if (ret) {
        HDF_LOGE("%s: set device5's power status failed, ret %d", __func__, ret);
    }
    EXPECT_NE(ret, INPUT_SUCCESS);
}

/**
  * @tc.name: GetPowerStatus001
  * @tc.desc: get devcie power status test
  * @tc.type: FUNC
  * @tc.require: AR000F867S
  */
HWTEST_F(HdiInputTest, GetPowerStatus001, TestSize.Level0)
{
    HDF_LOGI("%s: [Input] GetPowerStatus001 enter", __func__);
    int32_t ret;
    uint32_t getStatus = 0;

    INPUT_CHECK_NULL_POINTER(g_inputInterface, INPUT_NULL_PTR);
    INPUT_CHECK_NULL_POINTER(g_inputInterface->iInputController, INPUT_NULL_PTR);
    ret = g_inputInterface->iInputController->GetPowerStatus(TOUCH_INDEX, &getStatus);
    if (ret) {
        HDF_LOGE("%s: get device1's power status failed, ret %d", __func__, ret);
    }
    HDF_LOGI("%s: device1's power status is %d:", __func__, getStatus);
    EXPECT_EQ(ret, INPUT_SUCCESS);
}

/**
  * @tc.name: GetPowerStatus002
  * @tc.desc: get devcie power status test
  * @tc.type: FUNC
  * @tc.require: AR000F867S
  */
HWTEST_F(HdiInputTest, GetPowerStatus002, TestSize.Level0)
{
    HDF_LOGI("%s: [Input] GetPowerStatus002 enter", __func__);
    int32_t ret;
    uint32_t getStatus = 0;

    INPUT_CHECK_NULL_POINTER(g_inputInterface, INPUT_NULL_PTR);
    INPUT_CHECK_NULL_POINTER(g_inputInterface->iInputController, INPUT_NULL_PTR);
    /* Device "5" is used for testing nonexistent device node */
    ret = g_inputInterface->iInputController->GetPowerStatus(INVALID_INDEX, &getStatus);
    if (ret) {
        HDF_LOGE("%s: get device5's power status failed, ret %d", __func__, ret);
    }
    EXPECT_NE(ret, INPUT_SUCCESS);
}

/**
  * @tc.name: GetVendorName001
  * @tc.desc: get device vendor name test
  * @tc.type: FUNC
  * @tc.require: AR000F867T
  */
HWTEST_F(HdiInputTest, GetVendorName001, TestSize.Level0)
{
    HDF_LOGI("%s: [Input] GetVendorName001 enter", __func__);
    int32_t ret;
    char vendorName[NAME_MAX_LEN] = {0};

    INPUT_CHECK_NULL_POINTER(g_inputInterface, INPUT_NULL_PTR);
    INPUT_CHECK_NULL_POINTER(g_inputInterface->iInputController, INPUT_NULL_PTR);
    ret = g_inputInterface->iInputController->GetVendorName(TOUCH_INDEX, vendorName, NAME_MAX_LEN);
    if (ret) {
        HDF_LOGE("%s: get device1's vendor name failed, ret %d", __func__, ret);
    }
    HDF_LOGI("%s: device1's vendor name is %s:", __func__, vendorName);
    EXPECT_EQ(ret, INPUT_SUCCESS);
}

/**
  * @tc.name: GetVendorName002
  * @tc.desc: get device vendor name test
  * @tc.type: FUNC
  * @tc.require: AR000F867T
  */
HWTEST_F(HdiInputTest, GetVendorName002, TestSize.Level0)
{
    HDF_LOGI("%s: [Input] GetVendorName002 enter", __func__);
    int32_t ret;
    char vendorName[NAME_MAX_LEN] = {0};

    INPUT_CHECK_NULL_POINTER(g_inputInterface, INPUT_NULL_PTR);
    INPUT_CHECK_NULL_POINTER(g_inputInterface->iInputController, INPUT_NULL_PTR);
    /* Device "5" is used for testing nonexistent device node */
    ret = g_inputInterface->iInputController->GetVendorName(INVALID_INDEX, vendorName, NAME_MAX_LEN);
    if (ret) {
        HDF_LOGE("%s: get device5's vendor name failed, ret %d", __func__, ret);
    }
    EXPECT_NE(ret, INPUT_SUCCESS);
}

/**
  * @tc.name: GetChipName001
  * @tc.desc: get device chip name test
  * @tc.type: FUNC
  * @tc.require: AR000F867S
  */
HWTEST_F(HdiInputTest, GetChipName001, TestSize.Level0)
{
    HDF_LOGI("%s: [Input] GetChipName001 enter", __func__);
    int32_t ret;
    char chipName[NAME_MAX_LEN] = {0};

    INPUT_CHECK_NULL_POINTER(g_inputInterface, INPUT_NULL_PTR);
    INPUT_CHECK_NULL_POINTER(g_inputInterface->iInputController, INPUT_NULL_PTR);
    ret = g_inputInterface->iInputController->GetChipName(TOUCH_INDEX, chipName, NAME_MAX_LEN);
    if (ret) {
        HDF_LOGE("%s: get device1's chip name failed, ret %d", __func__, ret);
    }
    HDF_LOGI("%s: device1's chip name is %s", __func__, chipName);
    EXPECT_EQ(ret, INPUT_SUCCESS);
}

/**
  * @tc.name: GetChipName002
  * @tc.desc: get device chip name test
  * @tc.type: FUNC
  * @tc.require: AR000F867S
  */
HWTEST_F(HdiInputTest, GetChipName002, TestSize.Level0)
{
    HDF_LOGI("%s: [Input] GetChipName002 enter", __func__);
    int32_t ret;
    char chipName[NAME_MAX_LEN] = {0};

    INPUT_CHECK_NULL_POINTER(g_inputInterface, INPUT_NULL_PTR);
    INPUT_CHECK_NULL_POINTER(g_inputInterface->iInputController, INPUT_NULL_PTR);
    /* Device "5" is used for testing nonexistent device node */
    ret = g_inputInterface->iInputController->GetChipName(INVALID_INDEX, chipName, NAME_MAX_LEN);
    if (ret) {
        HDF_LOGE("%s: get device5's chip name failed, ret %d", __func__, ret);
    }
    EXPECT_NE(ret, INPUT_SUCCESS);
}

/**
  * @tc.name: SetGestureMode001
  * @tc.desc: set device gesture mode test
  * @tc.type: FUNC
  * @tc.require: AR000F867S
  */
HWTEST_F(HdiInputTest, SetGestureMode001, TestSize.Level0)
{
    HDF_LOGI("%s: [Input] SetGestureMode001 enter", __func__);
    int32_t ret;
    uint32_t gestureMode = 1;

    INPUT_CHECK_NULL_POINTER(g_inputInterface, INPUT_NULL_PTR);
    INPUT_CHECK_NULL_POINTER(g_inputInterface->iInputController, INPUT_NULL_PTR);
    ret = g_inputInterface->iInputController->SetGestureMode(TOUCH_INDEX, gestureMode);
    if (ret) {
        HDF_LOGE("%s: get device1's gestureMode failed, ret %d", __func__, ret);
    }
    EXPECT_EQ(ret, INPUT_SUCCESS);
}

/**
  * @tc.name: SetGestureMode002
  * @tc.desc: set device gesture mode test
  * @tc.type: FUNC
  * @tc.require: AR000F867S
  */
HWTEST_F(HdiInputTest, SetGestureMode002, TestSize.Level0)
{
    HDF_LOGI("%s: [Input] SetGestureMode001 enter", __func__);
    int32_t ret;
    uint32_t gestureMode = 1;

    INPUT_CHECK_NULL_POINTER(g_inputInterface, INPUT_NULL_PTR);
    INPUT_CHECK_NULL_POINTER(g_inputInterface->iInputController, INPUT_NULL_PTR);
    /* Device "5" is used for testing nonexistent device node */
    ret = g_inputInterface->iInputController->SetGestureMode(INVALID_INDEX, gestureMode);
    if (ret) {
        HDF_LOGE("%s: get device1's gestureMode failed, ret %d", __func__, ret);
    }
    EXPECT_NE(ret, INPUT_SUCCESS);
}

/**
  * @tc.name: RunCapacitanceTest001
  * @tc.desc: set device gesture mode test
  * @tc.type: FUNC
  * @tc.require: AR000F867R
  */
HWTEST_F(HdiInputTest, RunCapacitanceTest001, TestSize.Level0)
{
    HDF_LOGI("%s: [Input] RunCapacitanceTest001 enter", __func__);
    int32_t ret;
    char result[TEST_RESULT_LEN] = {0};
    uint32_t testType = MMI_TEST;

    INPUT_CHECK_NULL_POINTER(g_inputInterface, INPUT_NULL_PTR);
    INPUT_CHECK_NULL_POINTER(g_inputInterface->iInputController, INPUT_NULL_PTR);
    ret = g_inputInterface->iInputController->RunCapacitanceTest(TOUCH_INDEX, testType, result, TEST_RESULT_LEN);
    if (ret) {
        HDF_LOGE("%s: get device1's gestureMode failed, ret %d", __func__, ret);
    }
    EXPECT_EQ(ret, INPUT_SUCCESS);
}

/**
  * @tc.name: RunCapacitanceTest001
  * @tc.desc: set device gesture mode test
  * @tc.type: FUNC
  * @tc.require: AR000F867R
  */
HWTEST_F(HdiInputTest, RunExtraCommand001, TestSize.Level0)
{
    HDF_LOGI("%s: [Input] RunExtraCommand001 enter", __func__);
    int32_t ret;
    InputExtraCmd extraCmd = {0};
    extraCmd.cmdCode = "WakeUpMode";
    extraCmd.cmdValue = "Enable";

    INPUT_CHECK_NULL_POINTER(g_inputInterface, INPUT_NULL_PTR);
    INPUT_CHECK_NULL_POINTER(g_inputInterface->iInputController, INPUT_NULL_PTR);
    ret = g_inputInterface->iInputController->RunExtraCommand(TOUCH_INDEX, &extraCmd);
    if (ret) {
        HDF_LOGE("%s: get device1's gestureMode failed, ret %d", __func__, ret);
    }
    EXPECT_EQ(ret, INPUT_SUCCESS);
}

/**
  * @tc.name: RegisterCallbackAndReportData001
  * @tc.desc: get input device chip info test
  * @tc.type: FUNC
  * @tc.require: AR000F8682, AR000F8QNL
  */
HWTEST_F(HdiInputTest, RegisterCallbackAndReportData001, TestSize.Level0)
{
    HDF_LOGI("%s: [Input] RegisterCallbackAndReportData001 enter", __func__);
    int32_t ret;
    g_callback.ReportEventPkgCallback = ReportEventPkgCallback;

    INPUT_CHECK_NULL_POINTER(g_inputInterface, INPUT_NULL_PTR);
    INPUT_CHECK_NULL_POINTER(g_inputInterface->iInputReporter, INPUT_NULL_PTR);
    INPUT_CHECK_NULL_POINTER(g_inputInterface->iInputManager, INPUT_NULL_PTR);

    ret  = g_inputInterface->iInputReporter->RegisterReportCallback(TOUCH_INDEX, &g_callback);
    if (ret) {
        HDF_LOGE("%s: register callback failed for device 1, ret %d", __func__, ret);
    }
    EXPECT_EQ(ret, INPUT_SUCCESS);
    HDF_LOGI("%s: wait 10s for testing, pls touch the panel now", __func__);
    HDF_LOGI("%s: The event data is as following:", __func__);
    OsalMSleep(KEEP_ALIVE_TIME_MS);
}

/**
  * @tc.name: UnregisterReportCallback001
  * @tc.desc: get input device chip info test
  * @tc.type: FUNC
  * @tc.require: SR000F867Q
  */
HWTEST_F(HdiInputTest, UnregisterReportCallback001, TestSize.Level0)
{
    HDF_LOGI("%s: [Input] UnregisterReportCallback001 enter", __func__);
    int32_t ret;
    INPUT_CHECK_NULL_POINTER(g_inputInterface, INPUT_NULL_PTR);
    INPUT_CHECK_NULL_POINTER(g_inputInterface->iInputReporter, INPUT_NULL_PTR);
    INPUT_CHECK_NULL_POINTER(g_inputInterface->iInputManager, INPUT_NULL_PTR);

    ret  = g_inputInterface->iInputReporter->UnregisterReportCallback(TOUCH_INDEX);
    if (ret) {
        HDF_LOGE("%s: unregister callback failed for device1, ret %d", __func__, ret);
    }
    EXPECT_EQ(ret, INPUT_SUCCESS);

    ret = g_inputInterface->iInputManager->CloseInputDevice(TOUCH_INDEX);
    if (ret) {
        HDF_LOGE("%s: close device1 failed, ret %d", __func__, ret);
    }
    EXPECT_EQ(ret, INPUT_SUCCESS);
    HDF_LOGI("%s: Close the device1 successfully after all test", __func__);
}
