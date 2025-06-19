#include "rendering/ui/UIVisitors.h"
#include "hid/KeyboardPiano.h"


namespace l::ui {


    /***********************************************************************************/
    bool UIUpdate::ShouldUpdateContainer() {
        return true;
    }

    /***********************************************************************************/
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

            auto& layoutArea = container.GetLayoutArea();

            ImVec2 mousePos = input.GetLocalPos();
            ImVec2 localMousePos = ImVec2(layoutArea.mPosition.x - mousePos.x, layoutArea.mPosition.y - mousePos.y);
            ImVec2 p = container.GetPosition();
            container.Rescale(scaleChange);
            p.x = ((p.x + localMousePos.x) * scaleChange - localMousePos.x) / container.GetScale();
            p.y = ((p.y + localMousePos.y) * scaleChange - localMousePos.y) / container.GetScale();
            container.SetPosition(p);
            return true;
        }
        return false;
    }

    /***********************************************************************************/
    bool UIDrag::Active(UIContainer&, const InputState& input) {
        return (input.mStarted && !mDragging) || mDragging;
    }

    void UIDrag::Reset() {
        mDragging = false;
        if (mSourceContainer) {
            mSourceContainer->ClearNotification(UIContainer_DragFlag);
            mSourceContainer = nullptr;
        }
    }

    bool UIDrag::Visit(UIContainer& container, const InputState& input) {
        if (!container.HasConfigFlag(UIContainer_DragFlag)) {
            return false;
        }
        if (input.mStarted && !mDragging) {
            auto& layoutArea = container.GetLayoutArea();
            ImVec2 layoutPosition = layoutArea.mPosition;
            ImVec2 layoutSize = layoutArea.mSize;
            layoutPosition.x -= 6.0f;
            layoutPosition.y -= 8.0f;
            layoutSize.x -= 14.0f;
            layoutSize.y -= 14.0f;

            ImVec2 localMousePos = input.GetLocalPos();
            localMousePos.x -= layoutPosition.x;
            localMousePos.y -= layoutPosition.y;
            if (Overlap(localMousePos, ImVec2(), layoutSize)) {
                mDragging = true;
                mSourceContainer = &container;
            }
        }
        if (mDragging && mSourceContainer == &container) {
            auto& layoutArea = container.GetLayoutArea();

            ImVec2 move = DragMovement(input.mPrevPos, input.mCurPos, layoutArea.mScale * container.GetScale());
            container.Move(move);
            container.SetNotification(UIContainer_DragFlag);

            if (input.mStopped) {
                mDragging = false;
                mSourceContainer->ClearNotification(UIContainer_DragFlag);
                mSourceContainer = nullptr;
            }
            return mDragging;
        }
        return false;
    }

    /***********************************************************************************/
    bool UIMove::Active(UIContainer&, const InputState& input) {
        return (input.mStarted && !mMoving) || mMoving;
    }

    void UIMove::Reset() {
        mMoving = false;
        if (mSourceContainer) {
            mSourceContainer->ClearNotification(UIContainer_MoveFlag);
            mSourceContainer = nullptr;
        }
    }

    bool UIMove::Visit(UIContainer& container, const InputState& input) {
        if(!container.HasConfigFlag(UIContainer_MoveFlag)){
            return false;
        }
        if (input.mStarted && !mMoving) {
            auto& layoutArea = container.GetLayoutArea();
            if (Overlap(input.GetLocalPos(), container.GetPosition(), container.GetPositionAtSize(), layoutArea)) {
                mMoving = true;
                mSourceContainer = &container;
            }
        }
        if (mMoving && mSourceContainer == &container) {
            auto& layoutArea = container.GetLayoutArea();
            ImVec2 move = DragMovement(input.mPrevPos, input.mCurPos, layoutArea.mScale);
            container.Move(move);
            container.SetNotification(UIContainer_MoveFlag);

            if (input.mStopped) {
                Reset();

                if (mMoveHandler) {
                    auto p = container.GetPosition();
                    mMoveHandler(container.GetId(), container.GetNodeId(), p.x, p.y);
                }
            }
            return mMoving;
        }
        return false;
    }

    /***********************************************************************************/
    void UIResize::Reset() {
        mResizing = false;
        if (mSourceContainer) {
            mSourceContainer->ClearNotification(UIContainer_ResizeFlag);
            mSourceContainer = nullptr;
        }
    }

    bool UIResize::Visit(UIContainer& container, const InputState& input) {
        if (!container.HasConfigFlag(UIContainer_ResizeFlag)) {
            return false;
        }
        if (!mResizing) {
            ImVec2 p = container.GetPositionAtSize();
            auto& layoutArea = container.GetLayoutArea();
            float size = 3.0f * layoutArea.mScale;
            if (OverlapScreenRect(input.GetLocalPos(), p, ImVec2(size, size), layoutArea)) {
                mSourceContainer = &container;
                container.SetNotification(UIContainer_ResizeFlag);

                if (input.mStarted) {
                    mResizing = true;
                }
                else {
                    return true;
                }
            }
            else {
                mSourceContainer = nullptr;
                container.ClearNotification(UIContainer_ResizeFlag);
            }
        }
        if (mResizing && mSourceContainer == &container) {
            auto& layoutArea = container.GetLayoutArea();
            ImVec2 move = DragMovement(input.mPrevPos, input.mCurPos, layoutArea.mScale);
            container.Resize(move);

            if (input.mStopped) {
                Reset();

                if (mResizeHandler) {
                    auto s = container.GetSize();
                    mResizeHandler(container.GetId(), container.GetNodeId(), s.x, s.y);
                }
            }
            return mResizing;
        }
        return false;
    }

    /***********************************************************************************/
    bool UISelect::Visit(UIContainer& container, const InputState& input) {
        if (!container.HasConfigFlag(UIContainer_SelectFlag)) {
            return false;
        }
        if (input.mStarted) {
            auto& layoutArea = container.GetLayoutArea();
            if (ImGui::IsKeyDown(ImGuiKey::ImGuiKey_LeftShift)) {
                if (Overlap(input.GetLocalPos(), container.GetPosition(), container.GetPositionAtSize(), layoutArea)) {
                    if (!mSelectedContainers.contains(&container)) {
                        mSelectedContainers.emplace(&container);
                        container.SetNotification(UIContainer_SelectFlag);
                    }
                    else {
                        mSelectedContainers.erase(&container);
                        container.ClearNotification(UIContainer_SelectFlag);
                    }
                    return true;
                }
            }
            else if (Overlap(input.GetLocalPos(), container.GetPosition(), container.GetPositionAtSize(), layoutArea)) {
                for (auto it : mSelectedContainers) {
                    it->ClearNotification(UIContainer_SelectFlag);
                }
                mSelectedContainers.clear();
                mSelectedContainers.emplace(&container);
                container.SetNotification(UIContainer_SelectFlag);
            }
            else if (mSelectedContainers.contains(&container)){
                mSelectedContainers.erase(&container);
                container.ClearNotification(UIContainer_SelectFlag);
            }
        }
        if (ImGui::IsKeyDown(ImGuiKey::ImGuiKey_Delete)) {
            if (!mSelectedContainers.empty()) {
                for (auto it : mSelectedContainers) {
                    if (mRemoveHandler) {
                        mRemoveHandler(it->GetNodeId());
                    }
                    if (mDeleteHandler) {
                        mDeleteHandler(it->GetId(), it->GetNodeId());
                    }
                    DeleteContainer(mUIManager, it);
                }
                mSelectedContainers.clear();
                return true;
            }
        }
        return false;
    }

    /***********************************************************************************/
    bool UIEdit::Visit(UIContainer& container, const InputState& input) {
        if (!container.HasConfigFlag(UIContainer_EditFlag)) {
            return false;
        }
        if (input.mStarted && !mEditing) {
            auto& layoutArea = container.GetLayoutArea();
            if (Overlap(input.GetLocalPos(), container.GetPosition(), container.GetPositionAtSize(), layoutArea)) {
                mEditing = true;
                mSourceContainer = &container;
            }
        }
        if (mEditing && mSourceContainer == &container) {
            auto& layoutArea = container.GetLayoutArea();
            ImVec2 move = DragMovement(input.mPrevPos, input.mCurPos, layoutArea.mScale);

            if (mEditHandler) {
                mEditHandler(container.GetNodeId(), static_cast<int8_t>(container.GetChannelId()), move.x, move.y);
            }

            if (input.mStopped) {
                mEditing = false;
                mSourceContainer = nullptr;
            }
            return mEditing;
        }
        return false;
    }

    /***********************************************************************************/
    bool UIDraw::Visit(UIContainer& container, const InputState& input) {
        if (!mDebug && !container.HasConfigFlag(UIContainer_DrawFlag)) {
            return false;
        }

        auto& layoutArea = container.GetLayoutArea();

        float splineThickness = 2.0f;
        ImU32 color = container.GetRenderData().mColor;
        ImVec2 pTopLeft = container.GetPosition();
        ImVec2 pLowRight = container.GetPositionAtSize();
        ImVec2 pSize = container.GetSize();
        pSize.x *= layoutArea.mScale;
        pSize.y *= layoutArea.mScale;
        ImVec2 p1 = layoutArea.Transform(pTopLeft);
        ImVec2 p2 = layoutArea.Transform(pLowRight);
        ImVec2 p11;
        ImVec2 p22;
        float d12 = 1.0f;

        const char* nameStart;
        const char* nameEnd;

        auto renderType = container.GetRenderData().mType;
        switch (renderType) {
        case l::ui::UIRenderType::Rect:
            mDrawList->AddRect(p1, p2, color, 5.0f, ImDrawFlags_RoundCornersAll, 1.0f * container.GetScale() * layoutArea.mScale);
            break;
        case l::ui::UIRenderType::RectFilled:
            mDrawList->AddRectFilled(p1, p2, color, 5.0f, ImDrawFlags_RoundCornersAll);
            break;
        case l::ui::UIRenderType::Triangle:
            break;
        case l::ui::UIRenderType::TriangleFilled:
            break;
        case l::ui::UIRenderType::Circle:
            mDrawList->AddCircle(p1, pSize.x, color, 18, 2.0f * container.GetScale() * layoutArea.mScale);
            break;
        case l::ui::UIRenderType::CircleFilled:
            mDrawList->AddCircleFilled(p1, pSize.x, color, 15);
            break;
        case l::ui::UIRenderType::Polygon:
            break;
        case l::ui::UIRenderType::PolygonFilled:
            break;
        case l::ui::UIRenderType::LinkH:
            if (container.HasConfigFlag(UIContainer_LinkFlag)) {
                splineThickness = container.HasNotification(UIContainer_LinkFlag) ? 2.0f * splineThickness : splineThickness;
                ImVec2 pLinkInput = container.GetParent()->GetPosition();
                p1 = layoutArea.Transform(ImVec2());

                if (container.GetCoParent() == nullptr) {
                    p2 = input.mCurPos;
                    d12 = sqrt(0.25f * ((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y)) / (layoutArea.mScale * layoutArea.mScale));
                    p11 = layoutArea.Transform(ImVec2(pLinkInput.x + d12, 0.0f));
                    p22 = ImVec2(p2.x - d12 * layoutArea.mScale, p2.y);
                }
                else {
                    ImVec2 pLinkOutput = container.GetCoParent()->GetPosition();
                    auto& coLayoutArea = container.GetCoParent()->GetLayoutArea();
                    p2 = coLayoutArea.Transform(pLinkOutput);
                    d12 = sqrt(0.25f * ((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y)) / (layoutArea.mScale * layoutArea.mScale));

                    p11 = layoutArea.Transform(ImVec2(pLinkInput.x + d12, 0.0f));
                    p22 = coLayoutArea.Transform(ImVec2(pLinkOutput.x - d12, pLinkOutput.y));
                }
            }
            else {
                d12 = sqrt(0.25f * ((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y)) / (layoutArea.mScale * layoutArea.mScale));
                p11 = layoutArea.Transform(ImVec2(pTopLeft.x + d12, pTopLeft.y));
                p22 = layoutArea.Transform(ImVec2(pLowRight.x - d12, pLowRight.y));
            }
            {
                float scaleFactor = sqrt(d12 * layoutArea.mScale);
                if (scaleFactor > 50.0f) {
                    scaleFactor = 50.0f;
                }
                mDrawList->AddBezierCubic(p1, p11, p22, p2, color, splineThickness * layoutArea.mScale, static_cast<int>(10.0f + scaleFactor));
            }
            break;
        case l::ui::UIRenderType::Text:

            //const ImVec2 text_pos(window->DC.CursorPos.x, window->DC.CursorPos.y + window->DC.CurrLineTextBaseOffset);
            //const float wrap_pos_x = window->DC.TextWrapPos;

            //     if (window->DC.CurrentColumns)

            if (!container.GetDisplayName().empty()) {
                nameStart = container.GetDisplayName().data();
                nameEnd = container.GetDisplayName().data() + container.GetDisplayName().size();
                container.SetSize(ImGui::CalcTextSize(nameStart, nameEnd));
                mDrawList->AddText(ImGui::GetDefaultFont(), 13.0f * container.GetScale() * layoutArea.mScale, p1, color, nameStart, nameEnd);
            }
            break;
        case l::ui::UIRenderType::Texture:
            break;

        case l::ui::UIRenderType::NodeOutputValue:
            if (mDrawChannelTextHandler) {
                auto scale = 13.0f * container.GetScale() * layoutArea.mScale;
                mDrawChannelTextHandler(container.GetNodeId(), static_cast<int8_t>(container.GetChannelId()), p1, scale, color, mDrawList);
            }
            break;
        case l::ui::UIRenderType::NodeOutputGraph:
            if (mDrawLineHandler) {
                ImVec2 s = container.GetSize();
                s.x *= layoutArea.mScale;
                s.y *= layoutArea.mScale;
                auto scale = 2.0f * container.GetScale();
                mDrawLineHandler(container.GetNodeId(), static_cast<int8_t>(container.GetChannelId()), p1, s, scale, color, mDrawList);
            }
            break;
        }

        if (mDebug && !container.GetStringId().empty()) {
            // also render name if it is non empty as debug text
            nameStart = container.GetStringId().data();
            nameEnd = container.GetStringId().data() + container.GetStringId().size();
            mDrawList->AddText(p1, color, nameStart, nameEnd);
        }

        switch (container.GetRenderData().mType) {
        case l::ui::UIRenderType::Rect:
        case l::ui::UIRenderType::RectFilled:
        case l::ui::UIRenderType::Texture:
        case l::ui::UIRenderType::LinkH:
            if (container.HasConfigFlag(UIContainer_SelectFlag) && container.HasNotification(UIContainer_SelectFlag)) {
                auto p1cpy = ImVec2(p1.x - 1.0f, p1.y - 1.0f);
                auto p2cpy = ImVec2(p2.x + 1.0f, p2.y + 1.0f);
                mDrawList->AddRect(p1cpy, p2cpy, ImColor(ImVec4(0.7f, 1.0f, 1.0f, 1.0f)), 0.0f, 0, 1.0f);
            }
            if (container.HasConfigFlag(ui::UIContainer_ResizeFlag)) {
                float size = 3.0f * layoutArea.mScale;
                ImVec2 p3 = layoutArea.Transform(pLowRight, ImVec2(-size, -size));
                ImVec2 p4 = layoutArea.Transform(pLowRight, ImVec2(size, size));
                if (container.HasNotification(ui::UIContainer_ResizeFlag)) {
                    float size2 = 5.0f * layoutArea.mScale;
                    p3 = layoutArea.Transform(pLowRight, ImVec2(-size2, -size2));
                    p4 = layoutArea.Transform(pLowRight, ImVec2(size2, size2));
                }
                mDrawList->AddRectFilled(p3, p4, color);
            }
            break;
        default:
            break;
        }

        return false;
    }

    /***********************************************************************************/
    bool UILinkIO::Active(UIContainer& container, const InputState&) {
        return container.HasConfigFlag(UIContainer_InputFlag) || container.HasConfigFlag(UIContainer_OutputFlag) || container.HasConfigFlag(UIContainer_LinkFlag);
    }

    bool UILinkIO::Visit(UIContainer& container, const InputState& input) {
        // A link is a container object that is a child of another container with an output flag
        // The link itself has a parent (the container with an output flag)
        // and a co-parent (the container with an input flag)
        // The container with an input flag also has a co-parent and it is the link container
        // That way we have links with parents that are output containers
        // And links that have co-parents that are input containers
        // And input containers that have co-parents that are link containers
        // When deleting a node we must therefore remember to null all of those
        // * output container children -> link containers
        // * link container parent -> output container
        // * link container co-parent -> input container
        // * input container co-parent -> link container
        // But a link container is still owned by only one container, the output container

        {
            auto& outputContainer = container;

            // Create a link connection and attach it at a source node
            if (outputContainer.HasConfigFlag(UIContainer_OutputFlag) && !mDragging && input.mStarted && mLinkContainer.Get() == nullptr) {
                ImVec2 pCenter = outputContainer.GetPosition();
                ImVec2 size = outputContainer.GetSize();
                auto& layoutArea = outputContainer.GetLayoutArea();

                ImVec2 pT = layoutArea.Transform(pCenter);
                if (OverlapCircle(input.mCurPos, pT, 2.0f * size.x * layoutArea.mScale)) {
                    mDragging = true;
                    mLinkContainer = CreateContainer(mUIManager, UIContainer_LinkFlag | UIContainer_DrawFlag, UIRenderType::LinkH);
                    outputContainer.Add(mLinkContainer);
                    return true;
                }
            }
        }

        {
            auto& linkContainer = container;

            // Detach a link connection from a destination node with an existing link connection
            if (linkContainer.HasConfigFlag(UIContainer_LinkFlag) && !mDragging && input.mStarted && mLinkContainer.Get() == nullptr && linkContainer.GetCoParent() != nullptr) {
                ImVec2 pCenter = linkContainer.GetCoParent()->GetPosition();
                ImVec2 size = linkContainer.GetCoParent()->GetSize();
                ImVec2 pT = linkContainer.GetCoParent()->GetLayoutArea().Transform(pCenter);
                if (OverlapCircle(input.mCurPos, pT, 2.0f * size.x * linkContainer.GetCoParent()->GetLayoutArea().mScale)) {
                    mLinkContainer.mContainer = &linkContainer;
                    mLinkHandler(mLinkContainer->GetCoParent()->GetNodeId(), mLinkContainer->GetParent()->GetNodeId(), mLinkContainer->GetCoParent()->GetChannelId(), mLinkContainer->GetParent()->GetChannelId(), false);
                    mDragging = true;
                    return true;
                }
            }

            // Drag the link end
            if (mDragging && mLinkContainer.Get() != nullptr && linkContainer.HasConfigFlag(UIContainer_LinkFlag) && mLinkContainer.Get() == &linkContainer) {
                // On the newly created link container, drag the end point along the mouse movement
                auto& layoutArea = mLinkContainer->GetLayoutArea();

                ImVec2 move = DragMovement(input.mPrevPos, input.mCurPos, layoutArea.mScale * mLinkContainer->GetScale());
                mLinkContainer->Move(move);
            }
        }

        {
            auto& inputContainer = container;

            // Check containers with input flags, i.e. a node input channel area
            if (mDragging && mLinkContainer.Get() != nullptr && inputContainer.HasConfigFlag(UIContainer_InputFlag)) {
                ImVec2 pCenter = inputContainer.GetPosition();
                ImVec2 size = inputContainer.GetSize();
                auto& layoutArea = inputContainer.GetLayoutArea();

                ImVec2 pT = layoutArea.Transform(pCenter);

                // if there is overlap we connect it
                if (OverlapCircle(input.mCurPos, pT, 2.0f * size.x * layoutArea.mScale)) {
                    if (mLinkHandler(inputContainer.GetNodeId(), mLinkContainer->GetParent()->GetNodeId(), inputContainer.GetChannelId(), mLinkContainer->GetParent()->GetChannelId(), true)) {
                        mLinkContainer->SetNotification(UIContainer_LinkFlag);
                        mLinkContainer->SetCoParent(&inputContainer);
                        inputContainer.SetCoParent(mLinkContainer.Get());
                    }
                    else {
                        // This link is already connected (or there is another link connected already)
                    }
                }
                // If this link if connected to this input node channel area, we detach it because the overlap failed (we moved it away)
                else if (mLinkContainer->GetCoParent() == &inputContainer) {
                    mLinkHandler(inputContainer.GetNodeId(), mLinkContainer->GetParent()->GetNodeId(), inputContainer.GetChannelId(), mLinkContainer->GetParent()->GetChannelId(), false);
                    mLinkContainer->SetCoParent(nullptr);
                    mLinkContainer->ClearNotification(UIContainer_LinkFlag);
                }

                if (input.mStopped) {
                    mLinkContainer->ClearNotification(UIContainer_LinkFlag);

                    // dragging stopped and the link is attached
                    if (mLinkContainer->GetCoParent() != nullptr) {
                        mDragging = false;
                        mLinkContainer.Reset();
                    }
                    // dragging stopped and the link is detached so delete it
                    else {
                        DeleteContainer(mUIManager, mLinkContainer.Get());
                        mDragging = false;
                        mLinkContainer.Reset();
                        return true;
                    }
                }
            }
        }
        return false;
    }

}
