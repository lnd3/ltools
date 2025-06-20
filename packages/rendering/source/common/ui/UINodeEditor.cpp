#include "rendering/ui/UINodeEditor.h"

#include <memory>

namespace l::ui {

    void depthFirstTraversal(const nodegraph::TreeMenuNode& node, std::vector<std::string>& path, std::function<void(std::string_view, int32_t)> cbMenuItem) {
        if (node.GetPathPart().empty()) {
            for (const auto& child : node.mChildren) {
                depthFirstTraversal(child, path, cbMenuItem);
            }
            cbMenuItem(node.GetName(), node.GetId());
        }
        else {
            path.emplace_back(node.GetPathPart());
            if (ImGui::TreeNode(node.GetPathPart().data())) {
                for (const auto& child : node.mChildren) {
                    depthFirstTraversal(child, path, cbMenuItem);
                }
                ImGui::TreePop();
            }
            path.pop_back();
        }
    }

    void UINodeEditor::Init() {
        mUIRoot = CreateContainer(mUIManager, l::ui::UIContainer_DragFlag | l::ui::UIContainer_ZoomFlag);

        mUIWindow.SetContentWindow([&]() {
            ImGui::PushItemWidth(400);

            UIUpdate updateVisitor;
            mDrawVisitor.SetDrawList(ImGui::GetWindowDrawList());
            mUIRoot->SetLayoutSize(mUIWindow.GetSize());
            mUIRoot->SetLayoutPosition(mUIWindow.GetPosition());
            mUIRoot->Accept(updateVisitor, mUIInput, l::ui::UITraversalMode::BFS);
            mUIRoot->Accept(mDrawVisitor, mUIInput, l::ui::UITraversalMode::BFS);

            ImGui::PopItemWidth();

            });

        mUIWindow.SetPointerPopup([&]() {
            ImGui::Text("Node picker");
            ImGui::Separator();

            if (mNGSchema == nullptr) {
                return;
            }

            std::vector<std::string> path;
            depthFirstTraversal(mNGSchema->GetPickerRoot(), path, [&](std::string_view menuName, int32_t menuId) {
                if (!menuName.empty() && ImGui::MenuItem(menuName.data())) {
                    ImVec2 p = ImVec2(mUIInput.mCurPos.x - mUIWindow.GetPosition().x, mUIInput.mCurPos.y - mUIWindow.GetPosition().y);
                    p.x -= mUIRoot->GetPosition().x;
                    p.y -= mUIRoot->GetPosition().y;
                    p.x /= mUIRoot->GetScale();
                    p.y /= mUIRoot->GetScale();
                    p.x -= 3.0f;
                    p.y -= 3.0f;
                    auto nodeId = mNGSchema->NewNode(menuId);
                    auto node = mNGSchema->GetNode(nodeId);
                    if (node != nullptr) {
                        auto uiNode = l::ui::CreateUINode(mUIManager, *node, p);
                        mUIRoot->Add(uiNode);

                        auto& uiData = node->GetUIData();
                        auto position = uiNode->GetPosition();
                        auto size = uiNode->GetSize();
                        uiData.x = position.x;
                        uiData.y = position.y;
                        uiData.w = size.x;
                        uiData.h = size.y;
                    }
                }

                });

            });

        mDrawVisitor.SetDrawChannelTextHandler([&](int32_t nodeId, int8_t channelId, ImVec2 p, float scale, ImU32 color, ImDrawList* drawList) {
            if (mNGSchema == nullptr) {
                return;
            }

            auto node = mNGSchema->GetNode(nodeId);
            if (node) {
                l::string::string_buffer<32> sb;
                if (channelId < node->GetNumInputs()) {
                    if (node->IsInputDataVisible(channelId)) {
                        if (node->IsInputDataText(channelId)) {
                            auto nodeText = node->GetInputText(channelId, 31);
                            sb.append(nodeText);
                        }
                        else if (node->IsInputDataArray(channelId)) {
                            sb.append("{...}");
                        }
                        else {
                            float nodeValue = node->GetInput(channelId);
                            auto nodeValueAbs = l::math::abs(nodeValue);
                            if (nodeValueAbs > 100.0f) {
                                sb.printf("%.0f", nodeValue);
                            }
                            else if (nodeValueAbs > 10.0f) {
                                sb.printf("%.1f", nodeValue);
                            }
                            else if (nodeValueAbs > 1.0f) {
                                sb.printf("%.2f", nodeValue);
                            }
                            else if (nodeValueAbs > 0.1f) {
                                sb.printf("%.3f", nodeValue);
                            }
                            else {
                                sb.printf("%.4f", nodeValue);
                            }
                        }
                    }
                }
                else {
                    float nodeValue = node->GetOutput(channelId);
                    auto nodeValueAbs = l::math::abs(nodeValue);
                    if (nodeValueAbs > 100.0f) {
                        sb.printf("%.0f", nodeValue);
                    }
                    else if (nodeValueAbs > 10.0f) {
                        sb.printf("%.1f", nodeValue);
                    }
                    else if (nodeValueAbs > 1.0f) {
                        sb.printf("%.2f", nodeValue);
                    }
                    else if (nodeValueAbs > 0.1f) {
                        sb.printf("%.3f", nodeValue);
                    }
                    else {
                        sb.printf("%.4f", nodeValue);
                    }
                }

                drawList->AddText(ImGui::GetDefaultFont(), scale, p, color, sb.str().data());
            }
            });

        mDrawVisitor.SetDrawLineHandler([&](int32_t nodeId, int8_t channelId, ImVec2 p1, ImVec2 size, float scale, ImU32 color, ImDrawList* drawList) {
            if (mNGSchema == nullptr) {
                return;
            }

            auto node = mNGSchema->GetNode(nodeId);
            if (node && channelId < node->GetNumOutputs()) {
                float* nodeValues = &node->GetOutput(channelId);
                int32_t nodeValueCount = node->GetOutputSize(channelId);
                ImVec2 startPos = ImVec2(p1.x, p1.y + 0.5f * size.y);
                for (int32_t i = 0; i < nodeValueCount - 1; i++) {
                    float xpart1 = i / static_cast<float>(nodeValueCount);
                    float xpart2 = (i + 1) / static_cast<float>(nodeValueCount);
                    ImVec2 graphP1 = ImVec2(startPos.x + size.x * xpart1, startPos.y + 0.5f * nodeValues[i] * size.y);
                    ImVec2 graphP2 = ImVec2(startPos.x + size.x * xpart2, startPos.y + 0.5f * nodeValues[i + 1] * size.y);
                    drawList->AddLine(graphP1, graphP2, color, scale);
                }
            }
            });

        mLinkIOVisitor.SetLinkHandler([&](int32_t linkInputId, int32_t linkOutputId, int32_t inputChannel, int32_t outputChannel, bool connected) {
            if (mNGSchema == nullptr) {
                return false;
            }

            auto inputNode = mNGSchema->GetNode(linkInputId);
            if (inputNode == nullptr) {
                return false;
            }
            if (connected) {
                auto outputNode = mNGSchema->GetNode(linkOutputId);
                return outputNode != nullptr && inputNode->SetInput(static_cast<int8_t>(inputChannel), *outputNode, static_cast<int8_t>(outputChannel));
            }
            return inputNode->ClearInput(static_cast<int8_t>(inputChannel));
            });

        mEditVisitor.SetEditHandler([&](int32_t nodeId, int8_t channelId, float, float dy) {
            if (mNGSchema == nullptr) {
                return;
            }

            auto node = mNGSchema->GetNode(nodeId);
            if (node->IsInputDataEditable(channelId)) {
                float* nodeValue = nullptr;
                if (channelId < node->GetNumInputs()) {
                    nodeValue = &node->GetInput(channelId, 1);
                }
                else if (channelId < node->GetNumOutputs()) {
                    nodeValue = &node->GetOutput(channelId, 1);
                }
                if (nodeValue != nullptr) {
                    if (!ImGui::IsKeyDown(ImGuiKey::ImGuiKey_LeftShift)) {
                        if (!ImGui::IsKeyDown(ImGuiKey::ImGuiKey_LeftCtrl)) {
                            *nodeValue -= dy / 100.0f;
                        }
                        else {
                            *nodeValue -= dy / 10000.0f;
                        }
                    }
                    else {
                        if (!ImGui::IsKeyDown(ImGuiKey::ImGuiKey_LeftCtrl)) {
                            *nodeValue -= dy;
                        }
                        else {
                            *nodeValue -= 1000.0f * dy;
                        }
                    }
                    if (channelId < node->GetNumInputs()) {
                        node->SetInput(channelId, *nodeValue);
                    }
                    else if (channelId < node->GetNumOutputs()) {
                        node->GetOutput(channelId, 1) = *nodeValue;
                    }
                }
            }
            });

        mSelectVisitor.SetDeleteHandler([&](int32_t containerId, int32_t nodeId) {
            if (mNGSchema == nullptr) {
                return;
            }

            NodeEvent event;
            event.mNodeSchema = mNGSchema;
            event.mNodeEvent = 2; // 2 delete
            event.mContainerId = containerId;
            event.mNodeId = nodeId;

            for (auto& it : mEventListeners) {
                it(event);
            }
            });

        mMoveVisitor.SetMoveHandler([&](int32_t containerId, int32_t nodeId, float x, float y) {
            if (mNGSchema == nullptr) {
                return;
            }
            LOG(LogInfo) << "Container " << containerId << " moved to " << x << ", " << y;
            auto node = mNGSchema->GetNode(nodeId);
            if (node != nullptr) {
                auto& uiData = node->GetUIData();
                uiData.x = x;
                uiData.y = y;
            }
            });

        mResizeVisitor.SetResizeHandler([&](int32_t containerId, int32_t nodeId, float w, float h) {
            if (mNGSchema == nullptr) {
                return;
            }
            LOG(LogInfo) << "Container " << containerId << " resized to " << w << ", " << h;
            auto node = mNGSchema->GetNode(nodeId);
            if (node != nullptr) {
                auto& uiData = node->GetUIData();
                uiData.w = w;
                uiData.h = h;
            }
            });

        mSelectVisitor.SetRemoveHandler([&](int32_t nodeId) {
            if (mNGSchema == nullptr) {
                return;
            }

            mNGSchema->RemoveNode(nodeId);
            });
        //mSelectVisitor.SetDeleteHandler([&](int32_t nodeId) {
        //    if (mNGSchema == nullptr) {
        //       return;
        //    }
        //    });


    }

    bool UINodeEditor::IsShowing() {
        return mUIWindow.IsShowing();
    }

    void UINodeEditor::Show() {
        mUIWindow.Show();
    }

    void UINodeEditor::Open() {
        mUIWindow.Open();
    }

    void UINodeEditor::Close() {
        mUIWindow.Close();
    }

    void UINodeEditor::SetWindowName(std::string_view windowName) {
        mUIWindow.SetWindowName(windowName);

    }

    void UINodeEditor::SetNGSchema(l::nodegraph::NodeGraphSchema* ngSchema) {
        mUIRoot->RemoveAll();

        mNGSchema = ngSchema;

        mNGSchema->ForEachNode([&](nodegraph::NodeGraphBase* node) {
            if (node != nullptr) {
                auto& uiData = node->GetUIData();
                auto p = ImVec2(uiData.x, uiData.y);
                auto s = ImVec2(uiData.w, uiData.h);
                auto uiNode = l::ui::CreateUINode(mUIManager, *node, p);
                //if (s.x > 10.0f && s.y > 10.0f) {
                //    uiNode->SetSize(s);
                //}

                //LOG(LogInfo) << "Replicated node type " << node->GetTypeId() << " as a ui node";
                mUIRoot->Add(uiNode);
            }

            return true;
            });

        mNGSchema->ForEachNode([&](nodegraph::NodeGraphBase* node) {
            if (node != nullptr) {
                int inputChannel = 0;

                node->ForEachInput([&](l::nodegraph::NodeGraphInput& input) {
                    if (input.HasInputNode()) {

                        auto outputNode = input.GetInputNode();
                        auto outputChannel = input.GetInputSrcChannel();
                        auto mLinkContainer = CreateContainer(mUIManager, UIContainer_LinkFlag | UIContainer_DrawFlag, UIRenderType::LinkH);
                        mLinkContainer->SetColor(l::ui::pastellYellow);

                        auto outputContainer = mUIManager.FindNodeId(UIContainer_OutputFlag, outputNode->GetId(), outputChannel);
                        auto inputContainer = mUIManager.FindNodeId(UIContainer_InputFlag, node->GetId(), inputChannel);

                        ASSERT(outputContainer);
                        ASSERT(inputContainer);

                        outputContainer->Add(mLinkContainer);

                        // connect link to input
                        mLinkContainer->SetCoParent(inputContainer);
                        // and input to link
                        inputContainer->SetCoParent(mLinkContainer.Get());
                    }
                    inputChannel++;
                    });
            }

            return true;
            });
    }

    l::nodegraph::NodeGraphSchema* UINodeEditor::GetNGSchema() {
        return mNGSchema;
    }

    void UINodeEditor::SetEventListener(std::function<void(const NodeEvent& event)> cb) {
        mEventListeners.push_back(cb);
    }

    void UINodeEditor::Update(double, float) {

        if (mUIWindow.IsShowing()) {
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigWindowsMoveFromTitleBarOnly = true;

            mUIInput.mPrevPos.x = mUIInput.mCurPos.x;
            mUIInput.mPrevPos.y = mUIInput.mCurPos.y;
            mUIInput.mCurPos = io.MousePos;
            mUIInput.mScroll = io.MouseWheel;
            mUIInput.mStarted = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
            mUIInput.mStopped = ImGui::IsMouseReleased(ImGuiMouseButton_Left);

            if (mUIWindow.IsHovered()) {
                if (mUIRoot->Accept(mLinkIOVisitor, mUIInput, l::ui::UITraversalMode::DFS)) {
                }
                else if (mUIRoot->Accept(mEditVisitor, mUIInput, l::ui::UITraversalMode::DFS)) {
                }
                else if (mUIRoot->Accept(mSelectVisitor, mUIInput, l::ui::UITraversalMode::BFS)) {
                }
                else if (mUIRoot->Accept(mResizeVisitor, mUIInput, l::ui::UITraversalMode::DFS)) {
                }
                else if (mUIRoot->Accept(mMoveVisitor, mUIInput, l::ui::UITraversalMode::DFS)) {
                }
                else if (mUIRoot->Accept(mZoomVisitor, mUIInput, l::ui::UITraversalMode::DFS)) {
                }
                else if (mUIRoot->Accept(mDragVisitor, mUIInput, l::ui::UITraversalMode::DFS)) {
                }
            }
            else {
                mResizeVisitor.Reset();
                mMoveVisitor.Reset();
                mDragVisitor.Reset();
            }
        }
    }
}
