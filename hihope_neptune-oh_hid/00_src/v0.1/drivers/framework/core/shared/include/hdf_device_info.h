/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef HDF_DEVICE_INFO_H
#define HDF_DEVICE_INFO_H

#include "hdf_device_desc.h"
#include "hdf_slist.h"

struct HdfDeviceInfo {
    struct HdfSListNode node;
    bool isDynamic;
    uint16_t hostId;
    uint16_t deviceId;
    uint16_t policy;
    uint16_t priority;
    uint16_t preload;
    uint16_t permission;
    const char *moduleName;
    const char *svcName;
    const char *deviceMatchAttr;
};

struct HdfDeviceInfo *HdfDeviceInfoNewInstance(void);
void HdfDeviceInfoConstruct(struct HdfDeviceInfo *deviceInfo);
void HdfDeviceInfoFreeInstance(struct HdfDeviceInfo *deviceInfo);
void HdfDeviceInfoDelete(struct HdfSListNode *listEntry);

#endif /* HDF_DEVICE_INFO_H */
