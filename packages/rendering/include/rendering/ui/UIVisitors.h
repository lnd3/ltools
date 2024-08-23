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

    class UIUpdate : public UIVisitor {
    public:
        UIUpdate() {}
        ~UIUpdate() = default;

        virtual bool ShouldUpdateContainer();
    };

    class UIZoom : public UIVisitor {
    public:
        virtual bool Active(UIContainer& container, const InputState& input);
        virtual bool Visit(UIContainer& container, const InputState& input, const ContainerArea& parent);
    };

    class UIDrag : public UIVisitor {
    public:
        virtual bool Active(UIContainer& container, const InputState& input);
        virtual bool Visit(UIContainer& container, const InputState& input, const ContainerArea& parent);
    protected:
        bool mDragging = false;
        UIContainer* mSourceContainer = nullptr;
    };

    class UIMove : public UIVisitor {
    public:
        virtual bool Active(UIContainer& container, const InputState& input);
        virtual bool Visit(UIContainer& container, const InputState& input, const ContainerArea& parent);
    protected:
        bool mMoving = false;
        UIContainer* mSourceContainer = nullptr;
    };

    class UIResize : public UIVisitor {
    public:
        virtual bool Visit(UIContainer& container, const InputState& input, const ContainerArea& parent);
    protected:
        bool mResizing = false;
        float mResizeAreaSize = 8.0f;
        UIContainer* mSourceContainer = nullptr;
    };

    class UIDraw : public UIVisitor {
    public:
        UIDraw(ImDrawList* drawList) : mDrawList(drawList) {}
        ~UIDraw() = default;

        virtual bool Visit(UIContainer& container, const InputState& input, const ContainerArea& parent);
    protected:
        ImDrawList* mDrawList;
    };

    class UILinkIO : public UIVisitor {
    public:
        virtual bool Active(UIContainer& container, const InputState& input);

        UILinkIO(UICreator* creator = nullptr) : mCreator(creator) {}
        ~UILinkIO() = default;

        virtual bool Visit(UIContainer& container, const InputState& input, const ContainerArea& parent);
    protected:
        bool mDragging = false;
        bool mPossibleLinkImminent = false;
        UIHandle<UIContainer> mLinkContainer;
        UICreator* mCreator = nullptr;
    };

}
