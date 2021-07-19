/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef UI_FONT_ADAPTOR
#define UI_FONT_ADAPTOR

#include "common/typed_text.h"
#include "graphic_config.h"

namespace OHOS {
class UIFontAdaptor : public HeapBase {
public:
    static uint32_t GetNextLineAndWidth(const char* txt, int16_t letterSpace, int16_t& maxWidth,
                                        bool allBreak = false);

    static bool IsSameTTFId(uint8_t fontId, uint32_t unicode);
};
} // namespace OHOS
#endif // UI_FONT_ADAPTOR
