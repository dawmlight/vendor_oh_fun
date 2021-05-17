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

#include <gtest/gtest.h>
#include "hdf_base.h"
#include "hdf_uhdf_test.h"
#include "hdf_sbuf.h"
#include "wifi_hal.h"
#include "wifi_hal_ap_feature.h"
#include "wifi_hal_base_feature.h"
#include "wifi_hal_event.h"
#include "wifi_hal_sta_feature.h"

using namespace testing::ext;

namespace HalTest {
struct IWiFi *g_wifi = nullptr;
const int32_t USEC_TIME = 1000000;
const int32_t MSEC_TIME = 1000;
const int32_t COMMON_TIME = 3000;
const int32_t MEDIUM_TIME = 20000;
const int32_t LONG_TIME = 200000;
const int32_t ETH_ADDR_LEN = 6;
const int32_t WLAN_BAND_2G = 0;
const int32_t WLAN_FREQ_MAX_NUM = 14;
const int32_t WLAN_MAX_NUM_STA_WITH_AP = 4;
const uint32_t DEFAULT_COMBO_SIZE = 10;
const uint32_t RESET_TIME = 20;
const uint32_t WLAN_MIN_CHIPID = 0;
const uint32_t WLAN_MAX_CHIPID = 2;
const uint32_t IFNAME_MIN_NUM = 0;
const uint32_t IFNAME_MAX_NUM = 32;
const uint32_t MAX_IF_NAME_LENGTH = 16;
const uint32_t SIZE = 4;

static int32_t g_resetStatus = -1;

class HdfWlanPerformanceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void HdfWlanPerformanceTest::SetUpTestCase()
{
    int ret;

    ret = WifiConstruct(&g_wifi);
    ASSERT_EQ(HDF_SUCCESS, ret);
}

void HdfWlanPerformanceTest::TearDownTestCase()
{
    int ret;

    ret = WifiDestruct(&g_wifi);
    ASSERT_EQ(HDF_SUCCESS, ret);
}

void HdfWlanPerformanceTest::SetUp()
{
    int ret;

    ret = g_wifi->start(nullptr);
    ASSERT_NE(HDF_SUCCESS, ret);
    ret = g_wifi->start(g_wifi);
    ASSERT_EQ(HDF_SUCCESS, ret);
}

void HdfWlanPerformanceTest::TearDown()
{
    int ret;

    ret = g_wifi->stop(nullptr);
    ASSERT_NE(HDF_SUCCESS, ret);
    ret = g_wifi->stop(g_wifi);
    ASSERT_EQ(HDF_SUCCESS, ret);
}

static int32_t HalResetCallbackEvent(int32_t event, struct HdfSBuf *sbuf)
{
    (void)event;
    unsigned char chipId;
    int resetStatus;
    if (sbuf == nullptr) {
        return HDF_FAILURE;
    }

    if (!HdfSbufReadInt32(sbuf, &resetStatus)) {
        return HDF_FAILURE;
    }
    if (!HdfSbufReadUint8(sbuf, &chipId)) {
        return HDF_FAILURE;
    }
    g_resetStatus = resetStatus;
    return HDF_SUCCESS;
}

/**
 * @tc.name: WifiHalGetSupportFeature001
 * @tc.desc: Wifi hal get support feature function test
 * @tc.type: FUNC
 * @tc.require: AR000F869J
 */
HWTEST_F(HdfWlanPerformanceTest, WifiHalGetSupportFeature001, TestSize.Level0)
{
    int ret;
    uint8_t supportTest[PROTOCOL_80211_IFTYPE_NUM] = {0};
    uint8_t support[PROTOCOL_80211_IFTYPE_NUM + 1] = {0};
    struct timespec tv1 = (struct timespec){0};
    struct timespec tv2 = (struct timespec){0};
    int timeUsed = 0;

    ret = g_wifi->getSupportFeature(nullptr, 0);
    EXPECT_NE(HDF_SUCCESS, ret);
    ret = g_wifi->getSupportFeature(supportTest, PROTOCOL_80211_IFTYPE_NUM);
    EXPECT_NE(HDF_SUCCESS, ret);
    clock_gettime(CLOCK_REALTIME, &tv1);
    ret = g_wifi->getSupportFeature(support, PROTOCOL_80211_IFTYPE_NUM + 1);
    clock_gettime(CLOCK_REALTIME, &tv2);
    timeUsed = ((tv2.tv_sec * USEC_TIME + tv2.tv_nsec / MSEC_TIME) -
        (tv1.tv_sec * USEC_TIME + tv1.tv_nsec / MSEC_TIME));
    EXPECT_GE(COMMON_TIME, timeUsed);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.name: WifiHalGetSupportCombo001
 * @tc.desc: Wifi hal get support combo function test
 * @tc.type: FUNC
 * @tc.require: AR000F869J
 */
HWTEST_F(HdfWlanPerformanceTest, WifiHalGetSupportCombo001, TestSize.Level0)
{
    int ret;
    uint8_t support[PROTOCOL_80211_IFTYPE_NUM + 1] = {0};
    uint64_t combo[DEFAULT_COMBO_SIZE] = {0};
    struct timespec tv1 = (struct timespec){0};
    struct timespec tv2 = (struct timespec){0};
    int timeUsed = 0;

    ret = g_wifi->getSupportFeature(support, PROTOCOL_80211_IFTYPE_NUM + 1);
    EXPECT_EQ(HDF_SUCCESS, ret);
    ret = g_wifi->getSupportCombo(nullptr, 0);
    EXPECT_NE(HDF_SUCCESS, ret);
    clock_gettime(CLOCK_REALTIME, &tv1);
    ret = g_wifi->getSupportCombo(combo, DEFAULT_COMBO_SIZE);
    clock_gettime(CLOCK_REALTIME, &tv2);
    timeUsed = ((tv2.tv_sec * USEC_TIME + tv2.tv_nsec / MSEC_TIME) -
        (tv1.tv_sec * USEC_TIME + tv1.tv_nsec / MSEC_TIME));
    EXPECT_GE(COMMON_TIME, timeUsed);
    if (support[PROTOCOL_80211_IFTYPE_NUM] == 0) {
        EXPECT_EQ(HDF_ERR_NOT_SUPPORT, ret);
    } else {
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
}

/**
 * @tc.name: WifiHalGetDeviceMacAddress001
 * @tc.desc: Wifi hal get device Mac address function test
 * @tc.type: FUNC
 * @tc.require: AR000F869G
 */
HWTEST_F(HdfWlanPerformanceTest, WifiHalGetDeviceMacAddress001, TestSize.Level0)
{
    int ret;
    struct IWiFiAp *apFeature = nullptr;
    unsigned char mac[ETH_ADDR_LEN] = {0};
    struct timespec tv1 = (struct timespec){0};
    struct timespec tv2 = (struct timespec){0};
    int timeUsed = 0;

    ret = g_wifi->createFeature(PROTOCOL_80211_IFTYPE_AP, (struct IWiFiBaseFeature **)&apFeature);
    EXPECT_EQ(HDF_SUCCESS, ret);
    EXPECT_NE(nullptr, apFeature);
    ret = apFeature->baseFeature.getDeviceMacAddress((struct IWiFiBaseFeature *)apFeature, nullptr, 0);
    EXPECT_NE(HDF_SUCCESS, ret);
    ret = apFeature->baseFeature.getDeviceMacAddress((struct IWiFiBaseFeature *)apFeature, mac, ETH_ADDR_LEN - 1);
    EXPECT_NE(HDF_SUCCESS, ret);
    clock_gettime(CLOCK_REALTIME, &tv1);
    ret = apFeature->baseFeature.getDeviceMacAddress((struct IWiFiBaseFeature *)apFeature, mac, ETH_ADDR_LEN);
    clock_gettime(CLOCK_REALTIME, &tv2);
    timeUsed = ((tv2.tv_sec * USEC_TIME + tv2.tv_nsec / MSEC_TIME) -
        (tv1.tv_sec * USEC_TIME + tv1.tv_nsec / MSEC_TIME));
    EXPECT_GE(LONG_TIME, timeUsed);
    EXPECT_NE(HDF_FAILURE, ret);

    ret = g_wifi->destroyFeature((struct IWiFiBaseFeature *)apFeature);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.name: WifiHalGetDeviceMacAddress002
 * @tc.desc: Wifi hal get device Mac address function test
 * @tc.type: FUNC
 * @tc.require: AR000F869E
 */
HWTEST_F(HdfWlanPerformanceTest, WifiHalGetDeviceMacAddress002, TestSize.Level0)
{
    int ret;
    struct IWiFiSta *staFeature = nullptr;
    unsigned char mac[ETH_ADDR_LEN] = {0};
    struct timespec tv1 = (struct timespec){0};
    struct timespec tv2 = (struct timespec){0};
    int timeUsed = 0;

    ret = g_wifi->createFeature(PROTOCOL_80211_IFTYPE_STATION, (struct IWiFiBaseFeature **)&staFeature);
    EXPECT_EQ(HDF_SUCCESS, ret);
    EXPECT_NE(nullptr, staFeature);
    ret = staFeature->baseFeature.getDeviceMacAddress((struct IWiFiBaseFeature *)staFeature, nullptr, 0);
    EXPECT_NE(HDF_SUCCESS, ret);
    ret = staFeature->baseFeature.getDeviceMacAddress((struct IWiFiBaseFeature *)staFeature, mac, ETH_ADDR_LEN - 1);
    EXPECT_NE(HDF_SUCCESS, ret);
    clock_gettime(CLOCK_REALTIME, &tv1);
    ret = staFeature->baseFeature.getDeviceMacAddress((struct IWiFiBaseFeature *)staFeature, mac, ETH_ADDR_LEN);
    clock_gettime(CLOCK_REALTIME, &tv2);
    timeUsed = ((tv2.tv_sec * USEC_TIME + tv2.tv_nsec / MSEC_TIME) -
        (tv1.tv_sec * USEC_TIME + tv1.tv_nsec / MSEC_TIME));
    EXPECT_GE(LONG_TIME, timeUsed);
    EXPECT_NE(HDF_FAILURE, ret);

    ret = g_wifi->destroyFeature((struct IWiFiBaseFeature *)staFeature);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.name: WifiHaGetValidFreqsWithBand001
 * @tc.desc: Wifi hal get available freqs with band function test
 * @tc.type: FUNC
 * @tc.require: AR000F869G
 */
HWTEST_F(HdfWlanPerformanceTest, WifiHaGetValidFreqsWithBand001, TestSize.Level0)
{
    int ret;
    struct IWiFiAp *apFeature = nullptr;
    int32_t freq[WLAN_FREQ_MAX_NUM] = {0};
    uint32_t num = 0;
    struct timespec tv1 = (struct timespec){0};
    struct timespec tv2 = (struct timespec){0};
    int timeUsed = 0;

    ret = g_wifi->createFeature(PROTOCOL_80211_IFTYPE_AP, (struct IWiFiBaseFeature **)&apFeature);
    EXPECT_EQ(HDF_SUCCESS, ret);
    EXPECT_NE(nullptr, apFeature);
    ret = apFeature->baseFeature.getValidFreqsWithBand((struct IWiFiBaseFeature *)apFeature,
    WLAN_BAND_2G, nullptr, 0, nullptr);
    EXPECT_NE(HDF_SUCCESS, ret);
    clock_gettime(CLOCK_REALTIME, &tv1);
    ret = apFeature->baseFeature.getValidFreqsWithBand((struct IWiFiBaseFeature *)apFeature,
    WLAN_BAND_2G, freq, WLAN_FREQ_MAX_NUM, &num);
    clock_gettime(CLOCK_REALTIME, &tv2);
    timeUsed = ((tv2.tv_sec * USEC_TIME + tv2.tv_nsec / MSEC_TIME) -
        (tv1.tv_sec * USEC_TIME + tv1.tv_nsec / MSEC_TIME));
    EXPECT_GE(COMMON_TIME, timeUsed);
    EXPECT_EQ(HDF_SUCCESS, ret);

    ret = g_wifi->destroyFeature((struct IWiFiBaseFeature *)apFeature);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.name: WifiHalGetAsscociatedStas001
 * @tc.desc: Wifi hal get asscociated sta info function test
 * @tc.type: FUNC
 * @tc.require: AR000F869K
 */
HWTEST_F(HdfWlanPerformanceTest, WifiHalGetAsscociatedStas001, TestSize.Level0)
{
    int ret;
    struct IWiFiAp *apFeature = nullptr;
    struct StaInfo staInfo[WLAN_MAX_NUM_STA_WITH_AP] = {0};
    uint32_t num = 0;
    struct timespec tv1 = (struct timespec){0};
    struct timespec tv2 = (struct timespec){0};
    int timeUsed = 0;

    ret = g_wifi->createFeature(PROTOCOL_80211_IFTYPE_AP, (struct IWiFiBaseFeature **)&apFeature);
    EXPECT_EQ(HDF_SUCCESS, ret);
    EXPECT_NE(nullptr, apFeature);
    ret = apFeature->getAsscociatedStas(apFeature, nullptr, 0, nullptr);
    EXPECT_NE(HDF_SUCCESS, ret);
    clock_gettime(CLOCK_REALTIME, &tv1);
    ret = apFeature->getAsscociatedStas(apFeature, staInfo, WLAN_MAX_NUM_STA_WITH_AP, &num);
    clock_gettime(CLOCK_REALTIME, &tv2);
    timeUsed = ((tv2.tv_sec * USEC_TIME + tv2.tv_nsec / MSEC_TIME) -
        (tv1.tv_sec * USEC_TIME + tv1.tv_nsec / MSEC_TIME));
    EXPECT_GE(COMMON_TIME, timeUsed);
    EXPECT_EQ(HDF_SUCCESS, ret);

    ret = g_wifi->destroyFeature((struct IWiFiBaseFeature *)apFeature);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.name: WifiHalSetScanningMacAddress001
 * @tc.desc: Wifi hal set scanning Mac address function test
 * @tc.type: FUNC
 * @tc.require: AR000F869K
 */
HWTEST_F(HdfWlanPerformanceTest, WifiHalSetScanningMacAddress001, TestSize.Level0)
{
    int ret;
    struct IWiFiSta *staFeature = nullptr;
    unsigned char scanMac[ETH_ADDR_LEN] = {0x12, 0x34, 0x56, 0x78, 0xab, 0xcd};
    struct timespec tv1 = (struct timespec){0};
    struct timespec tv2 = (struct timespec){0};
    int timeUsed = 0;

    ret = g_wifi->createFeature(PROTOCOL_80211_IFTYPE_STATION, (struct IWiFiBaseFeature **)&staFeature);
    EXPECT_EQ(HDF_SUCCESS, ret);
    EXPECT_NE(nullptr, staFeature);
    ret = staFeature->setScanningMacAddress(staFeature, nullptr, 0);
    EXPECT_NE(HDF_SUCCESS, ret);
    clock_gettime(CLOCK_REALTIME, &tv1);
    ret = staFeature->setScanningMacAddress(staFeature, scanMac, ETH_ADDR_LEN);
    clock_gettime(CLOCK_REALTIME, &tv2);
    timeUsed = ((tv2.tv_sec * USEC_TIME + tv2.tv_nsec / MSEC_TIME) -
        (tv1.tv_sec * USEC_TIME + tv1.tv_nsec / MSEC_TIME));
    EXPECT_GE(LONG_TIME, timeUsed);
    EXPECT_EQ(HDF_ERR_NOT_SUPPORT, ret);

    ret = g_wifi->destroyFeature((struct IWiFiBaseFeature *)staFeature);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.name: WifiHalGetChipId001
 * @tc.desc: Wifi hal obtain the chip ID function test
 * @tc.type: FUNC
 * @tc.require: AR000F869G
 */
HWTEST_F(HdfWlanPerformanceTest, WifiHalGetChipId001, TestSize.Level0)
{
    int ret;
    struct IWiFiSta *staFeature = nullptr;
    unsigned char chipId = 0;
    struct timespec tv1 = (struct timespec){0};
    struct timespec tv2 = (struct timespec){0};
    int timeUsed = 0;

    ret = g_wifi->createFeature(PROTOCOL_80211_IFTYPE_STATION, (struct IWiFiBaseFeature **)&staFeature);
    EXPECT_EQ(HDF_SUCCESS, ret);
    EXPECT_NE(nullptr, staFeature);
    ret = staFeature->baseFeature.getChipId((struct IWiFiBaseFeature *)staFeature, nullptr);
    EXPECT_NE(HDF_SUCCESS, ret);
    clock_gettime(CLOCK_REALTIME, &tv1);
    ret = staFeature->baseFeature.getChipId((struct IWiFiBaseFeature *)staFeature, &chipId);
    clock_gettime(CLOCK_REALTIME, &tv2);
    timeUsed = ((tv2.tv_sec * USEC_TIME + tv2.tv_nsec / MSEC_TIME) -
        (tv1.tv_sec * USEC_TIME + tv1.tv_nsec / MSEC_TIME));
    EXPECT_GE(MEDIUM_TIME, timeUsed);
    ASSERT_TRUE(chipId <= WLAN_MAX_CHIPID && chipId >= WLAN_MIN_CHIPID);
    EXPECT_EQ(HDF_SUCCESS, ret);

    ret = g_wifi->destroyFeature((struct IWiFiBaseFeature *)staFeature);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.name: WifiHalGetIfNamesByChipId001
 * @tc.desc: Wifi hal obtain all ifNames and the number of the current chip function test
 * @tc.type: FUNC
 * @tc.require: AR000F869G
 */
HWTEST_F(HdfWlanPerformanceTest, WifiHalGetIfNamesByChipId001, TestSize.Level0)
{
    int ret;
    struct IWiFiSta *staFeature = nullptr;
    char *ifNames = nullptr;
    unsigned int num = 0;
    unsigned char chipId = 0;
    struct timespec tv1 = (struct timespec){0};
    struct timespec tv2 = (struct timespec){0};
    int timeUsed = 0;
    uint8_t i;

    ret = g_wifi->createFeature(PROTOCOL_80211_IFTYPE_STATION, (struct IWiFiBaseFeature **)&staFeature);
    EXPECT_EQ(HDF_SUCCESS, ret);
    EXPECT_NE(nullptr, staFeature);
    ret = staFeature->baseFeature.getChipId((struct IWiFiBaseFeature *)staFeature, &chipId);
    ASSERT_TRUE(chipId <= WLAN_MAX_CHIPID && chipId >= WLAN_MIN_CHIPID);
    EXPECT_EQ(HDF_SUCCESS, ret);
    ret = staFeature->baseFeature.getIfNamesByChipId(chipId, nullptr, nullptr);
    EXPECT_NE(HDF_SUCCESS, ret);
    clock_gettime(CLOCK_REALTIME, &tv1);
    ret = staFeature->baseFeature.getIfNamesByChipId(chipId, &ifNames, &num);
    clock_gettime(CLOCK_REALTIME, &tv2);
    EXPECT_NE(nullptr, ifNames);
    EXPECT_EQ(HDF_SUCCESS, ret);
    timeUsed = ((tv2.tv_sec * USEC_TIME + tv2.tv_nsec / MSEC_TIME) -
        (tv1.tv_sec * USEC_TIME + tv1.tv_nsec / MSEC_TIME));
    ASSERT_TRUE(num <= IFNAME_MAX_NUM && num >= IFNAME_MIN_NUM);
    for (i = 0; i < num; i++) {
        EXPECT_EQ(0, strncmp("wlan", ifNames + i * MAX_IF_NAME_LENGTH, SIZE));
    }
    EXPECT_GE(MEDIUM_TIME, timeUsed);
    free(ifNames);

    ret = g_wifi->destroyFeature((struct IWiFiBaseFeature *)staFeature);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.name: WifiHalResetDriver001
 * @tc.desc: Wifi hal Reset the wifi Driver function test
 * @tc.type: FUNC
 * @tc.require: AR000F869G
 */
HWTEST_F(HdfWlanPerformanceTest, WifiHalResetDriver001, TestSize.Level0)
{
    int ret;
    struct IWiFiSta *staFeature = nullptr;
    char *ifNames = nullptr;
    unsigned int num = 0;
    unsigned char chipId = 0;
    struct timespec tv1 = (struct timespec){0};
    struct timespec tv2 = (struct timespec){0};
    int timeUsed = 0;
    uint8_t i;

    ret = g_wifi->registerEventCallback(HalResetCallbackEvent);
    EXPECT_EQ(HDF_SUCCESS, ret);
    ret = g_wifi->createFeature(PROTOCOL_80211_IFTYPE_STATION, (struct IWiFiBaseFeature **)&staFeature);
    EXPECT_EQ(HDF_SUCCESS, ret);
    EXPECT_NE(nullptr, staFeature);
    ret = staFeature->baseFeature.getChipId((struct IWiFiBaseFeature *)staFeature, &chipId);
    ASSERT_TRUE(chipId <= WLAN_MAX_CHIPID && chipId >= WLAN_MIN_CHIPID);
    EXPECT_EQ(HDF_SUCCESS, ret);
    ret = staFeature->baseFeature.getIfNamesByChipId(chipId, &ifNames, &num);
    EXPECT_NE(nullptr, ifNames);
    ASSERT_TRUE(num <= IFNAME_MAX_NUM && num >= IFNAME_MIN_NUM);
    for (i = 0; i < num; i++) {
        EXPECT_EQ(0, strncmp("wlan", ifNames + i * MAX_IF_NAME_LENGTH, SIZE));
    }
    EXPECT_EQ(HDF_SUCCESS, ret);
    free(ifNames);
    ret = g_wifi->destroyFeature((struct IWiFiBaseFeature *)staFeature);
    EXPECT_EQ(HDF_SUCCESS, ret);

    g_resetStatus = HDF_FAILURE;
    clock_gettime(CLOCK_REALTIME, &tv1);
    ret = g_wifi->resetDriver(chipId);
    clock_gettime(CLOCK_REALTIME, &tv2);
    EXPECT_EQ(HDF_SUCCESS, ret);
    timeUsed = ((tv2.tv_sec * USEC_TIME + tv2.tv_nsec / MSEC_TIME) -
        (tv1.tv_sec * USEC_TIME + tv1.tv_nsec / MSEC_TIME));
    EXPECT_GE(LONG_TIME, timeUsed);
    sleep(RESET_TIME);
    EXPECT_EQ(HDF_SUCCESS, g_resetStatus);
}
};
