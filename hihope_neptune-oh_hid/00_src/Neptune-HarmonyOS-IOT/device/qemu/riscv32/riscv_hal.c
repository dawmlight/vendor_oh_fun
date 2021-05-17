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

#include "riscv_hal.h"
#include "los_debug.h"
#include "soc.h"
#include "plic.h"
#include "mtimer.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

VOID HalIrqDisable(UINT32 vector)
{
    if (vector <= RISCV_SYS_MAX_IRQ) {
        CLEAR_CSR(mie, 1 << vector);
    } else {
        PlicIrqDisable(vector);
    }
}

VOID HalIrqEnable(UINT32 vector)
{
    if (vector <= RISCV_SYS_MAX_IRQ) {
        SET_CSR(mie, 1 << vector);
    } else {
        PlicIrqEnable(vector);
    }
}

VOID HalSetLocalInterPri(UINT32 interPriNum, UINT16 prior)
{
    PlicIrqSetPrio(interPriNum, prior);
}

VOID HalGetSysCpuCycle(UINT32 *cntHi, UINT32 *cntLo)
{
    if ((cntHi == NULL) || (cntLo == NULL)) {
        return;
    }

    MTimerCpuCycle(cntHi, cntLo);
}

BOOL HalBackTraceFpCheck(UINT32 value)
{
    if (value >= (UINT32)(UINTPTR)(&__bss_end)) {
        return TRUE;
    }

    if ((value >= (UINT32)(UINTPTR)(&__start_and_irq_stack_top)) && (value < (UINT32)(UINTPTR)(&__except_stack_top))) {
        return TRUE;
    }

    return FALSE;
}

BOOL HalBackTraceRaCheck(UINT32 value)
{
    BOOL ret = FALSE;

    if ((value >= (UINT32)(UINTPTR)(&__text_start)) &&
        (value < (UINT32)(UINTPTR)(&__text_end))) {
        ret = TRUE;
    }

    return ret;
}

VOID HalClockInit(OS_TICK_HANDLER handler, UINT32 period)
{
    UINT32 ret;
    ret = MTimerTickInit(handler, period);
    if (ret != LOS_OK) {
        PRINT_ERR("Creat Mtimer failed! ret : 0x%x \n", ret);
        return;
    }

    PlicIrqInit();

    HalIrqEnable(RISCV_MACH_EXT_IRQ);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
