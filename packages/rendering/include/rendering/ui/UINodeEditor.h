#pragma once

#include "logging/LoggingAll.h"

#include "rendering/ui/UIContainer.h"
#include "rendering/ui/UIVisitors.h"
#include "rendering/ui/UIWindow.h"
#include "rendering/ui/UICreator.h"

#include "nodegraph/NodeGraphSchema.h"

#include <functional>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace l::ui {

    struct NodeEvent {
        l::nodegraph::NodeGraphSchema* mNodeSchema = nullptr;
        int32_t mNodeEvent = 0;
        int32_t mContainerId = 0;
        int32_t mNodeId = 0;
    };

    class UINodeEditor : public UIBase {
    public:
        UINodeEditor(std::string_view editorName) : mUIWindow(editorName), mLinkIOVisitor(mUIManager), mSelectVisitor(mUIManager) {
            Init();
        }
        ~UINodeEditor() = default;

        void Init();

        void Show() override;
        bool IsShowing() override;
        void Update(double, float) override;

        void Open();
        void Close();

        void SetNGSchema(l::nodegraph::NodeGraphSchema* ngSchema);
        void SetEventListener(std::function<void(const NodeEvent& event)> cb);
    protected:
        UIWindow mUIWindow;
        UIManager mUIManager;
        UIHandle mUIRoot;
        InputState mUIInput;

        l::nodegraph::NodeGraphSchema* mNGSchema = nullptr;

        UIDraw mDrawVisitor;
        UILinkIO mLinkIOVisitor;
        UISelect mSelectVisitor;
        UIZoom mZoomVisitor;
        UIDrag mDragVisitor;
        UIMove mMoveVisitor;
        UIResize mResizeVisitor;
        UIEdit mEditVisitor;

        std::vector<std::function<void(const NodeEvent&)>> mEventListeners;
    };
}
