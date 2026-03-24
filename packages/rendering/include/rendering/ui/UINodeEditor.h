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

    void depthFirstTraversal(const nodegraph::TreeMenuNode& node, std::vector<std::string>& path, std::function<void(std::string_view, int32_t, std::string_view)> cbMenuItem);

    struct NodeEvent {
        l::nodegraph::NodeGraphSchema* mNodeSchema = nullptr;
        int32_t mNodeEvent = 0;
        int32_t mContainerId = 0;
        int32_t mNodeId = 0;
    };

    class UINodeEditor : public UIWindow {
    public:
        UINodeEditor(std::string_view editorName) : UIWindow(editorName), mLinkIOVisitor(mUIManager), mSelectVisitor(mUIManager) {
            Init();
        }
        ~UINodeEditor() = default;

        void Init();

        void Update(double, float) override;

        void SetOverlayContentWindow(std::function<void(UINodeEditor&)> action);
        void SetNGSchema(l::nodegraph::NodeGraphSchema* ngSchema);
        void SetEventListener(std::function<void(const NodeEvent& event)> cb);

        l::nodegraph::NodeGraphSchema* GetNGSchema();

        // Add a single schema node + its links to the editor UI (for incremental import)
        void AddSchemaNodeToUI(int32_t nodeId);
        void AddSchemaLinksToUI(int32_t nodeId);
        // Expose current selection node IDs
        void GetSelectedNodeIds(std::vector<int32_t>& out);
    protected:
        UIManager mUIManager;
        UIHandle mUIRoot;
        InputState mUIInput;

        UIDraw mDrawVisitor;
        UILinkIO mLinkIOVisitor;
        UISelect mSelectVisitor;
        UIZoom mZoomVisitor;
        UIDrag mDragVisitor;
        UIMove mMoveVisitor;
        UIResize mResizeVisitor;
        UITouchEdit mTouchEditVisitor;
        UITextEdit mTextEditVisitor;

        l::nodegraph::NodeGraphSchema* mNGSchema = nullptr;

        std::vector<std::function<void(const NodeEvent&)>> mEventListeners;
        std::function<void(UINodeEditor&)> mOverlayContentWindow = nullptr;

        int32_t mDraggingGroupId = -1;

        bool UpdateGroupDrag();

        //l::string::string_buffer<20> mPickerSearch;
    };
}
