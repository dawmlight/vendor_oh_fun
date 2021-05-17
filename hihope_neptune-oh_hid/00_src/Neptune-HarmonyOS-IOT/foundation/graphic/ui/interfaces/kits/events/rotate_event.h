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

#ifndef GRAPHIC_LITE_ROTATE_EVENT_H
#define GRAPHIC_LITE_ROTATE_EVENT_H
/**
 * @addtogroup Graphic
 * @{
 *
 * @brief Defines a lightweight graphics system that provides basic UI and container views,
 *        including buttons, images, labels, lists, animators, scroll views, swipe views, and layouts.
 *        This system also provides the Design for X (DFX) capability to implement features such as
 *        view rendering, animation, and input event distribution.
 *
 * @since 1.0
 * @version 1.0
 */

/**
 * @file rotate_event.h
 *
 * @brief ������ת�¼�RotateEvent������ť��Ӳ������
 *
 * @since 5.0
 * @version 3.0
 */

#include "event.h"

namespace OHOS {
/**
 * @brief ������ת�¼�RotateEvent������ť��Ӳ������
 *
 * @since 5.0
 * @version 3.0
 */
class RotateEvent : public Event {
public:
    RotateEvent() = delete;

    /**
     * @brief ��ת�¼�<b>RotateEvent</b>ʵ���Ĺ��캯��.
     * @param rotate short����ת��ֵ
     *
     * @since 5.0
     * @version 3.0
     */
    RotateEvent(int16_t rotate) : rotate_(-rotate * ROTATE_SENSITIVITY) {} // ��ת��ת����

    ~RotateEvent() {};

    /**
     * @brief ��ȡ��תֵ
     * @return ��תֵ
     *
     * @since 5.0
     * @version 3.0
     */
    int16_t GetRotate() const
    {
        return rotate_;
    }

private:
    int16_t rotate_;
};
} // namespace OHOS
#endif // GRAPHIC_LITE_ROTATE_EVENT_H