#pragma once

#include "logging/LoggingAll.h"

#include "rendering/ui/UIContainer.h"
#include "rendering/ui/UIVisitors.h"
#include "rendering/ui/UIWindow.h"
#include "rendering/ui/UICreator.h"

#include "nodegraph/NodeGraph.h"
#include "nodegraph/NodeGraphOperations.h"
#include "nodegraph/NodeGraphSchema.h"

#include <functional>
#include <vector>
#include <set>
#include <string>

namespace l::ui {

    struct UINodeDesc {
        std::string_view GetTypeName() {
            return mTypeName;
        }
        std::string_view GetName() {
            return mName;
        }
        int32_t GetId() {
            return mId;
        }

        int32_t mId;
        std::string mName;
        std::string mTypeName;
    };

    class UINodeEditor : public UIBase {
    public:
        UINodeEditor(std::string_view editorName) : mUIWindow(editorName), mLinkIOVisitor(mUIStorage, mNGSchema) {
            mUIRoot = CreateContainer(mUIStorage, l::ui::UIContainer_DragFlag | l::ui::UIContainer_ZoomFlag);
            
            mRegisteredNodeTypes.push_back({ 0, "Add", "Numerical" });
            mRegisteredNodeTypes.push_back({ 1, "Subtract", "Numerical" });
            mRegisteredNodeTypes.push_back({ 2, "Negate", "Numerical" });
            mRegisteredNodeTypes.push_back({ 3, "Multiply", "Numerical" });
            mRegisteredNodeTypes.push_back({ 4, "Integral", "Numerical" });
            mRegisteredNodeTypes.push_back({ 20, "And", "Logical" });
            mRegisteredNodeTypes.push_back({ 21, "Or", "Logical" });
            mRegisteredNodeTypes.push_back({ 22, "Xor", "Logical" });
            mRegisteredNodeTypes.push_back({ 40, "Lowpass Filter", "" });


            mUIWindow.SetContentWindow([&]() {
                ImGui::PushItemWidth(400);

                UIDraw uiDrawVisitor(ImGui::GetWindowDrawList());
                UIUpdate updateVisitor;

                mUIRoot->SetLayoutSize(mUIWindow.GetSize());
                mUIRoot->SetLayoutPosition(mUIWindow.GetPosition());
                mUIRoot->Accept(updateVisitor, mUIInput, l::ui::UITraversalMode::BFS);
                mUIRoot->Accept(uiDrawVisitor, mUIInput, l::ui::UITraversalMode::BFS);

                ImGui::PopItemWidth();

                });

            mUIWindow.SetPointerPopup([&]() {
                ImGui::Text("Node picker");
                ImGui::Separator();

                std::set<std::string> nodeTypes;
                for (auto nodeType : mRegisteredNodeTypes) {
                    nodeTypes.emplace(nodeType.GetTypeName());
                }

                for (auto it = nodeTypes.rbegin(); it != nodeTypes.rend(); it++) {
                    if (it->empty() || ImGui::TreeNode(it->c_str())) {
                        for (auto& nodedesc : mRegisteredNodeTypes) {
                            if (*it != nodedesc.GetTypeName()) {
                                continue;
                            }

                            if (ImGui::MenuItem(nodedesc.GetName().data())) {
                                ImVec2 p = ImVec2(mUIInput.mCurPos.x - mUIWindow.GetPosition().x, mUIInput.mCurPos.y - mUIWindow.GetPosition().y);
                                p.x -= mUIRoot->GetPosition().x;
                                p.y -= mUIRoot->GetPosition().y;
                                p.x /= mUIRoot->GetScale();
                                p.y /= mUIRoot->GetScale();
                                p.x -= 3.0f;
                                p.y -= 3.0f;
                                auto nodeId = mNGSchema.NewNode(nodedesc.GetId());
                                auto node = mNGSchema.GetNode(nodeId);
                                if (node != nullptr) {
                                    auto uiNode = l::ui::CreateUINode(mUIStorage, *node, p);
                                    mUIRoot->Add(uiNode);
                                }
                            }
                        }
                        if (!it->empty()) {
                            ImGui::TreePop();
                        }
                    }
                }

                });

        
        }
        ~UINodeEditor() = default;

        void Show() override;
        bool IsShowing() override;

        void Open();
        void Close();

        void Update();

    protected:
        UIWindow mUIWindow;
        UIStorage mUIStorage;
        UIHandle mUIRoot;
        InputState mUIInput;

        l::nodegraph::NodeGraphSchema mNGSchema;

        std::vector<UINodeDesc> mRegisteredNodeTypes;

        UIZoom mZoomVisitor;
        UIDrag mDragVisitor;
        UIMove mMoveVisitor;
        UIResize mResizeVisitor;
        UILinkIO mLinkIOVisitor;

    };

}
