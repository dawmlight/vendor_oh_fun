/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
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

#include "ohos_types.h"
#include "ohos_init.h"
#include "hiview_config.h"

HiviewConfig g_hiviewConfig = {
    .outputOption = OUTPUT_OPTION_FLOW,
    .level = 1,    /* Control log output level. HILOG_LV_XXX, default is HILOG_LV_DEBUG */
    .logSwitch = HIVIEW_FEATURE_ON,
    .dumpSwitch = HIVIEW_FEATURE_OFF,
    .eventSwitch = HIVIEW_FEATURE_OFF,
};

static void HiviewConfigInit(void)
{
    g_hiviewConfig.hiviewInited = FALSE;
    g_hiviewConfig.logOutputModule = UINT64_MAX;
    g_hiviewConfig.writeFailureCount = 0;
}
CORE_INIT_PRI(HiviewConfigInit, 0);
