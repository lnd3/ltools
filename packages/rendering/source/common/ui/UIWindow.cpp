#include "rendering/ui/UIWindow.h"

#include <memory>

namespace l::ui {

    bool UIWindow::IsShowing() {
        return mWindowPtr && mOpened;
    }

    void UIWindow::SetContentWindow(std::function<void()> action) {
        mWindowFunction = action;
    }

    void UIWindow::SetPointerPopup(std::function<void()> popup) {
        mPointerPopupMenu = popup;
    }

    void UIWindow::Open() {
        mOpened = true;
    }

    void UIWindow::Close() {
        mOpened = false;
    }

    bool UIWindow::IsHovered() {
        return mWindowPtr && mIsHovered;
    }

    ImVec2 UIWindow::GetPosition() {
        return mWindowPtr->DC.CursorPos;
    }

    ImVec2 UIWindow::GetSize() {
        return mWindowPtr->Size;
    }

    void UIWindow::Show() {
        ImGui::PushStyleColor(ImGuiCol_WindowBg, mBgColor);
        if (ImGui::Begin(mWindowName.c_str(), &mOpened, ImGuiWindowFlags_MenuBar)) {
            mWindowPtr = ImGui::GetCurrentWindowRead();
            mIsHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup);
            mIsFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_NoPopupHierarchy);

            if (mWindowPtr && mWindowFunction) {
                mWindowFunction();
            }

            if (mPointerPopupMenu) {
                if (mIsHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
                    ImGui::OpenPopup(mPopupName.c_str(), ImGuiPopupFlags_NoOpenOverExistingPopup);
                    ImGuiIO& io = ImGui::GetIO();
                    ImGui::SetNextWindowPos(io.MousePos, ImGuiCond_Appearing, ImVec2(0.0f, 0.0f));
                }
                if (ImGui::BeginPopup(mPopupName.c_str(), ImGuiWindowFlags_AlwaysAutoResize)) {
                    mPointerPopupMenu();
                    ImGui::EndPopup();
                }
            }
        }
        if (!mOpened) {
            mIsHovered = false;
            mIsFocused = false;
        }
        ImGui::End();
        ImGui::PopStyleColor();
    }

    void UIWindow::SetBgColor(ImVec4 bgColor) {
        mBgColor = bgColor;
    }


}
