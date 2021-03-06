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

#include "dump_syspara.h"
#include <stdio.h>
#include <stdlib.h>
#include "parameter.h"
#if defined(__LITEOS_RISCV__)
#include "wifiiot_at.h"
#endif

static const SysParaInfoItem SYSPARA_LIST[] = {
    {"ProductType", GetProductType},
    {"Manufacture", GetManufacture},
    {"Brand", GetBrand},
    {"MarketName", GetMarketName},
    {"ProductSeries", GetProductSeries},
    {"ProductModel", GetProductModel},
    {"SoftwareModel", GetSoftwareModel},
    {"HardwareModel", GetHardwareModel},
    {"Serial", GetSerial},
    {"OsName", GetOsName},
    {"DisplayVersion", GetDisplayVersion},
    {"BootloaderVersion", GetBootloaderVersion},
    {"GetSecurityPatchTag", GetSecurityPatchTag},
    {"AbiList", GetAbiList},
    {"SdkApiLevel", GetSdkApiLevel},
    {"FirstApiLevel", GetFirstApiLevel},
    {"IncrementalVersion", GetIncrementalVersion},
    {"VersionId", GetVersionId},
    {"BuildType", GetBuildType},
    {"BuildUser", GetBuildUser},
    {"BuildHost", GetBuildHost},
    {"BuildTime", GetBuildTime},
    {"BuildRootHash", GetBuildRootHash},
};

int QuerySysparaCmd()
{
    int index = 0;
    int dumpInfoItemNum = (sizeof(SYSPARA_LIST) / sizeof(SysParaInfoItem));
    char *temp = NULL;
    int (*pfnPrintf)(const char *format, ...) = NULL;
#if defined(__LITEOS_RISCV__)
    pfnPrintf = &AtPrintf;
#else
    pfnPrintf = &printf;
#endif

    pfnPrintf("Begin dump syspara\r\n");
    pfnPrintf("=======================\r\n");
    while (index < dumpInfoItemNum) {
        temp = SYSPARA_LIST[index].getInfoValue();
        pfnPrintf("%s:%s\r\n", SYSPARA_LIST[index].infoName, temp);
        free(temp);
        temp = NULL;
        index++;
    }
    pfnPrintf("=======================\r\n");
    pfnPrintf("End dump syspara\r\n");
    pfnPrintf = NULL;
    return 0;
}
