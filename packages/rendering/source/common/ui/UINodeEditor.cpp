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
                    }
                }

                });

            });

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

    void UINodeEditor::SetNGSchema(l::nodegraph::NodeGraphSchema* ngSchema) {
        mNGSchema = ngSchema;
        mDrawVisitor.SetNGSchema(ngSchema);
        mLinkIOVisitor.SetNGSchema(ngSchema);
        mSelectVisitor.SetNGSchema(ngSchema);
        mEditVisitor.SetNGSchema(ngSchema);
        mSelectVisitor.SetDeleteHandler([&](int32_t containerId, int32_t nodeId) {
            NodeEvent event;
            event.mNodeSchema = ngSchema;
            event.mNodeEvent = 2; // 2 delete
            event.mContainerId = containerId;
            event.mNodeId = nodeId;

            for (auto& it : mEventListeners) {
                it(event);
            }

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
