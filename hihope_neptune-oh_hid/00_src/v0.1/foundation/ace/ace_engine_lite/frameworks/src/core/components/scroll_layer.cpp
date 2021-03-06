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

#include "scroll_layer.h"
#include "ace_log.h"
#include "component.h"
#include "component_utils.h"
#include "fatal_handler.h"
#include "root_view.h"

namespace OHOS {
namespace ACELite {
ScrollLayer::ScrollLayer() : scroll_(nullptr), pageRootView_(nullptr) {}

ScrollLayer::~ScrollLayer()
{
    if (scroll_ != nullptr) {
        delete (scroll_);
        scroll_ = nullptr;
    }
    pageRootView_ = nullptr;
    FatalHandler::GetInstance().SetCurrentPageRootView(nullptr);
}

UIScrollView *ScrollLayer::AddScrollLayer(UIView &view) const
{
    int16_t viewWidth = view.GetWidth();
    int16_t viewHeight = view.GetHeight();
    int16_t viewBorderWidth = view.GetStyle(STYLE_BORDER_WIDTH);
    int16_t scrollWidth = viewWidth + viewBorderWidth + viewBorderWidth;
    int16_t scrollHeight = viewHeight + viewBorderWidth + viewBorderWidth;

    if (scrollWidth <= 0 || scrollHeight <= 0) {
        HILOG_ERROR(HILOG_MODULE_ACE, "Scroll Layer: Get scroll width or height failed.");
        return nullptr;
    }

    uint16_t horizontalResolution = GetHorizontalResolution();
    uint16_t verticalResolution = GetVerticalResolution();
    if (scrollWidth > horizontalResolution || scrollHeight > verticalResolution) {
        UIScrollView *scroll = new UIScrollView();
        if (scroll == nullptr) {
            HILOG_ERROR(HILOG_MODULE_ACE, "Scroll Layer: Create scroll view failed.");
            return nullptr;
        }

        if (scrollHeight > verticalResolution) {
            scrollHeight = verticalResolution;
        }
        if (scrollWidth > horizontalResolution) {
            scrollWidth = horizontalResolution;
        }

        scroll->SetPosition(0, 0);
        scroll->SetWidth(scrollWidth);
        scroll->SetHeight(scrollHeight);
        scroll->SetXScrollBarVisible(false);
        scroll->SetYScrollBarVisible(false);
        scroll->SetThrowDrag(true);
        scroll->Add(&view);
        scroll->SetReboundSize(0);
        return scroll;
    }
    return nullptr;
}

void ScrollLayer::AppendScrollLayer(Component *rootComponent)
{
    if (rootComponent == nullptr) {
        HILOG_ERROR(HILOG_MODULE_ACE, "Scroll Layer: AppendScrollLayer function parameter rootComponent error.");
        return;
    }

    uint16_t horizontalResolution = GetHorizontalResolution();
    uint16_t verticalResolution = GetVerticalResolution();
    ConstrainedParameter rootViewParam(horizontalResolution, verticalResolution);
    Component::BuildViewTree(rootComponent, nullptr, rootViewParam);
    // root view will be attached soon, invoke the callback
    rootComponent->OnViewAttached();

    UIView *view = rootComponent->GetComponentRootView();
    if (view == nullptr) {
        HILOG_ERROR(HILOG_MODULE_ACE, "Scroll Layer: Failed to get view from js object.");
        return;
    }

    scroll_ = AddScrollLayer(*view);
    pageRootView_ = (scroll_ == nullptr) ? view : scroll_;
    FatalHandler::GetInstance().SetCurrentPageRootView(pageRootView_);
}

void ScrollLayer::Hide() const
{
    RootView *rootView = RootView::GetInstance();
    if (rootView == nullptr) {
        HILOG_ERROR(HILOG_MODULE_ACE, "get rootView is nullptr");
        return;
    }
    DetachFromRootView();
    rootView->Invalidate();
}

void ScrollLayer::DetachFromRootView() const
{
    if (pageRootView_ == nullptr) {
        return;
    }
    RootView *rootView = RootView::GetInstance();
    if (rootView == nullptr) {
        return;
    }
    rootView->Remove(pageRootView_);
}

void ScrollLayer::Show() const
{
    RootView *rootView = RootView::GetInstance();
    if (rootView == nullptr) {
        HILOG_ERROR(HILOG_MODULE_ACE, "get rootView is nullptr");
        return;
    }
    rootView->SetPosition(0, 0);
    rootView->SetWidth(GetHorizontalResolution());
    rootView->SetHeight(GetVerticalResolution());
    rootView->Add(pageRootView_);
    rootView->Invalidate();
}
} // namespace ACELite
} // namespace OHOS
