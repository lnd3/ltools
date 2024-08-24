#include "rendering/ui/UIVisitors.h"

namespace l::ui {

    bool UIUpdate::ShouldUpdateContainer() {
        return true;
    }

    bool UIZoom::Active(UIContainer&, const InputState& input) {
        return input.mScroll != 0;
    }

    bool UIZoom::Visit(UIContainer& container, const InputState& input) {
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

            auto& contentArea = container.GetLayoutArea();

            ImVec2 mousePos = input.GetLocalPos();
            ImVec2 localMousePos = ImVec2(contentArea.mPosition.x - mousePos.x, contentArea.mPosition.y - mousePos.y);
            ImVec2 p = container.GetPosition();
            container.Rescale(scaleChange);
            p.x = ((p.x + localMousePos.x) * scaleChange - localMousePos.x) / container.GetScale();
            p.y = ((p.y + localMousePos.y) * scaleChange - localMousePos.y) / container.GetScale();
            container.SetPosition(p);
            return true;
        }
        return false;
    }

    bool UIDrag::Active(UIContainer&, const InputState& input) {
        return (input.mStarted && !mDragging) || mDragging;
    }

    bool UIDrag::Visit(UIContainer& container, const InputState& input) {
        if (!container.HasConfigFlag(UIContainer_DragFlag)) {
            return false;
        }
        if (input.mStarted && !mDragging) {
            if (input.GetLocalPos().x >= 0.0f && input.GetLocalPos().y >= 0.0f) {
                mDragging = true;
                mSourceContainer = &container;
            }
        }
        if (mDragging && mSourceContainer == &container) {
            auto& contentArea = container.GetLayoutArea();

            ImVec2 move = DragMovement(input.mPrevPos, input.mCurPos, contentArea.mScale * container.GetScale());
            container.Move(move);
            container.Notification(UIContainer_DragFlag);

            if (input.mStopped) {
                mDragging = false;
                mSourceContainer = nullptr;
            }
            return mDragging;
        }
        return false;
    }

    bool UIMove::Active(UIContainer&, const InputState& input) {
        return (input.mStarted && !mMoving) || mMoving;
    }

    bool UIMove::Visit(UIContainer& container, const InputState& input) {
        if(!container.HasConfigFlag(UIContainer_MoveFlag)){
            return false;
        }
        if (input.mStarted && !mMoving) {
            auto& contentArea = container.GetLayoutArea();
            if (Overlap(input.GetLocalPos(), container.GetPosition(), container.GetPositionAtSize(), contentArea)) {
                mMoving = true;
                mSourceContainer = &container;
            }
        }
        if (mMoving && mSourceContainer == &container) {
            auto& contentArea = container.GetLayoutArea();
            ImVec2 move = DragMovement(input.mPrevPos, input.mCurPos, contentArea.mScale);
            container.Move(move);
            container.Notification(UIContainer_MoveFlag);

            if (input.mStopped) {
                mMoving = false;
                mSourceContainer = nullptr;
            }
            return mMoving;
        }
        return false;
    }

    bool UIResize::Visit(UIContainer& container, const InputState& input) {
        if (!container.HasConfigFlag(UIContainer_ResizeFlag)) {
            return false;
        }
        if (!mResizing) {
            const float radii = mResizeAreaSize * 0.5f;
            ImVec2 p = container.GetPositionAtSize();
            auto& contentArea = container.GetLayoutArea();
            if (OverlapScreenRect(input.GetLocalPos(), p, ImVec2(radii, radii), contentArea)) {
                mSourceContainer = &container;
                container.Notification(UIContainer_ResizeFlag);

                if (input.mStarted) {
                    mResizing = true;
                }
                else {
                    return true;
                }
            }
            else {
                mSourceContainer = nullptr;
                container.ClearNotifications();
            }
        }
        if (mResizing && mSourceContainer == &container) {
            auto& contentArea = container.GetLayoutArea();
            ImVec2 move = DragMovement(input.mPrevPos, input.mCurPos, contentArea.mScale);
            container.Resize(move);

            if (input.mStopped) {
                mResizing = false;
                mSourceContainer = nullptr;
                container.ClearNotifications();
            }
            return mResizing;
        }
        return false;
    }

    bool UIDraw::Visit(UIContainer& container, const InputState& input) {
        if (!mDebug && !container.HasConfigFlag(UIContainer_DrawFlag)) {
            return false;
        }

        auto& contentArea = container.GetLayoutArea();

        float splineThickness = 2.0f;
        ImU32 color = container.GetRenderData().mColor;
        ImVec2 pTopLeft = container.GetPosition();
        ImVec2 pCenter = container.GetPositionAtCenter();
        ImVec2 pLowRight = container.GetPositionAtSize();
        ImVec2 pSize = container.GetSize();
        pSize.x *= contentArea.mScale;
        pSize.y *= contentArea.mScale;
        ImVec2 p1 = contentArea.Transform(pTopLeft, input.mRootPos);
        ImVec2 p12 = contentArea.Transform(pCenter, input.mRootPos);
        ImVec2 p2 = contentArea.Transform(pLowRight, input.mRootPos);
        ImVec2 p11;
        ImVec2 p22;

        const char* nameStart;
        const char* nameEnd;

        switch (container.GetRenderData().mType) {
        case l::ui::UIRenderType::Rect:
            mDrawList->AddRect(p1, p2, color, 5.0f, ImDrawFlags_RoundCornersAll, 1.0f * container.GetScale() * contentArea.mScale);
            break;
        case l::ui::UIRenderType::RectFilled:
            mDrawList->AddRectFilled(p1, p2, color, 5.0f, ImDrawFlags_RoundCornersAll);
            break;
        case l::ui::UIRenderType::Triangle:
            break;
        case l::ui::UIRenderType::TriangleFilled:
            break;
        case l::ui::UIRenderType::Circle:
            mDrawList->AddCircle(p12, pSize.x, color, 15, 2.0f * container.GetScale() * contentArea.mScale);
            break;
        case l::ui::UIRenderType::CircleFilled:
            mDrawList->AddCircleFilled(p12, pSize.x, color, 15);
            break;
        case l::ui::UIRenderType::Polygon:
            break;
        case l::ui::UIRenderType::PolygonFilled:
            break;
        case l::ui::UIRenderType::Spline:
            if (container.HasConfigFlag(UIContainer_LinkFlag)) {
                splineThickness = container.HasNotification(UIContainer_LinkFlag) ? 2.0f * splineThickness : splineThickness;
                ImVec2 pLinkInput = container.GetParent()->GetPosition();
                p1 = contentArea.Transform(ImVec2(), input.mRootPos);

                if (container.GetCoParent() != nullptr) {
                    ImVec2 pLinkOutput = container.GetCoParent()->GetPosition();
                    auto& coContentArea = container.GetCoParent()->GetLayoutArea();
                    p2 = coContentArea.Transform(pLinkOutput, input.mRootPos);
                    float d12 = sqrt(0.25f*((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y)) / (contentArea.mScale * contentArea.mScale));

                    p11 = contentArea.Transform(ImVec2(pLinkInput.x + d12, pLinkInput.y), input.mRootPos);
                    p22 = coContentArea.Transform(ImVec2(pLinkOutput.x - d12, pLinkOutput.y), input.mRootPos);
                }
                else {
                    p2 = input.mCurPos;
                    float d12 = sqrt(0.25f * ((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y)) / (contentArea.mScale * contentArea.mScale));
                    p11 = contentArea.Transform(ImVec2(pLinkInput.x + d12, pLinkInput.y), input.mRootPos);
                    p22 = ImVec2(p2.x - d12 * contentArea.mScale, p2.y);
                }
            }
            else {
                float d12 = sqrt(0.25f * ((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y)) / (contentArea.mScale * contentArea.mScale));
                p11 = contentArea.Transform(ImVec2(pTopLeft.x + d12, pTopLeft.y), input.mRootPos);
                p22 = contentArea.Transform(ImVec2(pLowRight.x - d12, pLowRight.y), input.mRootPos);
            }
            mDrawList->AddBezierCubic(p1, p11, p22, p2, color, splineThickness, 30);
            break;
        case l::ui::UIRenderType::Text:

            //const ImVec2 text_pos(window->DC.CursorPos.x, window->DC.CursorPos.y + window->DC.CurrLineTextBaseOffset);
            //const float wrap_pos_x = window->DC.TextWrapPos;

            //     if (window->DC.CurrentColumns)

            if (!container.GetDisplayName().empty()) {
                nameStart = container.GetDisplayName().data();
                nameEnd = container.GetDisplayName().data() + container.GetDisplayName().size();
                container.SetSize(ImGui::CalcTextSize(nameStart, nameEnd));
                mDrawList->AddText(ImGui::GetDefaultFont(), 13.0f * container.GetScale() * contentArea.mScale, p1, color, nameStart, nameEnd);
            }
            break;
        case l::ui::UIRenderType::Texture:
            break;
        }

        if (mDebug && !container.GetId().empty()) {
            // also render name if it is non empty as debug text
            nameStart = container.GetId().data();
            nameEnd = container.GetId().data() + container.GetId().size();
            mDrawList->AddText(p1, color, nameStart, nameEnd);
        }

        switch (container.GetRenderData().mType) {
        case l::ui::UIRenderType::Rect:
        case l::ui::UIRenderType::RectFilled:
        case l::ui::UIRenderType::Texture:
        case l::ui::UIRenderType::Spline:
            if (container.HasConfigFlag(ui::UIContainer_ResizeFlag)) {
                ImVec2 p3 = contentArea.Transform(pLowRight, ImVec2(input.mRootPos.x - 3.0f, input.mRootPos.y - 3.0f));
                ImVec2 p4 = contentArea.Transform(pLowRight, ImVec2(input.mRootPos.x + 3.0f, input.mRootPos.y + 3.0f));
                if (container.HasNotification(ui::UIContainer_ResizeFlag)) {
                    p3 = contentArea.Transform(pLowRight, ImVec2(input.mRootPos.x - 5.0f, input.mRootPos.y - 5.0f));
                    p4 = contentArea.Transform(pLowRight, ImVec2(input.mRootPos.x + 5.0f, input.mRootPos.y + 5.0f));
                }
                mDrawList->AddRectFilled(p3, p4, color);
            }
            break;
        default:
            break;
        }

        return false;
    }

    bool UILinkIO::Active(UIContainer& container, const InputState&) {
        return container.HasConfigFlag(UIContainer_InputFlag) || container.HasConfigFlag(UIContainer_OutputFlag) || container.HasConfigFlag(UIContainer_LinkFlag);
    }

    bool UILinkIO::Visit(UIContainer& container, const InputState& input) {
        // Create link at from a clicked output container
        if (container.HasConfigFlag(UIContainer_OutputFlag) && !mDragging && input.mStarted && mLinkContainer.get() == nullptr) {
            ImVec2 pCenter = container.GetPosition();
            ImVec2 size = container.GetSize();
            auto& contentArea = container.GetLayoutArea();

            ImVec2 pT = contentArea.Transform(pCenter, input.mRootPos);
            if (OverlapCircle(input.mCurPos, pT, size.x * contentArea.mScale)) {
                mDragging = true;
                mLinkContainer = mCreator->CreateContainer(UIContainer_LinkFlag | UIContainer_DrawFlag, UIRenderType::Spline);
                container.Add(mLinkContainer);
                return true;
            }
        }
        if (container.HasConfigFlag(UIContainer_LinkFlag) && !mDragging && input.mStarted && mLinkContainer.get() == nullptr && container.GetCoParent() != nullptr) {
            ImVec2 pCenter = container.GetCoParent()->GetPosition();
            ImVec2 size = container.GetCoParent()->GetSize();
            ImVec2 pT = container.GetCoParent()->GetLayoutArea().Transform(pCenter, input.mRootPos);
            if (OverlapCircle(input.mCurPos, pT, size.x * container.GetCoParent()->GetLayoutArea().mScale)) {
                mDragging = true;
                mLinkContainer.mContainer = &container;
                return true;
            }
        }

        if (mDragging && mLinkContainer.get() != nullptr && container.HasConfigFlag(UIContainer_LinkFlag) && mLinkContainer.get() == &container) {
            // On the newly created link container, drag the end point along the mouse movement
            auto& contentArea = container.GetLayoutArea();

            ImVec2 move = DragMovement(input.mPrevPos, input.mCurPos, contentArea.mScale * container.GetScale());
            mLinkContainer->Move(move);
        }

        if (mDragging && mLinkContainer.get() != nullptr && container.HasConfigFlag(UIContainer_InputFlag)) {
            ImVec2 pCenter = container.GetPosition();
            ImVec2 size = container.GetSize();
            auto& contentArea = container.GetLayoutArea();

            ImVec2 pT = contentArea.Transform(pCenter, input.mRootPos);

            if (OverlapCircle(input.mCurPos, pT, size.x * contentArea.mScale)) {
                mLinkContainer->Notification(UIContainer_LinkFlag);
                mLinkContainer->SetCoParent(&container);
            }
            else if (mLinkContainer->GetCoParent() == &container){
                mLinkContainer->SetCoParent(nullptr);
                mLinkContainer->ClearNotifications();
            }

            if (input.mStopped) {
                mLinkContainer->ClearNotifications();
                if (mLinkContainer->GetCoParent() != nullptr) {
                    mDragging = false;
                    mLinkContainer.reset();
                }
                else {
                    mLinkContainer->GetParent()->Remove(mLinkContainer);
                    mDragging = false;
                    mLinkContainer.reset();
                }
            }
        }
        return false;
    }
}
