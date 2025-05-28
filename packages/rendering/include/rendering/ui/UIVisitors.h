#pragma once

#include "rendering/ui/UIContainer.h"
#include "rendering/ui/UIWindow.h"

#include <unordered_set>
#include <deque>

namespace l::nodegraph {
    class NodeGraphSchema;
}

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
        void Reset();
    protected:
        bool mDragging = false;
        UIContainer* mSourceContainer = nullptr;
    };

    class UIMove : public UIVisitor {
    public:
        virtual bool Active(UIContainer& container, const InputState& input);
        virtual bool Visit(UIContainer& container, const InputState& input);
        void Reset();
    protected:
        bool mMoving = false;
        UIContainer* mSourceContainer = nullptr;
    };

    class UIResize : public UIVisitor {
    public:
        virtual bool Visit(UIContainer& container, const InputState& input);
        void Reset();
    protected:
        bool mResizing = false;
        UIContainer* mSourceContainer = nullptr;
    };

    class UISelect : public UIVisitor {
    public:
        UISelect(UIManager& uiManager) : mUIManager(uiManager) {}

        virtual bool Visit(UIContainer& container, const InputState& input);

        void SetNGSchema(l::nodegraph::NodeGraphSchema* ngSchema) {
            mNGSchema = ngSchema;
            mSelectedContainers.clear();
        }
        void SetDeleteHandler(std::function<void(int32_t containerId, int32_t nodeId)> handler) {
            mDeleteEvent = handler;
        }
    protected:
        std::unordered_set<UIContainer*> mSelectedContainers;
        UIManager& mUIManager;
        l::nodegraph::NodeGraphSchema* mNGSchema = nullptr;
        std::function<void(int32_t containerId, int32_t nodeId)> mDeleteEvent = nullptr;
    };

    class UIEdit : public UIVisitor {
    public:
        virtual bool Visit(UIContainer& container, const InputState& input);

        void SetNGSchema(l::nodegraph::NodeGraphSchema* ngSchema) {
            mNGSchema = ngSchema;
            mSourceContainer = nullptr;
            mEditing = false;
        }
    protected:
        bool mEditing = false;
        UIContainer* mSourceContainer = nullptr;
        l::nodegraph::NodeGraphSchema* mNGSchema = nullptr;
    };

    class UIDraw : public UIVisitor {
    public:
        UIDraw(ImDrawList* drawList = nullptr) : mDrawList(drawList) {}
        ~UIDraw() = default;

        virtual bool Visit(UIContainer& container, const InputState& input);

        void SetDrawList(ImDrawList* drawList) {
            mDrawList = drawList;
        }

        void SetNGSchema(l::nodegraph::NodeGraphSchema* ngSchema) {
            mNGSchema = ngSchema;
        }

    protected:
        ImDrawList* mDrawList;
        l::nodegraph::NodeGraphSchema* mNGSchema = nullptr;
    };

    class UILinkIO : public UIVisitor {
    public:
        using HandlerFunctionType = bool(int32_t, int32_t, int32_t, int32_t, bool);

        UILinkIO(UIManager& uiManager) : mUIManager(uiManager) {}
        ~UILinkIO() = default;

        virtual bool Active(UIContainer& container, const InputState& input);
        virtual bool Visit(UIContainer& container, const InputState& input);

        bool LinkHandler(int32_t linkInputId, int32_t linkOutputId, int32_t inputChannel, int32_t outputChannel, bool connected);

        void SetNGSchema(l::nodegraph::NodeGraphSchema* ngSchema) {
            mNGSchema = ngSchema;
        }

    protected:
        bool mDragging = false;
        UIHandle mLinkContainer;
        UIManager& mUIManager;
        l::nodegraph::NodeGraphSchema* mNGSchema = nullptr;
    };

}
