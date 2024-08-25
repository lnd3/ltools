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
#include <unordered_map>

namespace l::ui {

    enum class UIAlignH {
        Left = 0,
        Center = 1,
        Right = 2
    };

    enum class UIAlignV {
        Top = 0,
        Middle = 1,
        Bottom = 2
    };

    enum class UILayoutH {
        Fixed = 0,
        Scaled = 1,
        Parent = 2
    };

    enum class UILayoutV {
        Fixed = 0,
        Scaled = 1,
        Parent = 2
    };

    enum class UIRenderType {
        Rect = 0,
        RectFilled = 1,
        Triangle = 2,
        TriangleFilled = 3,
        Circle = 4,
        CircleFilled = 5,
        Polygon = 6,
        PolygonFilled = 7,
        LinkH = 8,
        Text = 9,
        Texture = 10
    };

    enum class UITraversalMode {
        DFS = 0, // Depth first search, leaves first, root last
        BFS = 1, // Breadth first search, root first, leaves last
    };

    struct UIRenderData {
        UIRenderType mType = UIRenderType::Rect;
        ImU32 mColor = ImColor(ImVec4(1.0f, 0.4f, 0.4f, 1.0f));

        ImVec2 mData0;
        ImVec2 mData1;

        void SetColor(ImVec4 colf) {
            mColor = ImColor(colf);
        }
    };

    struct InputState {
        ImVec2 mCurPos;
        ImVec2 mPrevPos;
        float mScroll = 0.0f;
        bool mStarted = false;
        bool mStopped = false;

        ImVec2 GetLocalPos() const {
            return ImVec2(mCurPos.x, mCurPos.y);
        }
    };

    struct UILayoutData {
        UIAlignH mAlignH = UIAlignH::Left;
        UIAlignV mAlignV = UIAlignV::Top;
        UILayoutH mLayoutH = UILayoutH::Fixed;
        UILayoutV mLayoutV = UILayoutV::Fixed;
    };

    struct ContainerArea {
        ImVec2 mPosition;
        ImVec2 mSize = ImVec2(20.0f, 20.0f);
        float mScale = 1.0f;
        float mMargin = 3.0f;

        UILayoutData mLayout;
        UIRenderData mRender;

        ImVec2 GetPositionAtSize() const {
            return ImVec2(mPosition.x + mSize.x, mPosition.y + mSize.y);
        }

        // Used in visitors only and parent scale is already premultiplied
        ImVec2 Transform(const ImVec2& p, ImVec2 screenRootPos = ImVec2()) const {
            ImVec2 transformed;
            transformed.x = screenRootPos.x + mPosition.x + p.x * mScale;
            transformed.y = screenRootPos.y + mPosition.y + p.y * mScale;
            return transformed;
        }

        float GetWorldScale(float parentScale) {
            return parentScale * mScale;
        }

        // Used in ui container layout, this is where we premultiply parent scale
        ImVec2 GetWorldPos(float parentScale, ImVec2 parentPos) {
            ImVec2 worldPos;
            worldPos.x = parentPos.x + (mPosition.x + mMargin) * mScale * parentScale;
            worldPos.y = parentPos.y + (mPosition.y + mMargin) * mScale * parentScale;
            return worldPos;
        }

        ImVec2 GetWorldSize(float parentScale) const {
            ImVec2 worldSize;
            worldSize.x = (mSize.x - mMargin * 2.0f) * mScale * parentScale;
            worldSize.y = (mSize.y - mMargin * 2.0f) * mScale * parentScale;
            return worldSize;
        }

        ImVec2 GetLocalSize() const {
            ImVec2 localSize;
            localSize.x = mSize.x / mScale;
            localSize.y = mSize.y / mScale;
            return localSize;
        }

        ImVec2 GetWorldPosLayout(float parentScale, ImVec2 parentPos, ImVec2 contentSize, UIAlignH alignH, UIAlignV alignV) {
            ImVec2 worldPos;
            switch (alignH) {
            case UIAlignH::Left:
                worldPos.x = parentPos.x + (mPosition.x + mMargin) * mScale * parentScale;
                break;
            case UIAlignH::Center:
                worldPos.x = parentPos.x + (mPosition.x + mSize.x * 0.5f - contentSize.x * 0.5f) * mScale * parentScale;
                break;
            case UIAlignH::Right:
                worldPos.x = parentPos.x + (mPosition.x - mMargin + mSize.x - contentSize.x) * mScale * parentScale;
                break;
            }
            switch (alignV) {
            case UIAlignV::Top:
                worldPos.y = parentPos.y + (mPosition.y + mMargin) * mScale * parentScale;
                break;
            case UIAlignV::Middle:
                worldPos.y = parentPos.y + (mPosition.y + mSize.y * 0.5f - contentSize.y * 0.5f) * mScale * parentScale;
                break;
            case UIAlignV::Bottom:
                worldPos.y = parentPos.y + (mPosition.y - mMargin + mSize.y - contentSize.y) * mScale * parentScale;
                break;
            }
            return worldPos;
        }

        ImVec2 GetWorldSizeLayout(float parentScale) const {
            ImVec2 worldSize;
            worldSize.x = (mSize.x - mMargin) * mScale * parentScale;
            worldSize.y = (mSize.y - mMargin) * mScale * parentScale;
            return worldSize;
        }
    };

    ImVec2 DragMovement(const ImVec2& prevPos, const ImVec2& curPos, float curScale);
    bool Overlap(const ImVec2& p, const ImVec2& pMin, const ImVec2& pMax);
    bool Overlap(const ImVec2& p, const ImVec2& pMin, const ImVec2& pMax, const ContainerArea& parent);
    bool OverlapScreenRect(const ImVec2& p, const ImVec2& pCenter, const ImVec2& offset, const ContainerArea& parent);
    bool OverlapScreenCircle(const ImVec2& p, const ImVec2& pCenter, float radii, const ContainerArea& parent);
    bool OverlapCircle(const ImVec2& p, const ImVec2& pCenter, float radii);

    class UIContainer;

    class UIVisitor {
    public:
        virtual ~UIVisitor() = default;

        virtual bool Active(UIContainer&, const InputState&) {
            return true;
        }
        virtual bool Visit(UIContainer&, const InputState&) {
            return false;
        }
        virtual void Debug(bool on = true) {
            mDebug = on;
        }
        virtual bool ShouldUpdateContainer() {
            return false;
        }

    protected:
        bool mDebug = false;
    };

    const uint32_t UIContainer_ReservedMask = 0x0000000f;
    const uint32_t UIContainer_ConfigMask = 0x000ffff0;
    const uint32_t UIContainer_CustomMask = 0xfff00000;

    const uint32_t UIContainer_Reserved0 = 0x00000001;
    const uint32_t UIContainer_Reserved1 = 0x00000002;
    const uint32_t UIContainer_Reserved2 = 0x00000004;
    const uint32_t UIContainer_Reserved3 = 0x00000008;

    const uint32_t UIContainer_DrawFlag = 0x00000010;
    const uint32_t UIContainer_DragFlag = 0x00000020; // Can be dragged regardless of size
    const uint32_t UIContainer_ZoomFlag = 0x00000040; // Can be scaled, zoomed in/out
    const uint32_t UIContainer_MoveFlag = 0x00000080; // Can be moved when grabbed
    const uint32_t UIContainer_ResizeFlag = 0x00000100; // Can be resized when grabbing bottom right corner
    const uint32_t UIContainer_InputFlag = 0x00000200; // Can be grabbed and dropped on a container with output flag
    const uint32_t UIContainer_OutputFlag = 0x00000400; // Can be dropped a grabbed 
    const uint32_t UIContainer_LinkFlag = 0x00000800; // Can be dropped a grabbed 

    class UIDraw;

    template<class T, class = std::enable_if_t<std::is_base_of_v<UIContainer, T>>>
    class UIHandle {
    public:
        UIHandle() : mContainer(nullptr) {};
        UIHandle(std::string_view id, UIContainer* container) : mId(id), mContainer(container) {};
        ~UIHandle() = default;

        std::string mId;
        UIContainer* mContainer = nullptr;

        std::string_view id() {
            return mId;
        }

        void reset() {
            mId.clear();
            mContainer = nullptr;
        }

        UIContainer* get() {
            return mContainer;
        }

        T* operator->() {
            return reinterpret_cast<T*>(mContainer);
        }

        T& ref() {
            return &reinterpret_cast<T*>(mContainer);
        }
    };

    class UIContainer {
    public:
        UIContainer(uint32_t flags = 0, UIRenderType renderType = UIRenderType::Rect, UIAlignH alignH = UIAlignH::Left, UIAlignV alignV = UIAlignV::Top, UILayoutH layoutH = UILayoutH::Fixed, UILayoutV layoutV = UILayoutV::Fixed) : mConfigFlags(flags) {
            mDisplayArea.mRender.mType = renderType;
            mDisplayArea.mMargin = 3.0f;
            mDisplayArea.mLayout.mAlignH = alignH;
            mDisplayArea.mLayout.mAlignV = alignV;
            mDisplayArea.mLayout.mLayoutH = layoutH;
            mDisplayArea.mLayout.mLayoutV = layoutV;
            mLayoutArea.mMargin = 0.0f;
        }
        virtual ~UIContainer() = default;

        virtual bool Accept(UIVisitor& visitor, const InputState& input, UITraversalMode mode = UITraversalMode::BFS);
        virtual void Add(UIContainer* container, int32_t i = -1);

        template<class T>
        void Add(UIHandle<T>& handle, int32_t i = -1) {
            Add(handle.get(), i);
        }

        virtual void Remove(int32_t i);

        template<class T>
        void Remove(UIHandle<T>& handle) {
            for (auto it = mContent.begin(); it != mContent.end();it++) {
                auto containerPtr = *it;
                if (containerPtr == handle.get()) {
                    mContent.erase(it);
                    break;
                }
            }
        }

        void Move(ImVec2 localChange);
        void Resize(ImVec2 localChange);
        void Rescale(float localChange);
        void ClearNotifications();
        void Notification(uint32_t flag);
        bool HasNotification(uint32_t flag);
        bool HasConfigFlag(uint32_t flag);
        void SetScale(float scale);
        void SetPosition(ImVec2 p);
        void SetSize(ImVec2 s);
        void SetLayoutPosition(ImVec2 s);
        void SetLayoutSize(ImVec2 s);
        void SetDisplayName(std::string_view id);
        void SetId(std::string_view id);
        void SetContainerArea(const ContainerArea& area);
        void SetLayoutArea(const ContainerArea& area);
        void SetParent(UIContainer* input);
        void SetCoParent(UIContainer* input);
        UIContainer* GetParent();
        UIContainer* GetCoParent();
        ImVec2 GetPosition(bool untransformed = false) const;
        ImVec2 GetPositionAtCenter(ImVec2 offset = ImVec2(), bool untransformed = false);
        ImVec2 GetPositionAtSize(ImVec2 offset = ImVec2(), bool untransformed = false);
        ImVec2 GetSize(bool untransformed = false);
        float GetScale();
        ContainerArea& GetContainerArea();
        const ContainerArea& GetLayoutArea() const;

        void DebugLog();

        const UIRenderData& GetRenderData() const {
            return mDisplayArea.mRender;
        }

        const UILayoutData& GetLayoutData() const {
            return mDisplayArea.mLayout;
        }

        std::string_view GetDisplayName() {
            return mDisplayName;
        }

        std::string_view GetId() {
            return mId;
        }

    protected:
        std::string mId;
        std::string mDisplayName;
        uint32_t mConfigFlags = 0; // Active visitor flags
        uint32_t mNotificationFlags = 0; // Notification flags for ux feedback (resizing box animation etc)

        ContainerArea mDisplayArea;
        ContainerArea mLayoutArea;

        UIContainer* mParent = nullptr;
        UIContainer* mCoParent = nullptr; // when a container is influenced by two parent in a specific way defined by the type of container and the visitor
        std::vector<UIContainer*> mContent;
    };

    enum class UISplitMode {
        EqualSplitH = 0,
        EqualSplitV = 1,
        AppendH = 2,
        AppendV = 3,
        EqualResizeH = 4,
        EqualResizeV = 5
    };

    class UISplit : public UIContainer {
    public:
        UISplit(uint32_t flags, UISplitMode splitMode, UILayoutH layoutH, UILayoutV layoutV) : UIContainer(flags), mSplitMode(splitMode) {
            mDisplayArea.mRender.mType = UIRenderType::Rect;
            mDisplayArea.mMargin = 3.0f;
            mDisplayArea.mLayout.mLayoutH = layoutH;
            mDisplayArea.mLayout.mLayoutV = layoutV;
        }
        ~UISplit() = default;

        virtual bool Accept(UIVisitor& visitor, const InputState& input, UITraversalMode mode);
    protected:
        UISplitMode mSplitMode;
    };

    template<class T, class = std::enable_if_t<std::is_base_of_v<UIContainer, T>>>
    std::string CreateUniqueId() {
        static uint32_t mId = 0;
        std::string id;
        if constexpr (std::is_same_v<T, UIContainer>) {
            id += "UIContainer";
        }
        else if constexpr (std::is_same_v<T, UISplit>) {
            id += "UISplit";
        }
        else {
            id += "Unknown";
        }

        id += l::string::to_hex<uint32_t>(mId++, 4);

        return id;
    }

    class UICreator {
    public:
        UICreator() = default;
        ~UICreator() = default;

        UIHandle<UIContainer> CreateContainer(uint32_t flags, UIRenderType renderType = UIRenderType::Rect, UIAlignH alignH = UIAlignH::Left, UIAlignV alignV = UIAlignV::Top, UILayoutH layoutH = UILayoutH::Fixed, UILayoutV layoutV = UILayoutV::Fixed);
        UIHandle<UISplit> CreateSplit(uint32_t flags, UISplitMode splitMode = UISplitMode::AppendV, UILayoutH layoutH = UILayoutH::Fixed, UILayoutV layoutV = UILayoutV::Fixed);

    protected:
        std::unordered_map<std::string, std::unique_ptr<UIContainer>> mContainers;

    };
}
