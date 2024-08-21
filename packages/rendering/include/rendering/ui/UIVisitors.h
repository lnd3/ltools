#pragma once

#include "rendering/ui/UIContainer.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "implot/implot.h"
#include "implot/implot_internal.h"

namespace l::ui {

    class UIZoom : public UIVisitor {
    public:
        virtual bool Active(const InputState& input);
        virtual bool Visit(UIContainer& container, const InputState& input, const ContainerArea& parent);
    };

    class UIDrag : public UIVisitor {
    public:
        virtual bool Active(const InputState& input);
        virtual bool Visit(UIContainer& container, const InputState& input, const ContainerArea& parent);
    protected:
        bool mDragging = false;
        UIContainer* mCurrentContainer = nullptr;
    };

    class UIMove : public UIVisitor {
    public:
        virtual bool Active(const InputState& input);
        virtual bool Visit(UIContainer& container, const InputState& input, const ContainerArea& parent);
    protected:
        bool mMoving = false;
        UIContainer* mCurrentContainer = nullptr;
    };

    class UIResize : public UIVisitor {
    public:
        virtual bool Visit(UIContainer& container, const InputState& input, const ContainerArea& parent);
    protected:
        bool mResizing = false;
        float mResizeAreaSize = 8.0f;
        UIContainer* mCurrentContainer = nullptr;
    };

    class UIDraw : public UIVisitor {
    public:
        UIDraw(ImDrawList* drawList) : mDrawList(drawList) {}
        ~UIDraw() = default;

        void DebugLog();
        virtual bool Visit(UIContainer& container, const InputState& input, const ContainerArea& parent);
    protected:
        ImDrawList* mDrawList;
        bool mDebugLog = false;
    };
}
