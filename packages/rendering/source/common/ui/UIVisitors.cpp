#include "rendering/ui/UIVisitors.h"

namespace l::ui {

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
    return (input.mStarted && !mDragging) || mDragging;
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
    return (input.mStarted && !mMoving) || mMoving;
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
            mDrawList->AddCircle(p12, pSize.x, color, 15, 2.0f);
            break;
        case l::ui::UIRenderType::CircleFilled:
            mDrawList->AddCircleFilled(p12, pSize.x, color, 15);
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
