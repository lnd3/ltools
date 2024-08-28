#include "rendering/ui/UINodeEditor.h"

#include <memory>

namespace l::ui {

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

    void UINodeEditor::Update() {
        if (mUIWindow.IsShowing()) {
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigWindowsMoveFromTitleBarOnly = true;

            mUIInput.mCurPos = io.MousePos;
            mUIInput.mPrevPos = io.MousePosPrev;
            mUIInput.mScroll = io.MouseWheel;
            mUIInput.mStarted = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
            mUIInput.mStopped = ImGui::IsMouseReleased(ImGuiMouseButton_Left);

            if (mUIWindow.IsHovered()) {
                if (mUIRoot->Accept(mLinkIOVisitor, mUIInput, l::ui::UITraversalMode::DFS)) {
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
