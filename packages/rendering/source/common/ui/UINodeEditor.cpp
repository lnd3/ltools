#include "rendering/ui/UINodeEditor.h"

#include "rendering/ImguiSpectrum.h"

#include <algorithm>
#include <cfloat>
#include <memory>

namespace l::ui {

    void depthFirstTraversal(const nodegraph::TreeMenuNode& node, std::vector<std::string>& path, std::function<void(std::string_view, int32_t, std::string_view)> cbMenuItem) {
        if (node.GetPathPart().empty()) {
            for (const auto& child : node.mChildren) {
                depthFirstTraversal(child, path, cbMenuItem);
            }
            cbMenuItem(node.GetName(), node.GetId(), node.GetDescription());
        }
        else {
            path.emplace_back(node.GetPathPart());
            if (ImGui::BeginMenu(node.GetPathPart().data())) {
                for (const auto& child : node.mChildren) {
                    depthFirstTraversal(child, path, cbMenuItem);
                }
                ImGui::EndMenu();
            }
            path.pop_back();
        }
    }

    void UINodeEditor::Init() {
        SetContentWindow([&](UIWindow&) {
            ImGui::PushItemWidth(400);

            if (mNGSchema == nullptr) {
                return;
            }

            UIUpdate updateVisitor;
            mDrawVisitor.SetDrawList(ImGui::GetWindowDrawList());
            mUIRoot->SetLayoutSize(GetSize());
            mUIRoot->SetLayoutPosition(GetPosition());
            mUIRoot->Accept(updateVisitor, mUIInput, l::ui::UITraversalMode::BFS);

            // Render logical group rects before links and nodes (furthest back)
            auto& logicalGroups = mNGSchema->GetLogicalGroups();
            if (!logicalGroups.empty()) {
                ImDrawList* dl = ImGui::GetWindowDrawList();
                float rootScale = mUIRoot->GetScale();
                ImVec2 rootPos = mUIRoot->GetPosition();
                ImVec2 winPos = GetPosition();

                for (auto& group : logicalGroups) {
                    if (group.mNodeIds.empty()) continue;
                    float minX = FLT_MAX, minY = FLT_MAX, maxX = -FLT_MAX, maxY = -FLT_MAX;
                    bool anyValid = false;
                    for (int32_t nid : group.mNodeIds) {
                        auto* node = mNGSchema->GetNode(nid);
                        if (!node) continue;
                        auto& ui = node->GetUIData();
                        constexpr float pad = 12.0f;
                        minX = std::min(minX, ui.x - pad);
                        minY = std::min(minY, ui.y - pad);
                        maxX = std::max(maxX, ui.x + ui.w + pad);
                        maxY = std::max(maxY, ui.y + ui.h + pad);
                        anyValid = true;
                    }
                    if (!anyValid) continue;
                    auto& c = group.mColor;
                    auto toScreen = [&](float cx, float cy) -> ImVec2 {
                        return { winPos.x + rootPos.x + cx * rootScale,
                                 winPos.y + rootPos.y + cy * rootScale };
                    };
                    ImU32 fillCol = IM_COL32(int(c[0]*255), int(c[1]*255), int(c[2]*255), 20);
                    ImU32 lineCol = IM_COL32(int(c[0]*255), int(c[1]*255), int(c[2]*255), 180);
                    ImVec2 pMin = toScreen(minX, minY);
                    ImVec2 pMax = toScreen(maxX, maxY);
                    dl->AddRectFilled(pMin, pMax, fillCol, 6.0f);
                    dl->AddRect(pMin, pMax, lineCol, 6.0f, 0, 2.0f);
                    float labelX = (group.mLabelX != 0.0f || group.mLabelY != 0.0f) ? group.mLabelX : minX + 4.0f;
                    float labelY = (group.mLabelX != 0.0f || group.mLabelY != 0.0f) ? group.mLabelY : minY - 14.0f;
                    dl->AddText(toScreen(labelX, labelY), lineCol, group.mName.c_str());
                }
            }

            // Two-pass rendering: draw links first (behind), then nodes (in front)
            mDrawVisitor.SetDrawMode(UIDrawMode::LinksOnly);
            mUIRoot->Accept(mDrawVisitor, mUIInput, l::ui::UITraversalMode::BFS);
            mDrawVisitor.SetDrawMode(UIDrawMode::NoLinks);
            mUIRoot->Accept(mDrawVisitor, mUIInput, l::ui::UITraversalMode::BFS);

            ImGui::PopItemWidth();

            if (mOverlayContentWindow) {
                mOverlayContentWindow(*this);
            }

            });

        SetPointerPopup([&]() {
            if (mNGSchema == nullptr) {
                return;
            }

            // On popup open: capture selection and detect which group (if any) was right-clicked.
            if (ImGui::IsWindowAppearing()) {
                mSelectVisitor.GetSelectedNodeIds(mPopupSelectedIds);

                mPopupHoveredGroupId = -1;
                float scale  = mUIRoot->GetScale();
                ImVec2 rPos  = mUIRoot->GetPosition();
                ImVec2 wPos  = GetPosition();
                float cx = (mUIInput.mCurPos.x - wPos.x - rPos.x) / scale;
                float cy = (mUIInput.mCurPos.y - wPos.y - rPos.y) / scale;
                for (auto& group : mNGSchema->GetLogicalGroups()) {
                    if (group.mNodeIds.empty()) continue;
                    float minX = FLT_MAX, minY = FLT_MAX, maxX = -FLT_MAX, maxY = -FLT_MAX;
                    for (int32_t nid : group.mNodeIds) {
                        auto* node = mNGSchema->GetNode(nid);
                        if (!node) continue;
                        auto& ui = node->GetUIData();
                        constexpr float pad = 12.0f;
                        minX = std::min(minX, ui.x - pad);
                        minY = std::min(minY, ui.y - pad);
                        maxX = std::max(maxX, ui.x + ui.w + pad);
                        maxY = std::max(maxY, ui.y + ui.h + pad);
                    }
                    if (cx >= minX && cx <= maxX && cy >= minY && cy <= maxY) {
                        mPopupHoveredGroupId = group.mId;
                        break;
                    }
                }
            }

            // ── Group context menu (right-click on a group rect) ──────────────────
            if (mPopupHoveredGroupId >= 0) {
                auto* g = mNGSchema->GetLogicalGroup(mPopupHoveredGroupId);
                if (!g) { mPopupHoveredGroupId = -1; return; }

                ImGui::Text("%s", g->mName.c_str());
                ImGui::Separator();

                if (ImGui::BeginMenu("Rename")) {
                    static char renameBuf[64] = "";
                    ImGui::InputText("##grpctxname", renameBuf, sizeof(renameBuf));
                    if (ImGui::Button("Apply") && renameBuf[0] != '\0') {
                        g->mName = renameBuf;
                        renameBuf[0] = '\0';
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndMenu();
                }
                if (!mPopupSelectedIds.empty()) {
                    if (ImGui::MenuItem("Set Selection as Members")) {
                        g->mNodeIds = mPopupSelectedIds;
                        ImGui::CloseCurrentPopup();
                    }
                }
                if (mSaveGroupAsModuleCallback) {
                    if (ImGui::BeginMenu("Save as Module...")) {
                        static char modNameBuf[64] = "";
                        if (ImGui::IsWindowAppearing() && modNameBuf[0] == '\0') {
                            std::strncpy(modNameBuf, g->mName.c_str(), sizeof(modNameBuf) - 1);
                            modNameBuf[sizeof(modNameBuf) - 1] = '\0';
                        }
                        ImGui::InputText("Name##grpmodname", modNameBuf, sizeof(modNameBuf));
                        if (ImGui::Button("Save##grpmodsave") && modNameBuf[0] != '\0') {
                            mSaveGroupAsModuleCallback(mPopupHoveredGroupId, modNameBuf);
                            modNameBuf[0] = '\0';
                            ImGui::CloseCurrentPopup();
                        }
                        ImGui::EndMenu();
                    }
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Delete Group")) {
                    mNGSchema->RemoveLogicalGroup(mPopupHoveredGroupId);
                    ImGui::CloseCurrentPopup();
                }
                return;
            }

            // ── Node picker (right-click on empty canvas) ─────────────────────────
            ImGui::Text("Node picker");
            ImGui::Separator();

            // Create group from current selection
            if (!mPopupSelectedIds.empty()) {
                if (ImGui::BeginMenu("Group Selection...")) {
                    static char groupNameBuf[64] = "";
                    ImGui::InputText("Name##grpnew", groupNameBuf, sizeof(groupNameBuf));
                    if (ImGui::Button("Create") && groupNameBuf[0] != '\0') {
                        auto& grp = mNGSchema->AddLogicalGroup(groupNameBuf);
                        grp.mNodeIds = mPopupSelectedIds;
                        groupNameBuf[0] = '\0';
                        mPopupSelectedIds.clear();
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndMenu();
                }
                ImGui::Separator();
            }

            std::vector<std::string> path;
            depthFirstTraversal(mNGSchema->GetPickerRoot(), path, [&](std::string_view menuName, int32_t menuId, std::string_view description) {
                //if (mPickerSearch.data() != 0 && !l::string::equal_partial(mPickerSearch.data(), menuName.data(), 0, 0, 20)) {
                //    return;
                //}
                if (!menuName.empty()) {
                    if (ImGui::MenuItem(menuName.data())) {
                        ImVec2 p = ImVec2(mUIInput.mCurPos.x - GetPosition().x, mUIInput.mCurPos.y - GetPosition().y);
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
                    if (!description.empty() && ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip)) {
                        ImGui::BeginTooltip();
                        ImGui::PushTextWrapPos(350);
                        ImGui::PushStyleColor(0, ImGui::ColorConvertU32ToFloat4(ImGui::Spectrum::GRAY900));
                        ImGui::TextWrapped("%s", menuName.data());
                        ImGui::PopStyleColor();
                        ImGui::Separator();
                        ImGui::PushStyleColor(0, ImGui::ColorConvertU32ToFloat4(ImGui::Spectrum::GRAY700));
                        ImGui::TextWrapped("%s", description.data());
                        ImGui::PopStyleColor();
                        ImGui::EndTooltip();
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

        mTouchEditVisitor.SetEditHandler([&](int32_t nodeId, int8_t channelId, float, float dy) {
            if (mNGSchema == nullptr) {
                return;
            }

            auto node = mNGSchema->GetNode(nodeId);
            if (node->IsInputDataEditable(channelId) && !node->IsInputDataText(channelId)) {
                float* nodeValue = nullptr;
                if (channelId < node->GetNumInputs()) {
                    nodeValue = &node->GetInput(channelId, 1);
                }
                else if (channelId < node->GetNumOutputs()) {
                    nodeValue = &node->GetOutput(channelId, 1);
                }
                if (nodeValue != nullptr) {
                    if (!ImGui::IsKeyDown(ImGuiKey::ImGuiKey_LeftAlt)) {
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

        mTextEditVisitor.SetEditHandler([&](int32_t nodeId, int8_t channelId, std::string& text, bool noedit) {
            if (mNGSchema == nullptr) {
                return;
            }

            auto node = mNGSchema->GetNode(nodeId);
            if (noedit && node->IsInputDataEditable(channelId) && node->IsInputDataText(channelId)) {
                if (channelId < node->GetNumInputs()) {
                    text = node->GetInputText(channelId);
                }
                else if (channelId < node->GetNumOutputs()) {
                    text = node->GetOutputText(channelId);
                }
            }

            ImGuiIO& io = ImGui::GetIO();
            for (int i = 0; i < io.InputQueueCharacters.Size; i++) {
                ImWchar c = io.InputQueueCharacters[i];
                if (text.size() <= 16) {
                    text += static_cast<char>(c);
                }
            }
            if (!noedit && channelId < node->GetNumInputs()) {
                node->SetInput(channelId, text);
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

        mMoveVisitor.SetMoveHandler([&](int32_t, int32_t nodeId, float x, float y) {
            if (mNGSchema == nullptr) {
                return;
            }
            //LLOG(LogInfo) << "Container " << containerId << " moved to " << x << ", " << y;
            auto node = mNGSchema->GetNode(nodeId);
            if (node != nullptr) {
                auto& uiData = node->GetUIData();
                uiData.x = x;
                uiData.y = y;
            }
            });

        mResizeVisitor.SetResizeHandler([&](int32_t, int32_t nodeId, float w, float h) {
            if (mNGSchema == nullptr) {
                return;
            }
            //LLOG(LogInfo) << "Container " << containerId << " resized to " << w << ", " << h;
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

    void UINodeEditor::SetOverlayContentWindow(std::function<void(UINodeEditor&)> action) {
        mOverlayContentWindow = action;
    }

    void UINodeEditor::SetNGSchema(l::nodegraph::NodeGraphSchema* ngSchema) {
        mDrawVisitor.Reset();
        mLinkIOVisitor.Reset();
        mSelectVisitor.Reset();
        mZoomVisitor.Reset();
        mDragVisitor.Reset();
        mMoveVisitor.Reset();
        mResizeVisitor.Reset();
        mTouchEditVisitor.Reset();
        mTextEditVisitor.Reset();

        if (mUIRoot.IsValid()) {
            mUIRoot->RemoveAll();
        }
        mUIRoot.Reset();
        mUIManager.Reset();

        if (ngSchema == nullptr) {
            SetName("");
            return;
        }

        mNGSchema = ngSchema;

        auto editorName = mNGSchema->GetFileName();
        if (editorName.empty()) {
            mNGSchema->SetFileName("schema.json");
        }

        SetName(mNGSchema->GetFileName());

        mUIRoot = CreateContainer(mUIManager, l::ui::UIContainer_DragFlag | l::ui::UIContainer_ZoomFlag);

        mNGSchema->ForEachNode([&](nodegraph::NodeGraphBase* node) {
            if (node != nullptr) {
                auto& uiData = node->GetUIData();
                auto p = ImVec2(uiData.x, uiData.y);
                auto s = ImVec2(uiData.w, uiData.h);
                auto uiNode = l::ui::CreateUINode(mUIManager, *node, p, s);

                //LLOG(LogInfo) << "Replicated node type " << node->GetTypeId() << " as a ui node";
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

        if (IsShowing()) {
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigWindowsMoveFromTitleBarOnly = true;

            mUIInput.mPrevPos.x = mUIInput.mCurPos.x;
            mUIInput.mPrevPos.y = mUIInput.mCurPos.y;
            mUIInput.mCurPos = io.MousePos;
            mUIInput.mScroll = io.MouseWheel;
            mUIInput.mStarted = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
            mUIInput.mStopped = ImGui::IsMouseReleased(ImGuiMouseButton_Left);

            if (IsHovered()) {
                if (mUIRoot->Accept(mLinkIOVisitor, mUIInput, l::ui::UITraversalMode::DFS)) {
                }
                else if (mUIRoot->Accept(mTouchEditVisitor, mUIInput, l::ui::UITraversalMode::DFS)) {
                }
                else if (mUIRoot->Accept(mTextEditVisitor, mUIInput, l::ui::UITraversalMode::DFS)) {
                }
                else if (mUIRoot->Accept(mSelectVisitor, mUIInput, l::ui::UITraversalMode::BFS)) {
                }
                else if (mUIRoot->Accept(mResizeVisitor, mUIInput, l::ui::UITraversalMode::DFS)) {
                }
                else if (mUIRoot->Accept(mMoveVisitor, mUIInput, l::ui::UITraversalMode::DFS)) {
                }
                else if (UpdateGroupDrag()) {
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
                mDraggingGroupId = -1;
            }
        }
    }

    bool UINodeEditor::UpdateGroupDrag() {
        if (!mNGSchema || !mUIRoot.IsValid()) return false;
        auto& groups = mNGSchema->GetLogicalGroups();
        if (groups.empty()) return false;

        float rootScale = mUIRoot->GetScale();
        ImVec2 rootPos  = mUIRoot->GetPosition();
        ImVec2 winPos   = GetPosition();

        // Convert screen position to canvas position
        auto toCanvas = [&](ImVec2 sp) -> ImVec2 {
            return { (sp.x - winPos.x - rootPos.x) / rootScale,
                     (sp.y - winPos.y - rootPos.y) / rootScale };
        };

        // Ongoing drag: apply movement each frame, persist on release
        if (mDraggingGroupId >= 0) {
            ImVec2 move = DragMovement(mUIInput.mPrevPos, mUIInput.mCurPos, rootScale);
            for (auto& group : groups) {
                if (group.mId != mDraggingGroupId) continue;
                for (int32_t nid : group.mNodeIds) {
                    auto* c = mUIManager.FindNodeId(UIContainer_MoveFlag, nid);
                    auto* node = mNGSchema->GetNode(nid);
                    if (c) {
                        c->Move(move);
                        // Update UIData every frame so the group rect (which reads UIData) follows immediately
                        if (node) {
                            auto p = c->GetPosition();
                            node->GetUIData().x = p.x;
                            node->GetUIData().y = p.y;
                        }
                    }
                }
                if (group.mLabelX != 0.0f || group.mLabelY != 0.0f) {
                    group.mLabelX += move.x;
                    group.mLabelY += move.y;
                }
                break;
            }
            if (mUIInput.mStopped) {
                mDraggingGroupId = -1;
            }
            return true;
        }

        // New press: check if it lands inside a group rect (but not on a node — mMoveVisitor already consumed node hits)
        if (!mUIInput.mStarted) return false;

        ImVec2 curCanvas = toCanvas(mUIInput.mCurPos);

        for (auto& group : groups) {
            if (group.mNodeIds.empty()) continue;
            float minX = FLT_MAX, minY = FLT_MAX, maxX = -FLT_MAX, maxY = -FLT_MAX;
            for (int32_t nid : group.mNodeIds) {
                auto* node = mNGSchema->GetNode(nid);
                if (!node) continue;
                auto& ui = node->GetUIData();
                constexpr float pad = 12.0f;
                minX = std::min(minX, ui.x - pad);
                minY = std::min(minY, ui.y - pad);
                maxX = std::max(maxX, ui.x + ui.w + pad);
                maxY = std::max(maxY, ui.y + ui.h + pad);
            }
            if (curCanvas.x >= minX && curCanvas.x <= maxX &&
                curCanvas.y >= minY && curCanvas.y <= maxY) {
                mDraggingGroupId = group.mId;
                return true;
            }
        }
        return false;
    }

    void UINodeEditor::AddSchemaNodeToUI(int32_t nodeId) {
        if (!mNGSchema || !mUIRoot.IsValid()) return;
        auto* node = mNGSchema->GetNode(nodeId);
        if (!node) return;
        auto& uiData = node->GetUIData();
        auto p = ImVec2(uiData.x, uiData.y);
        auto s = ImVec2(uiData.w, uiData.h);
        auto uiNode = CreateUINode(mUIManager, *node, p, s);
        mUIRoot->Add(uiNode);
    }

    void UINodeEditor::AddSchemaLinksToUI(int32_t nodeId) {
        if (!mNGSchema || !mUIRoot.IsValid()) return;
        auto* node = mNGSchema->GetNode(nodeId);
        if (!node) return;
        int inputChannel = 0;
        node->ForEachInput([&](l::nodegraph::NodeGraphInput& input) {
            if (input.HasInputNode()) {
                auto* outputNode = input.GetInputNode();
                auto outputChannel = input.GetInputSrcChannel();
                auto linkContainer = CreateContainer(mUIManager, UIContainer_LinkFlag | UIContainer_DrawFlag, UIRenderType::LinkH);
                linkContainer->SetColor(l::ui::pastellYellow);
                auto* outputContainer = mUIManager.FindNodeId(UIContainer_OutputFlag, outputNode->GetId(), outputChannel);
                auto* inputContainer = mUIManager.FindNodeId(UIContainer_InputFlag, nodeId, inputChannel);
                if (outputContainer && inputContainer) {
                    outputContainer->Add(linkContainer);
                    linkContainer->SetCoParent(inputContainer);
                    inputContainer->SetCoParent(linkContainer.Get());
                }
            }
            inputChannel++;
        });
    }

    void UINodeEditor::GetSelectedNodeIds(std::vector<int32_t>& out) {
        mSelectVisitor.GetSelectedNodeIds(out);
    }
}
