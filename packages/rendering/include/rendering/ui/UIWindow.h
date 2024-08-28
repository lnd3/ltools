#pragma once

#include "logging/LoggingAll.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "implot/implot.h"
#include "implot/implot_internal.h"

#include <functional>

namespace l::ui {

    class UIWindow {
    public:
        UIWindow() : mWindowPtr(nullptr), mOpened(false), mIsHovered(false), mWindowFunction(nullptr), mContentScale(1.0f), mMoving(false) {}
        ~UIWindow() = default;

        void SetContentWindow(std::function<void()> action);
        void SetPointerPopup(std::function<void()> popup);

        void Open();
        bool IsShowing();
        bool IsHovered();
        ImVec2 GetPosition();
        ImVec2 GetSize();
        void Show();
        void SetBgColor(ImVec4 bgColor);
    protected:
        ImGuiWindow* mWindowPtr;
        bool mOpened;
        bool mIsHovered;
        std::function<void()> mWindowFunction;
        std::function<void()> mPointerPopupMenu;
        float mContentScale;
        bool mMoving;
        bool mPointerPopupOpen = false;

        ImVec4 mBgColor = ImVec4(0.01f, 0.01f, 0.01f, 1.0f);

        ImVec2 mContentPan;
    };

}
