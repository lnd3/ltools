#include "rendering/ui/UINodeEditor.h"

#include <memory>

namespace l::ui {

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

            mNGSchema->ForEachNodeType([&](std::string_view typeName, const std::vector<l::nodegraph::UINodeDesc>& types) {
                if (typeName.empty() || ImGui::TreeNode(typeName.data())) {
                    for (auto& it : types) {
                        if (ImGui::MenuItem(it.GetName().data())) {
                            ImVec2 p = ImVec2(mUIInput.mCurPos.x - mUIWindow.GetPosition().x, mUIInput.mCurPos.y - mUIWindow.GetPosition().y);
                            p.x -= mUIRoot->GetPosition().x;
                            p.y -= mUIRoot->GetPosition().y;
                            p.x /= mUIRoot->GetScale();
                            p.y /= mUIRoot->GetScale();
                            p.x -= 3.0f;
                            p.y -= 3.0f;
                            auto nodeId = mNGSchema->NewNode(it.GetId());
                            auto node = mNGSchema->GetNode(nodeId);
                            if (node != nullptr) {
                                auto uiNode = l::ui::CreateUINode(mUIManager, *node, p);
                                mUIRoot->Add(uiNode);
                            }
                        }
                    }
                    if (!typeName.empty()) {
                        ImGui::TreePop();
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
    }

    void UINodeEditor::Update() {

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
        }
    }

}
