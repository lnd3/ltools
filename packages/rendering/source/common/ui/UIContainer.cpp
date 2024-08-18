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

    bool Overlap(const ImVec2 rootPos, const ImVec2& p, const ImVec2& pMin, const ImVec2& pMax, const ContainerArea& parent) {
        ImVec2 pMinT = parent.Transform(rootPos, pMin);
        ImVec2 pMaxT = parent.Transform(rootPos, pMax);
        return pMinT.x < p.x && pMinT.y < p.y && pMaxT.x > p.x && pMaxT.y > p.y;
    }

    bool UIContainer::Accept(UIVisitor& visitor, const InputState& input, uint32_t flags) {
        ContainerArea current;
        return Accept(visitor, input, current, flags);
    }

    bool UIContainer::Accept(UIVisitor& visitor, const InputState& input, const ContainerArea& parent, uint32_t flags) {
        ContainerArea current;
        // compute local scale
        current.mScale = mArea.mScale * parent.mScale;

        // scale local position with accumulated world scale and add to accumulated world position
        current.mPosition.x = (parent.mPosition.x + mArea.mPosition.x * mArea.mScale) * parent.mScale;
        current.mPosition.y = (parent.mPosition.y + mArea.mPosition.y * mArea.mScale) * parent.mScale;

        // scale local size with accumulated world scale
        current.mSize.x = mArea.mSize.x * mArea.mScale * parent.mScale;
        current.mSize.y = mArea.mSize.y * mArea.mScale * parent.mScale;

        bool exitOnAccept = (UIContainer_ExitOnAccept & flags) == UIContainer_ExitOnAccept;

        for (auto& content : mContent) {
            if (content->Accept(visitor, input, current, flags)) {
                if (exitOnAccept) {
                    return true;
                }
            }
        }

        if ((flags & mConfigFlags) == (flags & UIContainer_ConfigMask)) {
            return visitor.Visit(*this, input, parent);
        }
        return false;
    }

    void UIContainer::Add(UIContainer* container, int32_t i) {
        if (i < 0) {
            mContent.push_back(container);
        }
        else {
            ASSERT(i < mContent.size());
            mContent.insert(mContent.begin() + i, container);
        }
    }

    void UIContainer::Remove(int32_t i) {
        ASSERT(i >= 0 && i < mContent.size());
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

    void UIContainer::SetPosition(ImVec2 p) {
        mArea.mPosition = p;
    }

    void UIContainer::SetSize(ImVec2 s) {
        mArea.mSize = s;
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

    void UIContainer::DebugLog() {
        LOG(LogDebug) << "UIContainer: " << mName << ", [" << mArea.mScale << "][" << mArea.mPosition.x << ", " << mArea.mPosition.y << "][" << mArea.mSize.x << ", " << mArea.mSize.y << "]";
    }



    bool UIZoom::Visit(UIContainer& container, const InputState& input, const ContainerArea& parent) {
        if (input.mScroll != 0.0f) {
            float scaleChange = 1.0f;
            float scaleDelta = 0.1f;
            scaleChange = 1.0f + scaleDelta * input.mScroll;
            if (input.mScroll < 0.0f) {
                scaleChange = 1.0f / (1.0f - scaleDelta * input.mScroll);
            }
            if (container.GetScale() > 100.0f && scaleChange > 1.0f || container.GetScale() < 0.01f && scaleChange < 1.0f) {
                return true;
            }

            ImVec2 mousePos = input.GetLocalPos();
            ImVec2 localMousePos = ImVec2(parent.mPosition.x - mousePos.x, parent.mPosition.y - mousePos.y);
            ImVec2 p = container.GetPosition();
            container.Rescale(scaleChange);
            p.x = ((p.x + localMousePos.x) * scaleChange - localMousePos.x) / container.GetScale();
            p.y = ((p.y + localMousePos.y) * scaleChange - localMousePos.y) / container.GetScale();
            container.SetPosition(p);
            return true;
        }
        return false;
    }

    bool UIDrag::Visit(UIContainer& container, const InputState& input, const ContainerArea& parent) {
        if (input.mStarted && !mDragging) {
            if (input.GetLocalPos().x >= 0.0f && input.GetLocalPos().y >= 0.0f) {
                mDragging = true;
                mCurrentContainer = &container;
            }
        }
        if (mDragging && mCurrentContainer == &container) {
            ImVec2 move = DragMovement(input.mPrevPos, input.mCurPos, parent.mScale * container.GetScale());
            container.Move(move);
            container.Notification(UIContainer_DragFlag);

            if (input.mStopped) {
                mDragging = false;
                mCurrentContainer = nullptr;
            }
            return mDragging;
        }
        return false;
    }

    bool UIMove::Visit(UIContainer& container, const InputState& input, const ContainerArea& parent) {
        if (input.mStarted && !mMoving) {
            if (Overlap(ImVec2(), input.GetLocalPos(), container.GetPosition(), container.GetPositionAtSize(), parent)) {
                mMoving = true;
                mCurrentContainer = &container;
            }
        }
        if (mMoving && mCurrentContainer == &container) {
            ImVec2 move = DragMovement(input.mPrevPos, input.mCurPos, parent.mScale);
            container.Move(move);
            container.Notification(UIContainer_MoveFlag);

            if (input.mStopped) {
                mMoving = false;
                mCurrentContainer = nullptr;
            }
            return mMoving;
        }
        return false;
    }

    bool UIResize::Visit(UIContainer& container, const InputState& input, const ContainerArea& parent) {
        if (input.mStarted && !mResizing) {
            const float radii = mResizeAreaSize * 0.5f;
            ImVec2 p = container.GetPositionAtSize();
            ImVec2 p1 = ImVec2(p.x - radii, p.y - radii);
            ImVec2 p2 = ImVec2(p.x + radii, p.y + radii);
            if (Overlap(ImVec2(), input.GetLocalPos(), p1, p2, parent)) {
                mResizing = true;
                mCurrentContainer = &container;
            }
        }
        if (mResizing && mCurrentContainer == &container) {
            ImVec2 move = DragMovement(input.mPrevPos, input.mCurPos, parent.mScale);
            container.Resize(move);
            container.Notification(UIContainer_ResizeFlag);

            if (input.mStopped) {
                mResizing = false;
                mCurrentContainer = nullptr;
                container.ClearNotifications();
            }
            return mResizing;
        }
        return false;
    }

    void UIDraw::DebugLog() {
        mDebugLog = true;
    }

    bool UIDraw::Visit(UIContainer& container, const InputState& input, const ContainerArea& parent) {

        if (mDebugLog) {
            container.DebugLog();
        }

        ImVec2 p1 = parent.Transform(input.mRootPos, container.GetPosition());
        ImVec2 p2 = parent.Transform(input.mRootPos, container.GetPositionAtSize());

        ImVec4 colf = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
        const ImU32 col = ImColor(colf);

        mDrawList->AddRect(p1, p2, col, 2.0f, ImDrawFlags_RoundCornersAll, 2.0f);

        ImVec2 p3 = parent.Transform(input.mRootPos, container.GetPositionAtSize(ImVec2(-3.0f, -3.0f)));
        ImVec2 p4 = parent.Transform(input.mRootPos, container.GetPositionAtSize(ImVec2(3.0f, 3.0f)));
        if (container.HasNotification(ui::UIContainer_ResizeFlag)) {
            p3 = parent.Transform(input.mRootPos, container.GetPositionAtSize(ImVec2(-5.0f, -5.0f)));
            p4 = parent.Transform(input.mRootPos, container.GetPositionAtSize(ImVec2(5.0f, 5.0f)));
        }
        mDrawList->AddRectFilled(p3, p4, col);
        return false;
    }

}
