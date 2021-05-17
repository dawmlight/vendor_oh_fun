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

#include "asm/platform.h"
#include "asm/io.h"
#include "soc/sys_ctrl.h"
#include "los_typedef.h"
#include "los_hwi.h"
#include "los_task_pri.h"
#include "los_spinlock.h"
#ifdef LOSCFG_DRIVERS_RANDOM
#include "soc/random.h"
#endif

UINT32 OsRandomStackGuard(VOID)
{
#ifdef LOSCFG_DRIVERS_RANDOM
    UINT32 stackGuard = 0;

    HiRandomHwInit();
    (VOID)HiRandomHwGetInteger(&stackGuard);
    HiRandomHwDeinit();
    return stackGuard;
#else
    return 0;
#endif
}

void OsReboot(void)
{
    writel(0xffffffff, (SYS_CTRL_REG_BASE + REG_SC_SYSRES));
}

void InitRebootHook(void)
{
    OsSetRebootHook(OsReboot);
}
