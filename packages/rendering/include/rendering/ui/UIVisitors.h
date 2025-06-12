#pragma once

#include "rendering/ui/UIContainer.h"
#include "rendering/ui/UIWindow.h"

#include <unordered_set>
#include <deque>

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

        void SetMoveHandler(std::function<void(int32_t containerId, int32_t nodeId, float x, float y)> handler) {
            mMoveHandler = handler;
        }
    protected:
        bool mMoving = false;
        UIContainer* mSourceContainer = nullptr;
        std::function<void(int32_t containerId, int32_t nodeId, float x, float y)> mMoveHandler = nullptr;
    };

    class UIResize : public UIVisitor {
    public:
        virtual bool Visit(UIContainer& container, const InputState& input);
        void Reset();

        void SetResizeHandler(std::function<void(int32_t containerId, int32_t nodeId, float width, float height)> handler) {
            mResizeHandler = handler;
        }
    protected:
        bool mResizing = false;
        UIContainer* mSourceContainer = nullptr;
        std::function<void(int32_t containerId, int32_t nodeId, float width, float height)> mResizeHandler = nullptr;
    };

    class UISelect : public UIVisitor {
    public:
        UISelect(UIManager& uiManager) : mUIManager(uiManager) {}

        virtual bool Visit(UIContainer& container, const InputState& input);

        void SetDeleteHandler(std::function<void(int32_t, int32_t)> handler) {
            mDeleteHandler = handler;
        }
        void SetRemoveHandler(std::function<void(int32_t)> handler) {
            mRemoveHandler = handler;
        }
    protected:
        std::unordered_set<UIContainer*> mSelectedContainers;
        UIManager& mUIManager;
        std::function<void(int32_t, int32_t)> mDeleteHandler = nullptr;
        std::function<void(int32_t)> mRemoveHandler = nullptr;
    };

    class UIEdit : public UIVisitor {
    public:
        virtual bool Visit(UIContainer& container, const InputState& input);

        void SetEditHandler(std::function<void(int32_t nodeId, int8_t channelId, float dx, float dy)> handler) {
            mEditHandler = handler;
        }

    protected:
        bool mEditing = false;
        UIContainer* mSourceContainer = nullptr;
        std::function<void(int32_t nodeId, int8_t channelId, float dx, float dy)> mEditHandler = nullptr;
    };

    class UIDraw : public UIVisitor {
    public:
        UIDraw(ImDrawList* drawList = nullptr) : mDrawList(drawList) {}
        ~UIDraw() = default;

        virtual bool Visit(UIContainer& container, const InputState& input);

        void SetDrawList(ImDrawList* drawList) {
            mDrawList = drawList;
        }

        void SetDrawChannelTextHandler(std::function<void(int32_t, int8_t, ImVec2, float, ImU32, ImDrawList*)> handler) {
            mDrawChannelTextHandler = handler;
        }

        void SetDrawLineHandler(std::function<void(int32_t, int8_t, ImVec2, ImVec2, float, ImU32, ImDrawList*)> handler) {
            mDrawLineHandler = handler;
        }

    protected:
        ImDrawList* mDrawList;
        std::function<void(int32_t, int8_t, ImVec2, float, ImU32, ImDrawList*)> mDrawChannelTextHandler = nullptr;
        std::function<void(int32_t, int8_t, ImVec2, ImVec2, float, ImU32, ImDrawList*)> mDrawLineHandler = nullptr;
    };

    class UILinkIO : public UIVisitor {
    public:
        using HandlerFunctionType = bool(int32_t, int32_t, int32_t, int32_t, bool);

        UILinkIO(UIManager& uiManager) : mUIManager(uiManager) {}
        ~UILinkIO() = default;

        virtual bool Active(UIContainer& container, const InputState& input);
        virtual bool Visit(UIContainer& container, const InputState& input);

        void SetLinkHandler(std::function<bool(int32_t linkInputId, int32_t linkOutputId, int32_t inputChannel, int32_t outputChannel, bool connected)> handler) {
            mLinkHandler = handler;
        }
    protected:
        bool mDragging = false;
        UIHandle mLinkContainer;
        UIManager& mUIManager;
        std::function<bool(int32_t linkInputId, int32_t linkOutputId, int32_t inputChannel, int32_t outputChannel, bool connected)> mLinkHandler = nullptr;
    };

}
