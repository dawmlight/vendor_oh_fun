/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
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

#include "gtest/gtest.h"
#include "utils/SamgrTestBase.h"

using namespace testing::ext;

struct DefaultFeatureApi {
    INHERIT_IUNKNOWN;
};

struct ExampleService {
    INHERIT_SERVICE;
    INHERIT_IUNKNOWNENTRY(DefaultFeatureApi);
    Identity identity;
};

struct DemoApi {
    INHERIT_IUNKNOWN;
};

struct DemoFeature {
    INHERIT_FEATURE;
    INHERIT_IUNKNOWNENTRY(DemoApi);
    Identity identity;
};

static const char *GetName(Service *service)
{
    (void)service;
    return "serviceName101";
}

static BOOL Initialize(Service *service, Identity identity)
{
    printf("[hcpptest]Initialize. \n");
    ExampleService *example = (ExampleService *)service;
    example->identity = identity;
    return TRUE;
}

static BOOL MessageHandle(Service *service, Request *msg)
{
    (void)service;
    return FALSE;
}

static TaskConfig GetTaskConfig(Service *service)
{
    (void)service;
    TaskConfig config = {LEVEL_HIGH, PRI_ABOVE_NORMAL, 0x800, 20, SHARED_TASK};
    return config;
}

static ExampleService g_service = {
    .GetName = GetName,
    .Initialize = Initialize,
    .MessageHandle = MessageHandle,
    .GetTaskConfig = GetTaskConfig,
    .ver = 1,
    .ref = 1,
    .iUnknown = {
        DEFAULT_IUNKNOWN_IMPL,
    },
};

static const char *FEATURE_GetName(Feature *feature)
{
    (void)feature;
    return "featureName101";
}
static void FEATURE_OnInitialize(Feature *feature, Service *parent, Identity identity)
{
    DemoFeature *demoFeature = (DemoFeature *)feature;
    demoFeature->identity = identity;
    (void)parent;
}
static void FEATURE_OnStop(Feature *feature, Identity identity)
{
    (void)feature;
    (void)identity;
}

static BOOL FEATURE_OnMessage(Feature *feature, Request *request)
{
    (void)feature;
    (void)request;
    return TRUE;
}

static DemoFeature g_feature = {
    .GetName = FEATURE_GetName,
    .OnInitialize = FEATURE_OnInitialize,
    .OnStop = FEATURE_OnStop,
    .OnMessage = FEATURE_OnMessage,
    .ver = DEFAULT_VERSION,
    .ref = 1,
    .iUnknown = {
        DEFAULT_IUNKNOWN_IMPL,
    },
    .identity = {-1, -1, NULL},
};

class ServiceTest : public testing::Test {
protected:
    // SetUpTestCase: Testsuit setup, run before 1st testcase
    static void SetUpTestCase(void)
    {
        SystemInitProxy();
    }
    // TearDownTestCase: Testsuit teardown, run after last testcase
    static void TearDownTestCase(void)
    {
    }
    // Testcase setup
    virtual void SetUp()
    {
    }
    // Testcase teardown
    virtual void TearDown()
    {
    }
};

/**
 * @tc.number    : DMSLite_SAMGR_RegisterService_0010
 * @tc.name      : Valid service can be registered
 * @tc.desc      : [C- SOFTWARE -0200]
*/
HWTEST_F(ServiceTest, testRegisterService0010, Function | MediumTest | Level1) {
    BOOL result = SAMGR_GetInstance()->RegisterService((Service *)&g_service);
    ASSERT_EQ(result, TRUE);

    SAMGR_GetInstance()->UnregisterService("serviceName101");
}

/**
 * @tc.number    : DMSLite_SAMGR_RegisterService_0020
 * @tc.name      : Service which is nullptr failed to register
 * @tc.desc      : [C- SOFTWARE -0200]
*/
HWTEST_F(ServiceTest, testRegisterService0020, Function | MediumTest | Level2) {
    BOOL result = SAMGR_GetInstance()->RegisterService(NULL);
    ASSERT_EQ(result, FALSE);
}

/**
 * @tc.number    : DMSLite_SAMGR_RegisterService_0030
 * @tc.name      : Service which already exist in samgr failed to register
 * @tc.desc      : [C- SOFTWARE -0200]
*/
HWTEST_F(ServiceTest, testRegisterService0030, Function | MediumTest | Level2) {
    SAMGR_GetInstance()->RegisterService((Service *)&g_service);
    BOOL result = SAMGR_GetInstance()->RegisterService((Service *)&g_service);
    ASSERT_EQ(result, FALSE);
    SAMGR_GetInstance()->UnregisterService("serviceName101");
}

/**
 * @tc.number    : DMSLite_SAMGR_UnregisterService_0010
 * @tc.name      : Service can be unregistered
 * @tc.desc      : [C- SOFTWARE -0200]
*/
HWTEST_F(ServiceTest, testUnregisterService0010, Function | MediumTest | Level2) {
    SAMGR_GetInstance()->RegisterService((Service *)&g_service);
    Service *resultService = SAMGR_GetInstance()->UnregisterService("serviceName101");
    ASSERT_EQ(strcmp(resultService->GetName(resultService), "serviceName101"), 0);
}

/**
 * @tc.number    : DMSLite_SAMGR_UnregisterService_0020
 * @tc.name      : Service which contains NULL failed to unregister
 * @tc.desc      : [C- SOFTWARE -0200]
*/
HWTEST_F(ServiceTest, testUnregisterService0020, Function | MediumTest | Level2) {
    Service *resultService = SAMGR_GetInstance()->UnregisterService(NULL);
    ASSERT_EQ(resultService == nullptr, TRUE);

    resultService = SAMGR_GetInstance()->UnregisterService("");
    ASSERT_EQ(resultService == nullptr, TRUE);
}

/**
 * @tc.number    : DMSLite_SAMGR_UnregisterService_0030
 * @tc.name      : Service which does not exist in samgr failed to unregister
 * @tc.desc      : [C- SOFTWARE -0200]
*/
HWTEST_F(ServiceTest, testUnregisterService0030, Function | MediumTest | Level2) {
    Service *resultService = SAMGR_GetInstance()->UnregisterService("noExistServiceName");
    ASSERT_EQ(resultService == nullptr, TRUE);
}

/**
 * @tc.number    : DMSLite_SAMGR_UnregisterService_0040
 * @tc.name      : Service which contains api failed to unregister
 * @tc.desc      : [C- SOFTWARE -0200]
*/
HWTEST_F(ServiceTest, testUnregisterService0040, Function | MediumTest | Level2) {
    SAMGR_GetInstance()->RegisterService((Service *)&g_service);
    SAMGR_GetInstance()->RegisterDefaultFeatureApi("serviceName101", GET_IUNKNOWN(g_service));

    Service *resultService = SAMGR_GetInstance()->UnregisterService("serviceName101");
    ASSERT_EQ(resultService == nullptr, TRUE);

    SAMGR_GetInstance()->UnregisterDefaultFeatureApi("serviceName101");
    SAMGR_GetInstance()->UnregisterService("serviceName101");
}

/**
 * @tc.number    : DMSLite_SAMGR_UnregisterService_0050
 * @tc.name      : Service which contains feature failed to unregister
 * @tc.desc      : [C- SOFTWARE -0200]
*/
HWTEST_F(ServiceTest, testUnregisterService0050, Function | MediumTest | Level2) {
    SAMGR_GetInstance()->RegisterService((Service *)&g_service);
    SAMGR_GetInstance()->RegisterFeature("serviceName101", (Feature *)&g_feature);

    Service *resultService = SAMGR_GetInstance()->UnregisterService("serviceName101");
    ASSERT_EQ(resultService == nullptr, TRUE);

    SAMGR_GetInstance()->UnregisterFeature("serviceName101", "featureName101");
    SAMGR_GetInstance()->UnregisterService("serviceName101");
}

/**
 * @tc.number    : DMSLite_SAMGR_UnregisterService_0060
 * @tc.name      : Register and unregister service repeatedly, no memory leak
 * @tc.desc      : [C- SOFTWARE -0200]
*/
HWTEST_F(ServiceTest, testUnregisterService0060, Function | MediumTest | Level2) {
    // RegisterService and unregister service over and over again, if there is memory leak samgr will crash
    for (int i = 0; i < PRESSURE_BASE; i++) {
        BOOL result = SAMGR_GetInstance()->RegisterService((Service *)&g_service);
        ASSERT_EQ(result, TRUE);

        Service *resultService = SAMGR_GetInstance()->UnregisterService("serviceName101");
        ASSERT_EQ(resultService != nullptr, TRUE);
    }
}

/**
 * @tc.number    : DMSLite_SAMGR_Bootstrap_0010
 * @tc.name      : Restart SAMGR service function is normal.
 * @tc.desc      : [C- SOFTWARE -0200]
*/
HWTEST_F(ServiceTest, testBootstrap0010, Function | MediumTest | Level2) {
    SAMGR_Bootstrap();
    usleep(OPER_INTERVAL);

    SamgrLite *samgrLite = SAMGR_GetInstance();
    ASSERT_EQ(samgrLite != nullptr, TRUE);
}