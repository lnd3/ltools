#pragma once

#include "logging/LoggingAll.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

#include <functional>

namespace l::ui {

    class UIBase {
    public:
        UIBase() = default;
        virtual ~UIBase() = default;

        virtual void Show() = 0;
        virtual bool IsShowing() = 0;
        virtual void Update(double, float) {}
    };

    class UIWindow final : public UIBase {
    public:
        UIWindow(std::string_view windowName) : mWindowName(windowName), mPopupName(mWindowName + "Popup") {}
        ~UIWindow() = default;

        void SetWindowName(std::string_view windowName);

        void Show() override;
        bool IsShowing() override;

        void SetContentWindow(std::function<void(UIWindow&)> action);
        void SetPointerPopup(std::function<void()> popup);

        void Open();
        void Close();
        bool IsHovered();
        ImVec2 GetPosition();
        ImVec2 GetSize();
        void SetBgColor(ImVec4 bgColor);

    protected:
        std::string mWindowName;
        std::string mPopupName;
        ImGuiWindow* mWindowPtr = nullptr;
        bool mOpened = false; 
        bool mIsHovered = false;
        bool mIsFocused = false;
        std::function<void(UIWindow&)> mWindowFunction = nullptr;
        std::function<void()> mPointerPopupMenu = nullptr;
        bool mPopupOpen = false;

        ImVec4 mBgColor = ImVec4(0.01f, 0.01f, 0.01f, 1.0f);
    };

}
