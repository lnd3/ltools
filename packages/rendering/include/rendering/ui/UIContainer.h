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

    ImVec2 DragMovement(const ImVec2& prevPos, const ImVec2& curPos, float curScale);

    struct InputState {
        ImVec2 mRootPos;
        ImVec2 mCurPos;
        ImVec2 mPrevPos;
        float mScroll = 0.0f;
        bool mStarted = false;
        bool mStopped = false;

        ImVec2 GetLocalPos() const {
            return ImVec2(mCurPos.x - mRootPos.x, mCurPos.y - mRootPos.y);
        }
    };

    struct ContainerArea {
        ImVec2 mPosition;
        ImVec2 mSize;
        float mScale = 1.0f;

        ImVec2 GetPositionAtSize() const {
            return ImVec2(mPosition.x + mSize.x, mPosition.y + mSize.y);
        }

        ImVec2 Transform(const ImVec2& rootPos, const ImVec2& p) const {
            ImVec2 transformed;
            transformed.x = rootPos.x + mPosition.x + p.x * mScale;
            transformed.y = rootPos.y + mPosition.y + p.y * mScale;
            return transformed;
        }
    };

    bool Overlap(const ImVec2& p, const ImVec2& pMin, const ImVec2& pMax);
    bool Overlap(const ImVec2 rootPos, const ImVec2& p, const ImVec2& pMin, const ImVec2& pMax, const ContainerArea& parent);

    class UIContainer;

    class UIVisitor {
    public:
        virtual bool Visit(UIContainer&, const InputState&, const ContainerArea&) {
            return false;
        }
    };

    const uint32_t UIContainer_ReservedMask = 0x0000000f;
    const uint32_t UIContainer_ConfigMask = 0x000ffff0;
    const uint32_t UIContainer_CustomMask = 0xfff00000;

    const uint32_t UIContainer_VisitAll = 0x00000000;
    const uint32_t UIContainer_ExitOnAccept = 0x00000001;

    const uint32_t UIContainer_Reserved1 = 0x00000002;
    const uint32_t UIContainer_Reserved2 = 0x00000004;
    const uint32_t UIContainer_Reserved3 = 0x00000008;

    const uint32_t UIContainer_RenderFlag = 0x00000010;
    const uint32_t UIContainer_DragFlag = 0x00000020; // Can be dragged regardless of size
    const uint32_t UIContainer_ZoomFlag = 0x00000040; // Can be scaled, zoomed in/out
    const uint32_t UIContainer_MoveFlag = 0x00000080; // Can be moved when grabbed
    const uint32_t UIContainer_ResizeFlag = 0x00000100; // Can be resized when grabbing bottom right corner
    const uint32_t UIContainer_InputFlag = 0x00000200;
    const uint32_t UIContainer_OutputFlag = 0x00000400;


    class UIContainer {
    public:
        UIContainer(std::string name, uint32_t flags = 0) : mName(name), mConfigFlags(flags) {}
        ~UIContainer() = default;

        bool Accept(UIVisitor& visitor, const InputState& input, uint32_t flags = 0);
        bool Accept(UIVisitor& visitor, const InputState& input, const ContainerArea& parent, uint32_t flags = 0);
        void Add(UIContainer* container, int32_t i = -1);
        void Remove(int32_t i);
        void Move(ImVec2 localChange);
        void Resize(ImVec2 localChange);
        void Rescale(float localChange);
        void ClearNotifications();
        void Notification(uint32_t flag);
        bool HasNotification(uint32_t flag);
        void SetPosition(ImVec2 p);
        void SetSize(ImVec2 s);
        void SetContainerArea(const ContainerArea& area);
        ImVec2 GetPosition(bool untransformed = false);
        ImVec2 GetPositionAtSize(ImVec2 offset = ImVec2(), bool untransformed = false);
        ImVec2 GetSize(bool untransformed = false);
        float GetScale();
        void DebugLog();

    protected:
        std::string mName;
        ContainerArea mArea;
        uint32_t mConfigFlags = 0; // Active visitor flags
        uint32_t mNotificationFlags = 0; // Notification flags for ux feedback (resizing box animation etc)

        std::vector<UIContainer*> mContent;
    };

    class UIZoom : public UIVisitor {
    public:
        virtual bool Visit(UIContainer& container, const InputState& input, const ContainerArea& parent);
    };

    class UIDrag : public UIVisitor {
    public:
        virtual bool Visit(UIContainer& container, const InputState& input, const ContainerArea& parent);
    protected:
        bool mDragging = false;
        UIContainer* mCurrentContainer = nullptr;
    };

    class UIMove : public UIVisitor {
    public:
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
