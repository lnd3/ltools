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

        void SetAction(std::function<void()> action);
        void Open();
        bool IsShowing();
        bool IsHovered();
        ImVec2 GetPosition();
        ImVec2 GetSize();
        float GetScale();
        bool TryInput(const ImVec2& prevPos, const ImVec2& curPos, float zoom, bool, bool);
        bool TryPan(const ImVec2& prevPos, const ImVec2& curPos, bool moveStart, bool moveStop);
        bool TryScale(const ImVec2& scalePos, float scroll);
        ImVec2 Transform(ImVec2 p, bool toWorld = true);
        void Show();
    protected:
        ImGuiWindow* mWindowPtr;
        bool mOpened;
        bool mIsHovered;
        ImVec2 mContentPan;
        float mContentScale;
        bool mMoving;

        std::function<void()> mWindowFunction;
    };

}
