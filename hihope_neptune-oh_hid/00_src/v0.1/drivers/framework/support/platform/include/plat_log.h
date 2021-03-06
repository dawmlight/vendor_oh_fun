/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef PLATFORM_LOG_H
#define PLATFORM_LOG_H

#include "hdf_log.h"

#if defined(__LITEOS__)
#define PLAT_LOGV(fmt, arg...)
#else
#define PLAT_LOGV(fmt, arg...) HDF_LOGV(fmt, ##arg)
#endif

#endif
