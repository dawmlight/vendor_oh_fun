/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include <securec.h>
#include "osal_cdev.h"
#include "osal_mem.h"
#include "devsvc_manager_clnt.h"
#include "hdf_base.h"
#include "hdf_log.h"
#include "event_hub.h"
#include "hdf_input_device_manager.h"

#define NODE_MODE            0660
#define SERVICE_NAME_LEN     16
#define INPUT_DEV_EXIST      1
#define INPUT_DEV_NOT_EXIST  0
#define MOUSE_DEV_ID         2
#define MAX_INPUT_DEV_NUM    32
#define INPUTDEV_FIRST_ID    1
#define FILLER_FLAG          1

InputManager *g_inputManager;

#ifndef __KERNEL__
int32_t TouchIoctl(InputDevice *inputdev, int32_t cmd, unsigned long arg);
uint32_t TouchPoll(struct file *filep, InputDevice *inputDev, poll_table *wait);
#endif

#ifndef __KERNEL__
static int32_t InputDevIoctl(struct file *filep, int32_t cmd, unsigned long arg)
{
    int32_t ret;
    InputDevice *inputdev = (InputDevice *)filep->f_inode->i_private;
    if (inputdev == NULL) {
        return HDF_FAILURE;
    }

    switch (inputdev->devType) {
        case INDEV_TYPE_TOUCH:
            ret = TouchIoctl(inputdev, cmd, arg);
            break;
        default:
            ret = 0;
            HDF_LOGE("%s: devType unknown, devType = %d", __func__, inputdev->devType);
            break;
    }
    return ret;
}

static int32_t InputDevOpen(struct file *filep)
{
    InputDevice *inputdev = (InputDevice *)filep->f_inode->i_private;
    if (inputdev == NULL) {
        HDF_LOGE("%s: filep is null", __func__);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

static int32_t InputDevClose(struct file *filep)
{
    InputDevice *inputdev = (InputDevice *)filep->f_inode->i_private;
    if (inputdev == NULL) {
        HDF_LOGE("%s: inputdev is null", __func__);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

#ifndef CONFIG_DISABLE_POLL
static int32_t InputDevPoll(struct file *filep, poll_table *wait)
{
    uint32_t pollMask = 0;
    InputDevice *inputdev = (InputDevice *)filep->f_inode->i_private;
    switch (inputdev->devType) {
        case INDEV_TYPE_TOUCH:
            pollMask = TouchPoll(filep, inputdev, wait);
            break;
        default:
            HDF_LOGE("%s: devType unknown, devType = %d", __func__, inputdev->devType);
            break;
    }
    return pollMask;
}
#endif

static const struct file_operations_vfs inputDevOps = {
    .open = InputDevOpen,
    .close = InputDevClose,
    .ioctl = InputDevIoctl,
#ifndef CONFIG_DISABLE_POLL
    .poll = InputDevPoll,
#endif
};

#endif

static bool IsHidDevice(uint32_t devType)
{
    if (devType == INDEV_TYPE_MOUSE) {
        return true;
    }
    return false;
}

static struct HdfDeviceObject *HidRegisterHdfDevice(InputDevice *inputDev)
{
    char svcName[SERVICE_NAME_LEN] = {0};
    const char *moduleName = "HDF_HID";
    struct HdfDeviceObject *hdfDev = NULL;
    int32_t ret = snprintf_s(svcName, SERVICE_NAME_LEN, strlen("event") + 1, "%s%u", "event", MOUSE_DEV_ID);
    if (ret < 0) {
        HDF_LOGE("%s: snprintf_s failed", __func__);
        return NULL;
    }

    hdfDev = HdfRegisterDevice(moduleName, svcName);
    if (hdfDev == NULL) {
        HDF_LOGE("%s: HdfRegisterDevice failed", __func__);
    }
    HDF_LOGI("%s: svcName is %s, devName = %s", __func__, svcName, inputDev->devName);
    return hdfDev;
}

static int32_t CreateDeviceNode(InputDevice *inputDev)
{
    if (IsHidDevice(inputDev->devType)) {
        HDF_LOGI("%s: prepare to register hdf device", __func__);
        inputDev->hdfDevObj = HidRegisterHdfDevice(inputDev);
        if (inputDev->hdfDevObj == NULL) {
            return HDF_DEV_ERR_NO_DEVICE;
        }
        inputDev->devId = MOUSE_DEV_ID;
    }

#ifndef __KERNEL__
    char devNode[INPUT_DEV_PATH_LEN] = {0};
    int32_t ret = snprintf_s(devNode, INPUT_DEV_PATH_LEN, strlen("/dev/input/event") + 1,
        "%s%u", "/dev/input/event", inputDev->devId);
    if (ret < 0) {
        HDF_LOGE("%s: snprintf_s failed", __func__);
        return HDF_FAILURE;
    }
    inputDev->devNode = devNode;
    ret = register_driver(inputDev->devNode, &inputDevOps, NODE_MODE, inputDev);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: register %s devnode failed, ret = %d", __func__, devNode, ret);
        inputDev->devNode = NULL;
        return ret;
    }
#endif

    HDF_LOGI("%s: create node succ, devId is %d ", __func__, inputDev->devId);
    return HDF_SUCCESS;
}

static void DeleteDeviceNode(InputDevice *inputDev)
{
    if (IsHidDevice(inputDev->devType)) {
        char svcName[SERVICE_NAME_LEN] = {0};
        const char *moduleName = "HDF_HID";
        int32_t ret = snprintf_s(svcName, SERVICE_NAME_LEN, strlen("event") + 1, "%s%u", "event", MOUSE_DEV_ID);
        if (ret < 0) {
            HDF_LOGE("%s: snprintf_s failed", __func__);
            return;
        }
        HDF_LOGI("%s: svcName is %s, devName = %s", __func__, svcName, inputDev->devName);
        HdfUnregisterDevice(moduleName, svcName);
    }

#ifndef __KERNEL__
    char devNode[INPUT_DEV_PATH_LEN] = {0};
    int32_t ret = snprintf_s(devNode, INPUT_DEV_PATH_LEN, strlen("/dev/input/event") + 1, "%s%u",
        "/dev/input/event", inputDev->devId);
    if (ret < 0) {
        HDF_LOGE("%s: snprintf_s failed", __func__);
        return;
    }
    inputDev->devNode = devNode;
    ret = unregister_driver(inputDev->devNode);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: delete dev node failed, ret %d", __func__, ret);
    }
#endif
    HDF_LOGI("%s: delete node succ, devId is %d", __func__, inputDev->devId);
}

static void AddInputDevice(InputDevice *inputDev)
{
    InputDevice *tmpDev = NULL;
    if (g_inputManager->inputDevList == NULL) {
        g_inputManager->inputDevList = inputDev;
        (g_inputManager->inputDevList)->next = NULL;
    } else {
        tmpDev = g_inputManager->inputDevList;
        while (tmpDev != NULL) {
            if (tmpDev->next == NULL) {
                tmpDev->next = inputDev;
                inputDev->next = NULL;
                break;
            }
            tmpDev = tmpDev->next;
        }
    }
    g_inputManager->devCount++;
}

static int32_t CheckInputDevice(InputDevice *inputDev)
{
    InputDevice *tmpDev = NULL;
    if (g_inputManager->inputDevList == NULL) {
        return HDF_SUCCESS;
    } else {
        tmpDev = g_inputManager->inputDevList;
        while (tmpDev != NULL) {
            if (tmpDev->devId == inputDev->devId) {
                HDF_LOGE("%s: device%d registered", __func__, inputDev->devId);
                return INPUT_DEV_EXIST;
            }
            tmpDev = tmpDev->next;
        }
    }
    return INPUT_DEV_NOT_EXIST;
}

static int32_t DeleteInputDevice(InputDevice *inputDev)
{
    if (g_inputManager->inputDevList == NULL) {
        return HDF_FAILURE;
    } else {
        if ((g_inputManager->inputDevList)->devId == inputDev->devId) {
            g_inputManager->inputDevList = g_inputManager->inputDevList->next;
            goto EXIT;
        }

        InputDevice *preNode = g_inputManager->inputDevList;
        InputDevice *tmpDev = preNode->next;
        while (tmpDev != NULL) {
            if (tmpDev->devId == inputDev->devId) {
                preNode->next = tmpDev->next;
                goto EXIT;
            }
            preNode = tmpDev;
            tmpDev = tmpDev->next;
        }
        HDF_LOGE("%s: device%d not exist", __func__, inputDev->devId);
        return HDF_FAILURE;
    }

EXIT:
    g_inputManager->devCount--;
    return HDF_SUCCESS;
}

#define DEFAULT_TOUCH_BUF_PKG_NUM      50
#define DEFAULT_KEY_BUF_PKG_NUM        10
#define DEFAULT_MOUSE_BUF_PKG_NUM      30
#define DEFAULT_CROWN_BUF_PKG_NUM      20
#define DEFAULT_ENCODER_BUF_PKG_NUM    20

static int32_t AllocPackageBuffer(InputDevice *inputDev)
{
    uint16_t pkgNum;
    switch (inputDev->devType) {
        case INDEV_TYPE_TOUCH:
            pkgNum = DEFAULT_TOUCH_BUF_PKG_NUM;
            break;
        case INDEV_TYPE_KEY:
            pkgNum = DEFAULT_KEY_BUF_PKG_NUM;
            break;
        case INDEV_TYPE_MOUSE:
            pkgNum = DEFAULT_MOUSE_BUF_PKG_NUM;
            break;
        case INDEV_TYPE_CROWN:
            pkgNum = DEFAULT_CROWN_BUF_PKG_NUM;
            break;
        case INDEV_TYPE_ENCODER:
            pkgNum = DEFAULT_ENCODER_BUF_PKG_NUM;
            break;
        default:
            HDF_LOGE("%s: devType not exist", __func__);
            return HDF_FAILURE;
    }
    inputDev->pkgBuf = HdfSBufObtain(sizeof(EventPackage) * pkgNum);
    if (inputDev->pkgBuf == NULL) {
        HDF_LOGE("%s: malloc sbuf failed", __func__);
        return HDF_ERR_MALLOC_FAIL;
    }
    inputDev->pkgNum = pkgNum;
    return HDF_SUCCESS;
}

static uint32_t AllocDeviceID(InputDevice *inputDev)
{
    InputDevice *tmpDev = g_inputManager->inputDevList;
    uint32_t idList[MAX_INPUT_DEV_NUM + 1];
    uint32_t id;
    (void)memset_s(idList, (MAX_INPUT_DEV_NUM + 1) * sizeof(uint32_t), 0,
                            (MAX_INPUT_DEV_NUM + 1) * sizeof(uint32_t));
    while (tmpDev != NULL) {
        if (idList[tmpDev->devId] == 0) {
            idList[tmpDev->devId] = FILLER_FLAG;
        }
        tmpDev = tmpDev->next;
    }
    for (id = INPUTDEV_FIRST_ID; id < MAX_INPUT_DEV_NUM + 1; id++) {
        if (id == MOUSE_DEV_ID) {
            continue;
        }
        if (idList[id] == 0) {
            inputDev->devId = id;
            return HDF_SUCCESS;
        }
    }
    return HDF_FAILURE;
}

int32_t RegisterInputDevice(InputDevice *inputDev)
{
    int32_t ret;

    HDF_LOGI("%s: enter", __func__);
    if (inputDev == NULL) {
        HDF_LOGE("%s: inputdev is null", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    if (g_inputManager == NULL || g_inputManager->initialized == false) {
        HDF_LOGE("%s: dev manager is null or initialized failed", __func__);
        return HDF_FAILURE;
    }

    OsalMutexLock(&g_inputManager->mutex);
    ret = AllocDeviceID(inputDev);
    if (ret != HDF_SUCCESS) {
        goto EXIT;
    }
    ret = CreateDeviceNode(inputDev);
    if (ret != HDF_SUCCESS) {
        goto EXIT1;
    }

    ret = AllocPackageBuffer(inputDev);
    if (ret != HDF_SUCCESS) {
        goto EXIT1;
    }

    AddInputDevice(inputDev);
    OsalMutexUnlock(&g_inputManager->mutex);
    HDF_LOGI("%s: exit succ, devCount is %d", __func__, g_inputManager->devCount);
    return HDF_SUCCESS;

EXIT1:
    DeleteDeviceNode(inputDev);
EXIT:
    OsalMutexUnlock(&g_inputManager->mutex);
    return ret;
}

int32_t UnregisterInputDevice(InputDevice *inputDev)
{
    int ret = HDF_FAILURE;
    HDF_LOGI("%s: enter", __func__);
    if (inputDev == NULL) {
        HDF_LOGE("%s: inputdev is null", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    if (g_inputManager == NULL || g_inputManager->initialized == false) {
        HDF_LOGE("%s: dev manager is null or initialized failed", __func__);
        return HDF_FAILURE;
    }

    OsalMutexLock(&g_inputManager->mutex);
    if (CheckInputDevice(inputDev) == INPUT_DEV_NOT_EXIST) {
        HDF_LOGE("%s: dev%d not exist", __func__, inputDev->devId);
        goto EXIT;
    }

    DeleteDeviceNode(inputDev);
    OsalMemFree(inputDev->pkgBuf);
    inputDev->pkgBuf = NULL;
    ret = DeleteInputDevice(inputDev);
    if (ret != HDF_SUCCESS) {
        goto EXIT;
    }

    OsalMutexUnlock(&g_inputManager->mutex);
    HDF_LOGI("%s: exit succ, devCount is %d", __func__, g_inputManager->devCount);
    return HDF_SUCCESS;

EXIT:
    OsalMutexUnlock(&g_inputManager->mutex);
    return ret;
}

static uint32_t GetDeviceCount(void)
{
    HDF_LOGI("%s: devCount = %d", __func__, g_inputManager->devCount);
    return g_inputManager->devCount;
}

static int32_t HdfInputManagerBind(struct HdfDeviceObject *device)
{
    if (device == NULL) {
        HDF_LOGE("%s: device is null", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    static IInputManagerService managerService = {
        .getDeviceCount = GetDeviceCount,
    };

    device->service = &managerService.ioService;
    return HDF_SUCCESS;
}

static InputManager *InputManagerInstance(void)
{
    InputManager *manager = (InputManager *)OsalMemAlloc(sizeof(InputManager));
    if (manager == NULL) {
        HDF_LOGE("%s: instance input manager failed", __func__);
        return NULL;
    }
    (void)memset_s(manager, sizeof(InputManager), 0, sizeof(InputManager));
    return manager;
}

static int32_t HdfInputManagerInit(struct HdfDeviceObject *device)
{
    HDF_LOGI("%s: enter", __func__);
    int32_t ret;
    if (device == NULL) {
        HDF_LOGE("%s: device is null", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

#ifndef __KERNEL__
    ret = mkdir("/dev/input", DEFAULT_DIR_MODE);
    if ((ret < 0) && (errno != EEXIST)) {
        HDF_LOGE("%s: mkdir fail, ret %d, error = %d\n", __func__, ret, errno);
        return HDF_FAILURE;
    }
#endif

    g_inputManager = InputManagerInstance();
    if (g_inputManager == NULL) {
        return HDF_ERR_MALLOC_FAIL;
    }

    if (OsalMutexInit(&g_inputManager->mutex) != HDF_SUCCESS) {
        HDF_LOGE("%s: mutex init failed", __func__);
        OsalMemFree(g_inputManager);
        g_inputManager = NULL;
        return HDF_FAILURE;
    }
    g_inputManager->initialized = true;
    g_inputManager->hdfDevObj = device;
    HDF_LOGI("%s: exit succ", __func__);
    return HDF_SUCCESS;
}

static void HdfInputManagerRelease(struct HdfDeviceObject *device)
{
    if (device == NULL) {
        HDF_LOGE("%s: device is null", __func__);
        return;
    }
    if (g_inputManager != NULL) {
        OsalMutexDestroy(&g_inputManager->mutex);
        OsalMemFree(g_inputManager);
        g_inputManager = NULL;
    }
}

struct HdfDriverEntry g_hdfInputEntry = {
    .moduleVersion = 1,
    .moduleName = "HDF_INPUT_MANAGER",
    .Bind = HdfInputManagerBind,
    .Init = HdfInputManagerInit,
    .Release = HdfInputManagerRelease,
};

HDF_INIT(g_hdfInputEntry);
