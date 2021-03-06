/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "devhost_service.h"
#include "devmgr_service_clnt.h"
#include "devsvc_manager_clnt.h"
#include "hdf_base.h"
#include "hdf_device_node_ext.h"
#include "hdf_driver_loader.h"
#include "hdf_io_service.h"
#include "hdf_log.h"
#include "hdf_object_manager.h"
#include "osal_mem.h"

#define HDF_LOG_TAG devhost_service

static struct HdfDevice *DevHostServiceFindDevice(struct DevHostService *inst, uint16_t deviceId)
{
    struct HdfSListIterator it;
    struct HdfDevice *deviceNode = NULL;
    if (inst == NULL) {
        HDF_LOGE("Find driver failed, inst is null");
        return NULL;
    }

    HdfSListIteratorInit(&it, &inst->devices);
    while (HdfSListIteratorHasNext(&it)) {
        deviceNode = (struct HdfDevice *)HDF_SLIST_CONTAINER_OF(
            struct HdfSListNode, HdfSListIteratorNext(&it), struct HdfDevice, node);
        if (deviceNode->deviceId == deviceId) {
            return deviceNode;
        }
    }
    return NULL;
}

static void DevHostServiceFreeDevice(struct DevHostService *inst, uint16_t deviceId)
{
    struct HdfDevice *deviceNode = DevHostServiceFindDevice(inst, deviceId);
    if (deviceNode != NULL) {
        HdfSListRemove(&inst->devices, &deviceNode->node);
        HdfDeviceFreeInstance(deviceNode);
    }
}

static struct HdfDevice *DevHostServiceGetDevice(struct DevHostService *inst, uint16_t deviceId)
{
    struct HdfDevice *device = DevHostServiceFindDevice(inst, deviceId);
    if (device == NULL) {
        device = HdfDeviceNewInstance();
        if (device == NULL) {
            HDF_LOGE("Dev host service create driver instance failed");
            return NULL;
        }
        device->hostId = inst->hostId;
        device->deviceId = deviceId;
        HdfSListAdd(&inst->devices, &device->node);
    }
    return device;
}

int DevHostServiceAddDevice(struct IDevHostService *inst, const struct HdfDeviceInfo *deviceInfo)
{
    int ret = HDF_FAILURE;
    struct HdfDevice *device = NULL;
    struct HdfDeviceNode *devNode = NULL;
    struct DevHostService *hostService = (struct DevHostService *)inst;
    struct IDriverLoader *driverLoader =  HdfDriverLoaderGetInstance();

    if ((deviceInfo == NULL) || (driverLoader == NULL) || (driverLoader->LoadNode == NULL)) {
        HDF_LOGE("Add device failed, input param is null");
        return ret;
    }

    device = DevHostServiceGetDevice(hostService, deviceInfo->deviceId);
    if (device == NULL || device->super.Attach == NULL) {
        ret = HDF_DEV_ERR_NO_MEMORY;
        goto error;
    }

    devNode = driverLoader->LoadNode(driverLoader, deviceInfo);
    if (devNode == NULL) {
        ret = HDF_DEV_ERR_NO_DEVICE_SERVICE;
        goto error;
    }
    devNode->hostService = hostService;
    ret = device->super.Attach(&device->super, devNode);
    if (ret != HDF_SUCCESS) {
        goto error;
    }
    return HDF_SUCCESS;

error:
    if (!HdfSListIsEmpty(&hostService->devices)) {
        DevHostServiceFreeDevice(hostService, deviceInfo->deviceId);
    }
    return ret;
}

static int DevHostServiceDelDevice(struct IDevHostService *inst, const struct HdfDeviceInfo *deviceInfo)
{
    struct HdfDevice *device = NULL;
    struct DevHostService *hostService = (struct DevHostService *)inst;
    struct IDriverLoader *driverLoader =  HdfDriverLoaderGetInstance();

    if ((deviceInfo == NULL) || (driverLoader == NULL) || (driverLoader->UnLoadNode == NULL)) {
        HDF_LOGE("Add device failed, input param is null");
        return HDF_FAILURE;
    }

    device = DevHostServiceGetDevice(hostService, deviceInfo->deviceId);
    if (device == NULL) {
        HDF_LOGW("Del device failed, device is not exist");
        return HDF_SUCCESS;
    }

    driverLoader->UnLoadNode(driverLoader, deviceInfo);
    struct HdfSListIterator it;
    struct HdfDeviceNode *deviceNode = NULL;
    HdfSListIteratorInit(&it, &device->services);
    while (HdfSListIteratorHasNext(&it)) {
        deviceNode = (struct HdfDeviceNode *)HDF_SLIST_CONTAINER_OF(
            struct HdfSListNode, HdfSListIteratorNext(&it), struct HdfDeviceNode, entry);
        if ((strcmp(deviceNode->deviceInfo->moduleName, deviceInfo->moduleName) == 0) &&
            (strcmp(deviceNode->deviceInfo->svcName, deviceInfo->svcName) == 0)) {
            struct DeviceNodeExt *deviceNodeExt = (struct DeviceNodeExt *)deviceNode;
            HdfSListRemove(&device->services, &deviceNode->entry);
            DeviceNodeExtRelease(&deviceNodeExt->super.super.object);
        }
    }

    if (!HdfSListIsEmpty(&hostService->devices)) {
        DevHostServiceFreeDevice(hostService, device->deviceId);
    }
    DevSvcManagerClntRemoveService(deviceInfo->svcName);
    return HDF_SUCCESS;
}

static int DevHostServiceStartService(struct IDevHostService *service)
{
    struct DevHostService *hostService = (struct DevHostService*)service;
    if (hostService == NULL) {
        HDF_LOGE("Start device service failed, hostService is null");
        return HDF_FAILURE;
    }
    return DevmgrServiceClntAttachDeviceHost(hostService->hostId, service);
}

void DevHostServiceConstruct(struct DevHostService *service)
{
    struct IDevHostService *hostServiceIf = &service->super;
    if (hostServiceIf != NULL) {
        hostServiceIf->AddDevice = DevHostServiceAddDevice;
        hostServiceIf->DelDevice = DevHostServiceDelDevice;
        hostServiceIf->StartService = DevHostServiceStartService;
        HdfSListInit(&service->devices);
        HdfServiceObserverConstruct(&service->observer);
    }
}

void DevHostServiceDestruct(struct DevHostService *service)
{
    HdfSListFlush(&service->devices, HdfDeviceDelete);
    HdfServiceObserverDestruct(&service->observer);
}

struct HdfObject *DevHostServiceCreate()
{
    struct DevHostService *devHostService = (struct DevHostService *)OsalMemCalloc(sizeof(struct DevHostService));
    if (devHostService != NULL) {
        DevHostServiceConstruct(devHostService);
    }
    return (struct HdfObject *)devHostService;
}

void DevHostServiceRelease(struct HdfObject *object)
{
    struct DevHostService *devHostService = (struct DevHostService *)object;
    if (devHostService != NULL) {
        DevHostServiceDestruct(devHostService);
        OsalMemFree(devHostService);
    }
}

struct IDevHostService *DevHostServiceNewInstance(uint16_t hostId, const char *hostName)
{
    struct DevHostService *hostService =
        (struct DevHostService *)HdfObjectManagerGetObject(HDF_OBJECT_ID_DEVHOST_SERVICE);
    if (hostService != NULL) {
        hostService->hostId = hostId;
        hostService->hostName = hostName;
    }
    return (struct IDevHostService *)hostService;
}

void DevHostServiceFreeInstance(struct IDevHostService *service)
{
    if (service != NULL) {
        HdfObjectManagerFreeObject(&service->object);
    }
}

