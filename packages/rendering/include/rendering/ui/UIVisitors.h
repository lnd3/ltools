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
        virtual bool Visit(UIContainer& container, const InputState& input);
    };

    class UIDrag : public UIVisitor {
    public:
        virtual bool Active(UIContainer& container, const InputState& input);
        virtual bool Visit(UIContainer& container, const InputState& input);
    protected:
        bool mDragging = false;
        UIContainer* mSourceContainer = nullptr;
    };

    class UIMove : public UIVisitor {
    public:
        virtual bool Active(UIContainer& container, const InputState& input);
        virtual bool Visit(UIContainer& container, const InputState& input);
    protected:
        bool mMoving = false;
        UIContainer* mSourceContainer = nullptr;
    };

    class UIResize : public UIVisitor {
    public:
        virtual bool Visit(UIContainer& container, const InputState& input);
    protected:
        bool mResizing = false;
        UIContainer* mSourceContainer = nullptr;
    };

    class UIDraw : public UIVisitor {
    public:
        UIDraw(ImDrawList* drawList) : mDrawList(drawList) {}
        ~UIDraw() = default;

        virtual bool Visit(UIContainer& container, const InputState& input);
    protected:
        ImDrawList* mDrawList;
    };

    class UILinkIO : public UIVisitor {
    public:
        using HandlerFunctionType = bool(int32_t, int32_t, int8_t, int8_t, bool);

        virtual bool Active(UIContainer& container, const InputState& input);

        UILinkIO(UICreator* creator, std::function<HandlerFunctionType> handler = nullptr) : mCreator(creator), mHandler(std::move(handler)) {}
        ~UILinkIO() = default;

        virtual bool Visit(UIContainer& container, const InputState& input);

        void SetHandler(std::function<HandlerFunctionType> handler) {
            mHandler = std::move(handler);
        }

    protected:
        bool mDragging = false;
        bool mPossibleLinkImminent = false;
        UIHandle<UIContainer> mLinkContainer;
        UICreator* mCreator = nullptr;

        std::function<HandlerFunctionType> mHandler;
    };

}
