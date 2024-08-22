#include "rendering/ui/UIContainer.h"

#include <memory>

namespace l::ui {

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

    bool Overlap(const ImVec2& p, const ImVec2& pMin, const ImVec2& pMax, const ContainerArea& parent) {
        ImVec2 pMinT = parent.Transform(pMin);
        ImVec2 pMaxT = parent.Transform(pMax);
        return pMinT.x < p.x && pMinT.y < p.y && pMaxT.x > p.x && pMaxT.y > p.y;
    }

    bool OverlapScreenRect(const ImVec2& p, const ImVec2& pCenter, const ImVec2& offset, const ContainerArea& parent) {
        ImVec2 pMinT = parent.Transform(pCenter, ImVec2(-offset.x, -offset.y));
        ImVec2 pMaxT = parent.Transform(pCenter, ImVec2(offset.x, offset.y));
        return pMinT.x < p.x && pMinT.y < p.y && pMaxT.x > p.x && pMaxT.y > p.y;
    }

    bool OverlapScreenCircle(const ImVec2& p, const ImVec2& pCenter, float radii, const ContainerArea& parent) {
        ImVec2 pT = parent.Transform(pCenter);
        ImVec2 d = ImVec2(pT.x - p.x, pT.y - p.y);
        return d.x*d.x + d.y*d.y < radii * radii;
    }

    void UIContainer::Add(UIContainer* container, int32_t i) {
        if (i < 0) {
            mContent.push_back(container);
        }
        else {
            ASSERT(static_cast<size_t>(i) < mContent.size());
            mContent.insert(mContent.begin() + i, container);
        }
    }

    void UIContainer::Remove(int32_t i) {
        ASSERT(i >= 0 && static_cast<size_t>(i) < mContent.size());
        mContent.erase(mContent.begin() + i);
    }

    void UIContainer::Move(ImVec2 localChange) {
        mArea.mPosition.x += localChange.x;
        mArea.mPosition.y += localChange.y;
    }

    void UIContainer::Resize(ImVec2 localChange) {
        mArea.mSize.x += localChange.x;
        mArea.mSize.y += localChange.y;
        mArea.mSize.x = mArea.mSize.x < 1.0f ? 1.0f : mArea.mSize.x;
        mArea.mSize.y = mArea.mSize.y < 1.0f ? 1.0f : mArea.mSize.y;
    }

    void UIContainer::Rescale(float localChange) {
        mArea.mScale *= localChange;
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

    void UIContainer::SetPosition(ImVec2 p) {
        mArea.mPosition = p;
    }

    void UIContainer::SetSize(ImVec2 s) {
        mArea.mSize = s;
    }

    void UIContainer::SetDisplayName(std::string_view displayName) {
        mDisplayName = displayName;
    }

    void UIContainer::SetId(std::string_view id) {
        mId = id;
    }


    void UIContainer::SetContainerArea(const ContainerArea& area) {
        mArea = area;
    }

    ImVec2 UIContainer::GetPosition(bool untransformed) {
        if (untransformed) {
            return mArea.mPosition;
        }
        return ImVec2(mArea.mPosition.x * mArea.mScale, mArea.mPosition.y * mArea.mScale);
    }

    ImVec2 UIContainer::GetPositionAtCenter(ImVec2 offset, bool untransformed) {
        if (untransformed) {
            return ImVec2(mArea.mPosition.x + mArea.mSize.x * 0.5f + offset.x, mArea.mPosition.y * 0.5f + mArea.mSize.y + offset.y);
        }
        return ImVec2((mArea.mPosition.x + mArea.mSize.x * 0.5f + offset.x) * mArea.mScale, (mArea.mPosition.y * 0.5f + mArea.mSize.y + offset.y) * mArea.mScale);
    }

    ImVec2 UIContainer::GetPositionAtSize(ImVec2 offset, bool untransformed) {
        if (untransformed) {
            return ImVec2(mArea.mPosition.x + mArea.mSize.x + offset.x, mArea.mPosition.y + mArea.mSize.y + offset.y);
        }
        return ImVec2((mArea.mPosition.x + mArea.mSize.x + offset.x) * mArea.mScale, (mArea.mPosition.y + mArea.mSize.y + offset.y) * mArea.mScale);
    }

    ImVec2 UIContainer::GetSize(bool untransformed) {
        if (untransformed) {
            return mArea.mSize;
        }
        return ImVec2(mArea.mSize.x * mArea.mScale, mArea.mSize.y * mArea.mScale);
    }

    float UIContainer::GetScale() {
        return mArea.mScale;
    }

    ContainerArea& UIContainer::GetContainerArea() {
        return mArea;
    }

    void UIContainer::DebugLog() {
        LOG(LogDebug) << "UIContainer: " << mDisplayName << ", [" << mArea.mScale << "][" << mArea.mPosition.x << ", " << mArea.mPosition.y << "][" << mArea.mSize.x << ", " << mArea.mSize.y << "]";
    }

    bool UIContainer::Accept(UIVisitor& visitor, const InputState& input, UITraversalMode mode) {
        ContainerArea current;
        if (visitor.Active(input)) {
            return Accept(visitor, input, current, mode);
        }
        return false;
    }

    bool UIContainer::Accept(UIVisitor& visitor, const InputState& input, const ContainerArea& parent, UITraversalMode mode) {
        //ContainerArea current;
        //current.mScale = mArea.GetWorldScale(parent.mScale);
        //current.mPosition = mArea.GetWorldPos(parent.mScale, parent.mPosition);
        //current.mSize = mArea.GetWorldSize(parent.mScale);
        auto& layout = GetContainerArea().mLayout;
        switch (layout.mLayoutH) {
        case UILayoutH::Fixed:
            break;
        case UILayoutH::Scaled:
            break;
        case UILayoutH::Parent:
            mArea.mSize.x = parent.GetLocalSize().x;
            break;
        }
        switch (layout.mLayoutV) {
        case UILayoutV::Fixed:
            break;
        case UILayoutV::Scaled:
            break;
        case UILayoutV::Parent:
            mArea.mSize.y = parent.GetLocalSize().y;
            break;
        }


        for (auto& content : mContent) {
            auto contentSize = content->GetSize();
            auto& contentLayout = content->GetContainerArea().mLayout;
            ContainerArea current;
            current.mScale = mArea.GetWorldScale(parent.mScale);
            current.mSize = mArea.GetWorldSizeLayout(parent.mScale);
            current.mPosition = mArea.GetWorldPosLayout(parent.mScale, parent.mPosition, contentSize, contentLayout.mAlignH, contentLayout.mAlignV);

            if (content->Accept(visitor, input, current, mode)) {
                if (mode == UITraversalMode::Once) {
                    return true;
                }
            }
        }

        if (visitor.Active(input)) {
            return visitor.Visit(*this, input, parent);
        }
        return false;
    }

    bool UISplit::Accept(UIVisitor& visitor, const InputState& input, const ContainerArea& parent, UITraversalMode mode) {
        // Since we can have multiple layouts in a container for different content, it will act as
        // an anchor rather than a container, therefore it has to align within it and size 

        auto& layout = GetContainerArea().mLayout;
        switch (layout.mLayoutH) {
        case UILayoutH::Fixed:
            break;
        case UILayoutH::Scaled:
            break;
        case UILayoutH::Parent:
            mArea.mSize.y = parent.GetLocalSize().y;
            break;
        }
        switch (layout.mLayoutV) {
        case UILayoutV::Fixed:
            break;
        case UILayoutV::Scaled:
            break;
        case UILayoutV::Parent:
            mArea.mSize.x = parent.GetLocalSize().x;
            break;
        }

        float contentCount = static_cast<float>(mContent.size());

        ContainerArea current;
        current.mScale = mArea.GetWorldScale(parent.mScale);
        current.mSize = mArea.GetWorldSize(parent.mScale);
        current.mPosition = mArea.GetWorldPos(parent.mScale, parent.mPosition);

        if (mHorizontalSplit) {
            current.mSize.x /= contentCount;
        }
        else {
            current.mSize.y /= contentCount;
        }

        for (auto& content : mContent) {
            if (content->Accept(visitor, input, current, mode)) {
                if (mode == UITraversalMode::Once) {
                    return true;
                }
            }

            if (mHorizontalSplit) {
                current.mPosition.x += current.mSize.x;
            }
            else {
                current.mPosition.y += current.mSize.y;
            }
        }

        if (visitor.Active(input)) {
            return visitor.Visit(*this, input, parent);
        }
        return false;
    }

}
