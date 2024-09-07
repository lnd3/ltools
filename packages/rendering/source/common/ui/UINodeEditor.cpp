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
}
