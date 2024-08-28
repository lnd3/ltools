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
        if (mOpened) {
            ImGui::PushStyleColor(ImGuiCol_WindowBg, mBgColor);
            if (ImGui::Begin(mWindowName.c_str(), &mOpened, ImGuiWindowFlags_MenuBar)) {
                if (mOpened) {
                    mWindowPtr = ImGui::GetCurrentWindowRead();
                    mIsHovered = ImGui::IsWindowHovered();;
                }

                if (mWindowPtr && mWindowFunction) {
                    mWindowFunction();
                }

                if (mPointerPopupMenu) {
                    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
                        ImGui::OpenPopup("PointerPopupMenu");
                        mPointerPopupOpen = true;
                    }
                    if (mPointerPopupOpen) {
                        ImGuiIO& io = ImGui::GetIO();
                        ImGui::SetNextWindowPos(io.MousePos, ImGuiCond_Appearing, ImVec2(0.0f, 0.0f));
                    }
                    if (ImGui::BeginPopup("PointerPopupMenu", ImGuiWindowFlags_AlwaysAutoResize)) {
                        mPointerPopupMenu();
                        ImGui::EndPopup();
                    }
                    else {
                        mPointerPopupOpen = false;
                    }
                }

                if (!mOpened) {
                    mWindowPtr = nullptr;
                    mIsHovered = false;
                }
            }
            ImGui::End();
            ImGui::PopStyleColor();
        }
    }

    void UIWindow::SetBgColor(ImVec4 bgColor) {
        mBgColor = bgColor;
    }


}
