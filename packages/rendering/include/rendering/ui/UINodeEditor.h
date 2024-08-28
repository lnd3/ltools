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
#include <set>
#include <string>
#include <string_view>

namespace l::ui {

    class UINodeEditor : public UIBase {
    public:
        UINodeEditor(std::string_view editorName) : mUIWindow(editorName), mLinkIOVisitor(mUIStorage, mNGSchema) {
            mUIRoot = CreateContainer(mUIStorage, l::ui::UIContainer_DragFlag | l::ui::UIContainer_ZoomFlag);
            
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

                mNGSchema.ForEachNodeType([&](std::string_view typeName, const std::vector<l::nodegraph::UINodeDesc>& types) {
                    if (typeName.empty() || ImGui::TreeNode(typeName.data())) {
                        for (auto it : types) {
                            if (ImGui::MenuItem(it.GetName().data())) {
                                ImVec2 p = ImVec2(mUIInput.mCurPos.x - mUIWindow.GetPosition().x, mUIInput.mCurPos.y - mUIWindow.GetPosition().y);
                                p.x -= mUIRoot->GetPosition().x;
                                p.y -= mUIRoot->GetPosition().y;
                                p.x /= mUIRoot->GetScale();
                                p.y /= mUIRoot->GetScale();
                                p.x -= 3.0f;
                                p.y -= 3.0f;
                                auto nodeId = mNGSchema.NewNode(it.GetId());
                                auto node = mNGSchema.GetNode(nodeId);
                                if (node != nullptr) {
                                    auto uiNode = l::ui::CreateUINode(mUIStorage, *node, p);
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

        UIZoom mZoomVisitor;
        UIDrag mDragVisitor;
        UIMove mMoveVisitor;
        UIResize mResizeVisitor;
        UILinkIO mLinkIOVisitor;

    };

}
