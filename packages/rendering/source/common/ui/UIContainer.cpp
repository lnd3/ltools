#include "rendering/ui/UIContainer.h"

#include <memory>

namespace l::ui {

    UIHandle<UIContainer> UICreator::CreateContainer(uint32_t flags, UIRenderType renderType, UIAlignH alignH, UIAlignV alignV, UILayoutH layoutH, UILayoutV layoutV) {
        std::unique_ptr<UIContainer> container = std::make_unique<UIContainer>(flags, renderType, alignH, alignV, layoutH, layoutV);

        std::string id = CreateUniqueId<UIContainer>();
        container->SetId(id);
        mContainers.insert({ id, std::move(container) });

        return UIHandle<UIContainer>{ id, mContainers.at(id).get() };
    }

    UIHandle<UISplit> UICreator::CreateSplit(uint32_t flags, UISplitMode splitMode, UILayoutH layoutH, UILayoutV layoutV) {
        std::unique_ptr<UISplit> container = std::make_unique<UISplit>(flags, splitMode, layoutH, layoutV);

        std::string id = CreateUniqueId<UISplit>();
        container->SetId(id);
        mContainers.insert({ id, std::move(container) });

        return UIHandle<UISplit>{ id, mContainers.at(id).get() };
    }

    ImVec2 DragMovement(const ImVec2& prevPos, const ImVec2& curPos, float curScale) {
        ImVec2 move = curPos;
        move.x -= prevPos.x;
        move.y -= prevPos.y;
        move.x = move.x / curScale;
        move.y = move.y / curScale;
        return move;
    }

    bool Overlap(const ImVec2& p, const ImVec2& pMin, const ImVec2& pMax) {
        return pMin.x < p.x && pMin.y < p.y && pMax.x > p.x && pMax.y > p.y;
    }

    bool Overlap(const ImVec2& p, const ImVec2& pMin, const ImVec2& pMax, const ContainerArea& area) {
        ImVec2 pMinT = area.Transform(pMin);
        ImVec2 pMaxT = area.Transform(pMax);
        return pMinT.x < p.x && pMinT.y < p.y && pMaxT.x > p.x && pMaxT.y > p.y;
    }

    bool OverlapScreenRect(const ImVec2& p, const ImVec2& pCenter, const ImVec2& screenOffset, const ContainerArea& area) {
        ImVec2 pMinT = area.Transform(pCenter, ImVec2(-screenOffset.x, -screenOffset.y));
        ImVec2 pMaxT = area.Transform(pCenter, ImVec2(screenOffset.x, screenOffset.y));
        return pMinT.x < p.x && pMinT.y < p.y && pMaxT.x > p.x && pMaxT.y > p.y;
    }

    bool OverlapScreenCircle(const ImVec2& p, const ImVec2& pCenter, float radii, const ContainerArea& area) {
        ImVec2 pT = area.Transform(pCenter);
        ImVec2 d = ImVec2(pT.x - p.x, pT.y - p.y);
        return d.x * d.x + d.y * d.y < radii * radii;
    }

    bool OverlapCircle(const ImVec2& p, const ImVec2& pCenter, float radii) {
        ImVec2 d = ImVec2(pCenter.x - p.x, pCenter.y - p.y);
        return d.x * d.x + d.y * d.y < radii * radii;
    }

    void UIContainer::Add(UIContainer* container, int32_t i) {
        if (i < 0) {
            mContent.push_back(container);
        }
        else {
            ASSERT(static_cast<size_t>(i) < mContent.size());
            mContent.insert(mContent.begin() + i, container);
        }
        container->SetParent(this);
    }

    void UIContainer::Remove(int32_t i) {
        ASSERT(i >= 0 && static_cast<size_t>(i) < mContent.size());
        mContent.at(i)->SetParent(nullptr);
        mContent.erase(mContent.begin() + i);
    }

    void UIContainer::Move(ImVec2 localChange) {
        mDisplayArea.mPosition.x += localChange.x;
        mDisplayArea.mPosition.y += localChange.y;
    }

    void UIContainer::Resize(ImVec2 localChange) {
        mDisplayArea.mSize.x += localChange.x;
        mDisplayArea.mSize.y += localChange.y;
        if (mDisplayArea.mRender.mType != UIRenderType::Spline) {
            mDisplayArea.mSize.x = mDisplayArea.mSize.x < 1.0f ? 1.0f : mDisplayArea.mSize.x;
            mDisplayArea.mSize.y = mDisplayArea.mSize.y < 1.0f ? 1.0f : mDisplayArea.mSize.y;
        }
    }

    void UIContainer::Rescale(float localChange) {
        mDisplayArea.mScale *= localChange;
    }

    void UIContainer::ClearNotifications() {
        mNotificationFlags = 0;
    }

    void UIContainer::Notification(uint32_t flag) {
        mNotificationFlags |= flag;
    }

    bool UIContainer::HasNotification(uint32_t flag) {
        return (mNotificationFlags & flag) == flag;
    }

    bool UIContainer::HasConfigFlag(uint32_t flag) {
        return (mConfigFlags & flag) == flag;
    }

    void UIContainer::SetScale(float scale) {
        mDisplayArea.mScale = scale;
    }

    void UIContainer::SetPosition(ImVec2 p) {
        mDisplayArea.mPosition = p;
    }

    void UIContainer::SetSize(ImVec2 s) {
        mDisplayArea.mSize = s;
    }

    void UIContainer::SetLayoutPosition(ImVec2 s) {
        mLayoutArea.mPosition = s;
    }

    void UIContainer::SetLayoutSize(ImVec2 s) {
        mLayoutArea.mSize = s;
    }

    void UIContainer::SetDisplayName(std::string_view displayName) {
        mDisplayName = displayName;
    }

    void UIContainer::SetId(std::string_view id) {
        mId = id;
    }

    void UIContainer::SetContainerArea(const ContainerArea& area) {
        mDisplayArea = area;
    }

    void UIContainer::SetLayoutArea(const ContainerArea& transformedLayoutArea) {
        mLayoutArea = transformedLayoutArea;
    }

    UIContainer* UIContainer::GetParent() {
        return mParent;
    }

    void UIContainer::SetParent(UIContainer* parent) {
        mParent = parent;
    }

    void UIContainer::SetCoParent(UIContainer* coParent) {
        mCoParent = coParent;
    }

    UIContainer* UIContainer::GetCoParent() {
        return mCoParent;
    }

    ImVec2 UIContainer::GetPosition(bool untransformed) {
        if (untransformed) {
            return mDisplayArea.mPosition;
        }
        return ImVec2(mDisplayArea.mPosition.x * mDisplayArea.mScale, mDisplayArea.mPosition.y * mDisplayArea.mScale);
    }

    ImVec2 UIContainer::GetPositionAtCenter(ImVec2 offset, bool untransformed) {
        if (untransformed) {
            return ImVec2(mDisplayArea.mPosition.x + mDisplayArea.mSize.x * 0.5f + offset.x, mDisplayArea.mPosition.y * 0.5f + mDisplayArea.mSize.y + offset.y);
        }
        return ImVec2((mDisplayArea.mPosition.x + mDisplayArea.mSize.x * 0.5f + offset.x) * mDisplayArea.mScale, (mDisplayArea.mPosition.y * 0.5f + mDisplayArea.mSize.y + offset.y) * mDisplayArea.mScale);
    }

    ImVec2 UIContainer::GetPositionAtSize(ImVec2 offset, bool untransformed) {
        if (untransformed) {
            return ImVec2(mDisplayArea.mPosition.x + mDisplayArea.mSize.x + offset.x, mDisplayArea.mPosition.y + mDisplayArea.mSize.y + offset.y);
        }
        return ImVec2((mDisplayArea.mPosition.x + mDisplayArea.mSize.x + offset.x) * mDisplayArea.mScale, (mDisplayArea.mPosition.y + mDisplayArea.mSize.y + offset.y) * mDisplayArea.mScale);
    }

    ImVec2 UIContainer::GetSize(bool untransformed) {
        if (untransformed) {
            return mDisplayArea.mSize;
        }
        return ImVec2(mDisplayArea.mSize.x * mDisplayArea.mScale, mDisplayArea.mSize.y * mDisplayArea.mScale);
    }

    float UIContainer::GetScale() {
        return mDisplayArea.mScale;
    }

    ContainerArea& UIContainer::GetContainerArea() {
        return mDisplayArea;
    }

    const ContainerArea& UIContainer::GetLayoutArea() const {
        return mLayoutArea;
    }

    void UIContainer::DebugLog() {
        LOG(LogDebug) << "UIContainer: " << mDisplayName << ", [" << mDisplayArea.mScale << "][" << mDisplayArea.mPosition.x << ", " << mDisplayArea.mPosition.y << "][" << mDisplayArea.mSize.x << ", " << mDisplayArea.mSize.y << "]";
    }

    bool UIContainer::Accept(UIVisitor& visitor, const InputState& input, UITraversalMode mode) {
        if (visitor.ShouldUpdateContainer()) {
            auto& layout = mDisplayArea.mLayout;
            switch (layout.mLayoutH) {
            case UILayoutH::Fixed:
                break;
            case UILayoutH::Scaled:
                break;
            case UILayoutH::Parent:
                mDisplayArea.mSize.x = mLayoutArea.GetLocalSize().x;
                break;
            }
            switch (layout.mLayoutV) {
            case UILayoutV::Fixed:
                break;
            case UILayoutV::Scaled:
                break;
            case UILayoutV::Parent:
                mDisplayArea.mSize.y = mLayoutArea.GetLocalSize().y;
                break;
            }
        }

        if (mode == UITraversalMode::BFS && visitor.Active(*this, input)) {
            if (visitor.Visit(*this, input)) {
                return true;
            }
        }

        size_t i = 0;
        for (auto& content : mContent) {
            if (visitor.ShouldUpdateContainer()) {
                auto contentSize = content->GetSize();
                auto& contentLayout = content->mDisplayArea.mLayout;
                ContainerArea current;
                current.mScale = mDisplayArea.GetWorldScale(mLayoutArea.mScale);
                current.mSize = mDisplayArea.GetWorldSizeLayout(mLayoutArea.mScale);
                current.mPosition = mDisplayArea.GetWorldPosLayout(mLayoutArea.mScale, mLayoutArea.mPosition, contentSize, contentLayout.mAlignH, contentLayout.mAlignV);
                content->SetLayoutArea(current);
            }

            if (content->Accept(visitor, input, mode)) {
                return true;
            }
            i++;
        }

        if (mode == UITraversalMode::DFS && visitor.Active(*this, input)) {
            return visitor.Visit(*this, input);
        }
        return false;
    }

    bool UISplit::Accept(UIVisitor& visitor, const InputState& input, UITraversalMode mode) {
        // Since we can have multiple layouts in a container for different content, it will act as
        // an anchor rather than a container, therefore it has to align within it and size 

        ContainerArea current;

        if (visitor.ShouldUpdateContainer()) {
            auto& layout = mDisplayArea.mLayout;
            switch (layout.mLayoutH) {
            case UILayoutH::Fixed:
                break;
            case UILayoutH::Scaled:
                break;
            case UILayoutH::Parent:
                mDisplayArea.mSize.x = mLayoutArea.GetLocalSize().x;
                break;
            }
            switch (layout.mLayoutV) {
            case UILayoutV::Fixed:
                break;
            case UILayoutV::Scaled:
                break;
            case UILayoutV::Parent:
                mDisplayArea.mSize.y = mLayoutArea.GetLocalSize().y;
                break;
            }

            float contentCount = static_cast<float>(mContent.size());
            current.mScale = mDisplayArea.GetWorldScale(mLayoutArea.mScale);
            current.mSize = mDisplayArea.GetWorldSize(mLayoutArea.mScale);
            current.mPosition = mDisplayArea.GetWorldPos(mLayoutArea.mScale, mLayoutArea.mPosition);

            switch (mSplitMode) {
            case UISplitMode::EqualSplitH:
                current.mSize.x /= contentCount;
                break;
            case UISplitMode::EqualSplitV:
                current.mSize.y /= contentCount;
                break;
            case UISplitMode::AppendH:
                break;
            case UISplitMode::AppendV:
                break;
            case UISplitMode::EqualResizeH:
                break;
            case UISplitMode::EqualResizeV:
                break;
            }
        }

        if (mode == UITraversalMode::BFS && visitor.Active(*this, input)) {
            if (visitor.Visit(*this, input)) {
                return true;
            }
        }

        size_t i = 0;
        for (auto& content : mContent) {
            if (visitor.ShouldUpdateContainer()) {
                content->SetLayoutArea(current);
            }

            if (content->Accept(visitor, input, mode)) {
                return true;
            }

            if (visitor.ShouldUpdateContainer()) {
                switch (mSplitMode) {
                case UISplitMode::EqualSplitH:
                    current.mPosition.x += current.mSize.x;
                    break;
                case UISplitMode::EqualSplitV:
                    current.mPosition.y += current.mSize.y;
                    break;
                case UISplitMode::AppendH:
                    current.mPosition.x += content->GetSize().x * mLayoutArea.mScale;
                    break;
                case UISplitMode::AppendV:
                    current.mPosition.y += content->GetSize().y * mLayoutArea.mScale;
                    break;
                case UISplitMode::EqualResizeH:
                    break;
                case UISplitMode::EqualResizeV:
                    break;
                }
            }
            i++;
        }

        if ((mode == UITraversalMode::DFS) && visitor.Active(*this, input)) {
            return visitor.Visit(*this, input);
        }
        return false;
    }

}
