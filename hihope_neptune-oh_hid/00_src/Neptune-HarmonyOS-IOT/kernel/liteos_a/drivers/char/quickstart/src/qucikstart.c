/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 *    of conditions and the following disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "los_quick_start_pri.h"
#include "bits/ioctl.h"
#include "fcntl.h"
#include "linux/kernel.h"

#define QUICKSTART_IOC_MAGIC    'T'
#define QUICKSTART_INITSTEP2    _IO(QUICKSTART_IOC_MAGIC, 0)
#define QUICKSTART_UNREGISTER   _IO(QUICKSTART_IOC_MAGIC, 1)
#define QUICKSTART_NODE         "/dev/quickstart"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

static int QuickstartOpen(struct file *filep)
{
    return 0;
}

static int QuickstartClose(struct file *filep)
{
    return 0;
}

static void SystemInitStep2(void)
{
    static int once = 0;
    /* Only one call is allowed */
    if (once != 0) {
        return;
    }
    once = 1;

    unsigned int ret = OsSystemInitStep2();
    if (ret != LOS_OK) {
        PRINT_ERR("systemInitStep2 failed\n");
    }
}

static int QuickstartDevUnregister(void)
{
    return unregister_driver(QUICKSTART_NODE);
}

static ssize_t QuickstartIoctl(struct file *filep, int cmd, unsigned long arg)
{
    switch (cmd) {
        case QUICKSTART_INITSTEP2:
            SystemInitStep2();
            break;
        case QUICKSTART_UNREGISTER:
            QuickstartDevUnregister();
            break;

        default:
            break;
    }
    return 0;
}

static const struct file_operations_vfs g_quickstartDevOps = {
    QuickstartOpen,  /* open */
    QuickstartClose, /* close */
    NULL,  /* read */
    NULL, /* write */
    NULL,      /* seek */
    QuickstartIoctl,      /* ioctl */
    NULL,   /* mmap */
#ifndef CONFIG_DISABLE_POLL
    NULL,      /* poll */
#endif
    NULL,      /* unlink */
};

int DevQuickStartRegister(void)
{
    return register_driver(QUICKSTART_NODE, &g_quickstartDevOps, 0666, 0); /* 0666: file mode */
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
