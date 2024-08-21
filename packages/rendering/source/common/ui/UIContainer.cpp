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

    bool UIContainer::Accept(UIVisitor& visitor, const InputState& input, UITraversalMode mode) {
        ContainerArea current;
        if (visitor.Active(input)) {
            return Accept(visitor, input, current, mode);
        }
        return false;
    }

    bool UIContainer::Accept(UIVisitor& visitor, const InputState& input, const ContainerArea& parent, UITraversalMode mode) {
        ContainerArea current;
        current.mScale = mArea.GetWorldScale(parent.mScale);
        current.mPosition = mArea.GetWorldPos(parent.mScale, parent.mPosition);
        current.mSize = mArea.GetWorldSize(parent.mScale);

        for (auto& content : mContent) {

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

void UIContainer::DebugLog() {
    LOG(LogDebug) << "UIContainer: " << mName << ", [" << mArea.mScale << "][" << mArea.mPosition.x << ", " << mArea.mPosition.y << "][" << mArea.mSize.x << ", " << mArea.mSize.y << "]";
}


bool UIZoom::Active(const InputState& input) {
    return input.mScroll != 0;
}

bool UIZoom::Visit(UIContainer& container, const InputState& input, const ContainerArea& parent) {
    if (!container.HasConfigFlag(UIContainer_ZoomFlag)) {
        return false;
    }
    if (input.mScroll != 0.0f) {
        float scaleChange = 1.0f;
        float scaleDelta = 0.1f;
        scaleChange = 1.0f + scaleDelta * input.mScroll;
        if (input.mScroll < 0.0f) {
            scaleChange = 1.0f / (1.0f - scaleDelta * input.mScroll);
        }
        if ((container.GetScale() > 100.0f && scaleChange > 1.0f) || (container.GetScale() < 0.01f && scaleChange < 1.0f)) {
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

bool UIDrag::Active(const InputState& input) {
    return input.mStarted && !mDragging || mDragging;
}

bool UIDrag::Visit(UIContainer& container, const InputState& input, const ContainerArea& parent) {
    if (!container.HasConfigFlag(UIContainer_DragFlag)) {
        return false;
    }
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

bool UIMove::Active(const InputState& input) {
    return input.mStarted && !mMoving || mMoving;
}

bool UIMove::Visit(UIContainer& container, const InputState& input, const ContainerArea& parent) {
        if(!container.HasConfigFlag(UIContainer_MoveFlag)){
            return false;
        }
        if (input.mStarted && !mMoving) {
            if (Overlap(input.GetLocalPos(), container.GetPosition(), container.GetPositionAtSize(), parent)) {
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
        if (!container.HasConfigFlag(UIContainer_ResizeFlag)) {
            return false;
        }
        if (!mResizing) {
            const float radii = mResizeAreaSize * 0.5f;
            ImVec2 p = container.GetPositionAtSize();
            if (OverlapScreenRect(input.GetLocalPos(), p, ImVec2(radii, radii), parent)) {
                mCurrentContainer = &container;
                container.Notification(UIContainer_ResizeFlag);

                if (input.mStarted) {
                    mResizing = true;
                }
                else {
                    return true;
                }
            }
            else {
                mCurrentContainer = nullptr;
                container.ClearNotifications();
            }
        }
        if (mResizing && mCurrentContainer == &container) {
            ImVec2 move = DragMovement(input.mPrevPos, input.mCurPos, parent.mScale);
            container.Resize(move);

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
        if (!container.HasConfigFlag(UIContainer_DrawFlag)) {
            return false;
        }
        if (mDebugLog) {
            container.DebugLog();
        }

        ImU32 color = container.GetRenderData().mColor;
        ImVec2 pTopLeft = container.GetPosition();
        ImVec2 pCenter = container.GetPositionAtCenter();
        ImVec2 pLowRight = container.GetPositionAtSize();
        ImVec2 pSize = container.GetSize();
        ImVec2 p1 = parent.Transform(pTopLeft, input.mRootPos);
        ImVec2 p12 = parent.Transform(pCenter, input.mRootPos);
        ImVec2 p2 = parent.Transform(pLowRight, input.mRootPos);

        switch (container.GetRenderData().mType) {
        case l::ui::UIRenderType::Rect:
            mDrawList->AddRect(p1, p2, color, 2.0f, ImDrawFlags_RoundCornersAll, 2.0f);
            break;
        case l::ui::UIRenderType::RectFilled:
            mDrawList->AddRectFilled(p1, p2, color, 2.0f, ImDrawFlags_RoundCornersAll);
            break;
        case l::ui::UIRenderType::Triangle:
            break;
        case l::ui::UIRenderType::TriangleFilled:
            break;
        case l::ui::UIRenderType::Circle:
            mDrawList->AddCircle(p1, pSize.x, color, 15, 2.0f);
            break;
        case l::ui::UIRenderType::CircleFilled:
            mDrawList->AddCircleFilled(p1, pSize.x, color, 15);
            break;
        case l::ui::UIRenderType::Polygon:
            break;
        case l::ui::UIRenderType::PolygonFilled:
            break;
        case l::ui::UIRenderType::Spline:
            break;
        case l::ui::UIRenderType::Text:
            break;
        case l::ui::UIRenderType::Texture:
            break;
        }

        switch (container.GetRenderData().mType) {
        case l::ui::UIRenderType::Rect:
        case l::ui::UIRenderType::RectFilled:
        case l::ui::UIRenderType::Texture:
            mDrawList->AddRect(p1, p2, color, 2.0f, ImDrawFlags_RoundCornersAll, 2.0f);

            if (container.HasConfigFlag(ui::UIContainer_ResizeFlag)) {
                ImVec2 p3 = parent.Transform(pLowRight, ImVec2(input.mRootPos.x - 3.0f, input.mRootPos.y - 3.0f));
                ImVec2 p4 = parent.Transform(pLowRight, ImVec2(input.mRootPos.x + 3.0f, input.mRootPos.y + 3.0f));
                if (container.HasNotification(ui::UIContainer_ResizeFlag)) {
                    p3 = parent.Transform(pLowRight, ImVec2(input.mRootPos.x - 5.0f, input.mRootPos.y - 5.0f));
                    p4 = parent.Transform(pLowRight, ImVec2(input.mRootPos.x + 5.0f, input.mRootPos.y + 5.0f));
                }
                mDrawList->AddRectFilled(p3, p4, color);
            }
            break;
        default:
            break;
        }

        return false;
    }

}
