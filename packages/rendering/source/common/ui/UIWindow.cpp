#include "rendering/ui/UIWindow.h"

#include <memory>

namespace l::ui {

    void UIWindow::SetContentWindow(std::function<void()> action) {
        mWindowFunction = action;
    }

    void UIWindow::SetPointerPopup(std::function<void()> popup) {
        mPointerPopupMenu = popup;
    }

    void UIWindow::Open() {
        mOpened = true;
    }

    bool UIWindow::IsShowing() {
        return mWindowPtr && mOpened;
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

    float UIWindow::GetScale() {
        return mContentScale;
    }

    bool UIWindow::TryInput(const ImVec2& prevPos, const ImVec2& curPos, float zoom, bool, bool) {
        if (TryPan(prevPos, curPos, ImGui::IsMouseClicked(ImGuiMouseButton_Left), ImGui::IsMouseReleased(ImGuiMouseButton_Left))) {
            return true;
        }
        else if (TryScale(curPos, zoom)) {
            return true;
        }
        return false;
    }

    bool UIWindow::TryPan(const ImVec2& prevPos, const ImVec2& curPos, bool moveStart, bool moveStop) {
        if (IsShowing()) {
            if (IsHovered() && moveStart) {
                mMoving = true;
            }
            if (mMoving) {
                ImVec2 drag = curPos;
                drag.x -= prevPos.x;
                drag.y -= prevPos.y;
                mContentPan.x += drag.x;
                mContentPan.y += drag.y;

                if (moveStop) {
                    mMoving = false;
                }
            }
        }
        return mMoving;
    }

    bool UIWindow::TryScale(const ImVec2& scalePos, float scroll) {
        if (IsShowing() && IsHovered() && scroll != 0.0f) {
            ImVec2 parentPos = GetPosition();
            float scaleChange = (1.0f + 0.1f * scroll);
            if ((mContentScale > 100.0f && scaleChange > 1.0f) || (mContentScale < 0.01f && scaleChange < 1.0f)) {
                return true;
            }
            mContentScale *= scaleChange;
            mContentPan.x = scalePos.x + (mContentPan.x + parentPos.x - scalePos.x) * scaleChange - parentPos.x;
            mContentPan.y = scalePos.y + (mContentPan.y + parentPos.y - scalePos.y) * scaleChange - parentPos.y;
            return true;
        }
        return false;
    }

    ImVec2 UIWindow::Transform(ImVec2 p, bool toWorld) {
        ImVec2 parentPos = GetPosition();

        float x = p.x * mContentScale;
        float y = p.y * mContentScale;
        x += mContentPan.x;
        y += mContentPan.y;
        if (toWorld) {
            x += parentPos.x;
            y += parentPos.y;
        }
        return ImVec2(x, y);
    }

    void UIWindow::Show() {
        if (mOpened) {
            ImGui::PushStyleColor(ImGuiCol_WindowBg, mBgColor);
            if (ImGui::Begin("Test primitive rendering", &mOpened)) {
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
