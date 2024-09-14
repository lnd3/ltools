#include "rendering/ui/UIVisitors.h"
#include "hid/KeyboardPiano.h"

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

    bool UIDrag::Active(UIContainer&, const InputState& input) {
        return (input.mStarted && !mDragging) || mDragging;
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
                mResizing = false;
                mSourceContainer = nullptr;
                container.ClearNotification(UIContainer_ResizeFlag);
            }
            return mResizing;
        }
        return false;
    }

    bool UISelect::Visit(UIContainer& container, const InputState& input) {
        if (!container.HasConfigFlag(UIContainer_SelectFlag)) {
            return false;
        }
        if (input.mStarted) {
            if (ImGui::IsKeyDown(ImGuiKey::ImGuiKey_LeftShift)) {
                auto& layoutArea = container.GetLayoutArea();
                if (Overlap(input.GetLocalPos(), container.GetPosition(), container.GetPositionAtSize(), layoutArea)) {
                    if (!mSelectedContainers.contains(&container)) {
                        mSelectedContainers.emplace(&container);
                        container.SetNotification(UIContainer_SelectFlag);
                    }
                    else {
                        mSelectedContainers.erase(&container);
                        container.ClearNotification(UIContainer_SelectFlag);
                    }
                }
            }
            else if (!mSelectedContainers.empty()) {
                for (auto it : mSelectedContainers) {
                    it->ClearNotification(UIContainer_SelectFlag);
                }
                mSelectedContainers.clear();
                return true;
            }
        }
        if (!mSelectedContainers.empty()) {
            if (ImGui::IsKeyDown(ImGuiKey::ImGuiKey_Delete)) {
                for (auto it : mSelectedContainers) {
                    if (mNGSchema != nullptr) {
                        mNGSchema->RemoveNode(it->GetNodeId());
                    }
                    DeleteContainer(mUIStorage, it);
                }
                mSelectedContainers.clear();
                return true;
            }
        }
        return false;
    }

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

            if (mNGSchema) {
                auto node = mNGSchema->GetNode(container.GetNodeId());
                auto nodeChannel = static_cast<int8_t>(container.GetChannelId());
                if (node->IsDataEditable(nodeChannel)) {
                    float* nodeValue = nullptr;
                    if (nodeChannel < node->GetNumInputs()) {
                        nodeValue = &node->GetInput(nodeChannel, 1);
                    }
                    else if (nodeChannel < node->GetNumOutputs()) {
                        nodeValue = &node->GetOutput(nodeChannel, 1);
                    }
                    if (nodeValue != nullptr) {
                        if (!ImGui::IsKeyDown(ImGuiKey::ImGuiKey_LeftShift)) {
                            if (!ImGui::IsKeyDown(ImGuiKey::ImGuiKey_LeftCtrl)) {
                                *nodeValue -= move.y / 100.0f;
                            }
                            else {
                                *nodeValue -= move.y / 10000.0f;
                            }
                        }
                        else {
                            if (!ImGui::IsKeyDown(ImGuiKey::ImGuiKey_LeftCtrl)) {
                                *nodeValue -= move.y;
                            }
                            else {
                                *nodeValue -= 1000.0f * move.y;
                            }
                        }
                        if (nodeChannel < node->GetNumInputs()) {
                            node->SetInput(nodeChannel, *nodeValue);
                        }
                        else if (nodeChannel < node->GetNumOutputs()) {
                            node->GetOutput(nodeChannel, 1) = *nodeValue;
                        }
                    }
                }
            }

            if (input.mStopped) {
                mEditing = false;
                mSourceContainer = nullptr;
            }
            return mEditing;
        }
        return false;
    }

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
            if (mNGSchema) {
                auto node = mNGSchema->GetNode(container.GetNodeId());
                float nodeValue = 0.0f;
                if (container.GetChannelId() < node->GetNumInputs()) {
                    nodeValue = node->GetInput(static_cast<int8_t>(container.GetChannelId()));
                }
                else {
                    nodeValue = node->GetOutput(static_cast<int8_t>(container.GetChannelId()));
                }
                auto nodeString = std::to_string(nodeValue);
                mDrawList->AddText(ImGui::GetDefaultFont(), 13.0f * container.GetScale() * layoutArea.mScale, p1, color, nodeString.c_str());
            }
            break;
        case l::ui::UIRenderType::NodeOutputGraph:
            if (mNGSchema) {
                auto node = mNGSchema->GetNode(container.GetNodeId());
                if (container.GetChannelId() < node->GetNumOutputs()) {
                    int8_t channel = static_cast<int8_t>(container.GetChannelId());
                    float* nodeValues = &node->GetOutput(channel);
                    int32_t nodeValueCount = node->GetOutputSize(channel);
                    ImVec2 size = container.GetSize();
                    size.x *= layoutArea.mScale;
                    size.y *= layoutArea.mScale;
                    ImVec2 startPos = ImVec2(p1.x, p1.y + 0.5f * size.y);
                    for (int32_t i = 0; i < nodeValueCount - 1; i++) {
                        float xpart1 = i / static_cast<float>(nodeValueCount);
                        float xpart2 = (i+1) / static_cast<float>(nodeValueCount);
                        ImVec2 graphP1 = ImVec2(startPos.x + size.x * xpart1, startPos.y + 0.5f * nodeValues[i] * size.y);
                        ImVec2 graphP2 = ImVec2(startPos.x + size.x * xpart2, startPos.y + 0.5f * nodeValues[i+1] * size.y);
                        mDrawList->AddLine(graphP1, graphP2, color, 2.0f * container.GetScale());
                    }
                }
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
                mDrawList->AddRect(p1, p2, ImColor(ImVec4(0.9f, 1.0f, 1.0f, 1.0f)), 0.0f, 0, 2.0f);
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

    bool UILinkIO::Active(UIContainer& container, const InputState&) {
        return container.HasConfigFlag(UIContainer_InputFlag) || container.HasConfigFlag(UIContainer_OutputFlag) || container.HasConfigFlag(UIContainer_LinkFlag);
    }

    bool UILinkIO::Visit(UIContainer& container, const InputState& input) {
        // Create link at from a clicked output container
        if (container.HasConfigFlag(UIContainer_OutputFlag) && !mDragging && input.mStarted && mLinkContainer.Get() == nullptr) {
            ImVec2 pCenter = container.GetPosition();
            ImVec2 size = container.GetSize();
            auto& layoutArea = container.GetLayoutArea();

            ImVec2 pT = layoutArea.Transform(pCenter);
            if (OverlapCircle(input.mCurPos, pT, 2.0f * size.x * layoutArea.mScale)) {
                mDragging = true;
                mLinkContainer = CreateContainer(mUIStorage, UIContainer_LinkFlag | UIContainer_DrawFlag, UIRenderType::LinkH);
                container.Add(mLinkContainer);
                return true;
            }
        }
        if (container.HasConfigFlag(UIContainer_LinkFlag) && !mDragging && input.mStarted && mLinkContainer.Get() == nullptr && container.GetCoParent() != nullptr) {
            ImVec2 pCenter = container.GetCoParent()->GetPosition();
            ImVec2 size = container.GetCoParent()->GetSize();
            ImVec2 pT = container.GetCoParent()->GetLayoutArea().Transform(pCenter);
            if (OverlapCircle(input.mCurPos, pT, 2.0f * size.x * container.GetCoParent()->GetLayoutArea().mScale)) {
                mLinkContainer.mContainer = &container;
                LinkHandler(mLinkContainer->GetCoParent()->GetNodeId(), mLinkContainer->GetParent()->GetNodeId(), mLinkContainer->GetCoParent()->GetChannelId(), mLinkContainer->GetParent()->GetChannelId(), false);
                mDragging = true;
                return true;
            }
        }

        if (mDragging && mLinkContainer.Get() != nullptr && container.HasConfigFlag(UIContainer_LinkFlag) && mLinkContainer.Get() == &container) {
            // On the newly created link container, drag the end point along the mouse movement
            auto& layoutArea = container.GetLayoutArea();

            ImVec2 move = DragMovement(input.mPrevPos, input.mCurPos, layoutArea.mScale * container.GetScale());
            mLinkContainer->Move(move);
        }

        if (mDragging && mLinkContainer.Get() != nullptr && container.HasConfigFlag(UIContainer_InputFlag)) {
            ImVec2 pCenter = container.GetPosition();
            ImVec2 size = container.GetSize();
            auto& layoutArea = container.GetLayoutArea();

            ImVec2 pT = layoutArea.Transform(pCenter);

            if (OverlapCircle(input.mCurPos, pT, 2.0f * size.x * layoutArea.mScale)) {
                if (LinkHandler(container.GetNodeId(), mLinkContainer->GetParent()->GetNodeId(), container.GetChannelId(), mLinkContainer->GetParent()->GetChannelId(), true)) {
                    mLinkContainer->SetNotification(UIContainer_LinkFlag);
                    mLinkContainer->SetCoParent(&container);
                }
                else {
                    // Failed to connect link
                }
            }
            else if (mLinkContainer->GetCoParent() == &container) {
                LinkHandler(container.GetNodeId(), mLinkContainer->GetParent()->GetNodeId(), container.GetChannelId(), mLinkContainer->GetParent()->GetChannelId(), false);
                mLinkContainer->SetCoParent(nullptr);
                mLinkContainer->ClearNotification(UIContainer_LinkFlag);
            }

            if (input.mStopped) {
                mLinkContainer->ClearNotification(UIContainer_LinkFlag);
                if (mLinkContainer->GetCoParent() != nullptr) {
                    mDragging = false;
                    mLinkContainer.Reset();
                }
                else {
                    DeleteContainer(mUIStorage, mLinkContainer.Get());
                    mDragging = false;
                    mLinkContainer.Reset();
                    return true;
                }
            }
        }
        return false;
    }

}
