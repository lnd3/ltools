#include "rendering/ui/UICreator.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

#include <memory>

namespace l::ui {


    UIHandle CreateUINode(UIManager& uiManager, l::nodegraph::NodeGraphBase& node, ImVec2 p) {


        auto numInputChannels = node.GetNumInputs();
        auto numOutputChannels = node.GetNumOutputs();
        auto numRows = numInputChannels > numOutputChannels ? numInputChannels : numOutputChannels;

        auto node4 = CreateSplit(uiManager, l::ui::UIContainer_ResizeFlag | l::ui::UIContainer_MoveFlag | l::ui::UIContainer_DrawFlag | UIContainer_SelectFlag, l::ui::UIRenderType::RectFilled, l::ui::UISplitMode::AppendV);
        node4->SetColor(ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
        node4->SetPosition(p);
        node4->GetContainerArea().mMargin = 0.0f;
        node4->SetNodeId(node.GetId());
        ImVec2 sizeEstimate = ImVec2(100.0f, 22.0f + 20.0f * numRows);
        {
            float ioSize = 4.0f;
            float ioOffsetV = 1.6f;

            auto row0 = CreateContainer(uiManager, l::ui::UIContainer_DrawFlag, l::ui::UIRenderType::RectFilled, l::ui::UIAlignH::Left, l::ui::UIAlignV::Top, l::ui::UILayoutH::Parent, l::ui::UILayoutV::Fixed);
            row0->SetSize(ImVec2(1.0f, 18.0f));
            row0->GetContainerArea().mMargin = 2.0f;
            node4->Add(row0);
            {
                auto connector1Text = CreateContainer(uiManager, l::ui::UIContainer_DrawFlag, l::ui::UIRenderType::Text, l::ui::UIAlignH::Center, l::ui::UIAlignV::Middle);
                connector1Text->SetDisplayName(node.GetName());
                auto textSize = ImGui::CalcTextSize(node.GetName().data());
                sizeEstimate.x = sizeEstimate.x < textSize.x ? textSize.x : sizeEstimate.x;
                connector1Text->GetContainerArea().mRender.mColor = ImColor(ImVec4(0.5f, 1.0f, 0.4f, 1.0f));
                row0->Add(connector1Text);
            }
            for (int8_t i = 0; i < numInputChannels || i < numOutputChannels; i++) {
                auto row = CreateContainer(uiManager, 0, l::ui::UIRenderType::Rect, l::ui::UIAlignH::Left, l::ui::UIAlignV::Top, l::ui::UILayoutH::Parent, l::ui::UILayoutV::Fixed);
                row->GetContainerArea().mMargin = ioSize;
                node4->Add(row);
                {
                    bool showsInput = i < numInputChannels;
                    bool showsOutput = i < numOutputChannels;

                    float estimatedWidth = 0.0f;
                    if (showsInput && !node.IsDataConstant(i)) {
                        auto in = CreateContainer(uiManager, l::ui::UIContainer_InputFlag | l::ui::UIContainer_DrawFlag, l::ui::UIRenderType::CircleFilled, l::ui::UIAlignH::Left);
                        in->SetPosition(ImVec2(-ioSize, ioSize * ioOffsetV));
                        in->SetSize(ImVec2(ioSize, ioSize));
                        in->GetContainerArea().mMargin = 0.0f;
                        in->SetNodeId(node.GetId());
                        in->SetChannelId(i);
                        estimatedWidth += ioSize;
                        row->Add(in);
                        auto inText = CreateContainer(uiManager, l::ui::UIContainer_DrawFlag, l::ui::UIRenderType::Text, l::ui::UIAlignH::Left);
                        inText->SetPosition(ImVec2(0.0f, 0.0f));
                        inText->SetDisplayName(node.GetInputName(i));
                        estimatedWidth += ImGui::CalcTextSize(node.GetInputName(i).data()).x;
                        row->Add(inText);
                    }

                    if (node.IsDataVisible(i)) {
                        auto inText = CreateContainer(uiManager, (node.IsDataEditable(i) ? l::ui::UIContainer_EditFlag : 0) | l::ui::UIContainer_DrawFlag, l::ui::UIRenderType::NodeOutputValue, l::ui::UIAlignH::Left);
                        inText->SetPosition(ImVec2(estimatedWidth, 0.0f));
                        inText->SetSize(ImVec2(10 * 7, 14.0f));
                        inText->SetNodeId(node.GetId());
                        inText->SetChannelId(i);
                        estimatedWidth += 10 * 7 + 10;
                        row->Add(inText);
                    }

                    if (showsOutput) {
                        auto out = CreateContainer(uiManager, l::ui::UIContainer_OutputFlag | l::ui::UIContainer_DrawFlag, l::ui::UIRenderType::CircleFilled, l::ui::UIAlignH::Right);
                        out->SetPosition(ImVec2(ioSize * 2.0f, ioSize * ioOffsetV));
                        out->SetSize(ImVec2(ioSize, ioSize));
                        out->GetContainerArea().mMargin = 0.0f;
                        out->SetNodeId(node.GetId());
                        out->SetChannelId(i);
                        estimatedWidth += ioSize;
                        row->Add(out);
                        auto outText = CreateContainer(uiManager, l::ui::UIContainer_DrawFlag, l::ui::UIRenderType::Text, l::ui::UIAlignH::Right);
                        outText->SetPosition(ImVec2(0.0f, 0.0f));
                        outText->SetDisplayName(node.GetOutputName(i));
                        estimatedWidth += ImGui::CalcTextSize(node.GetOutputName(i).data()).x;
                        row->Add(outText);
                    }
                    sizeEstimate.x = sizeEstimate.x < estimatedWidth ? estimatedWidth : sizeEstimate.x;
                }
            }

            if (node.GetOutputType() == l::nodegraph::NodeType::ExternalVisualOutput) {
                auto row = CreateContainer(uiManager, l::ui::UIContainer_DrawFlag, l::ui::UIRenderType::Rect, l::ui::UIAlignH::Left, l::ui::UIAlignV::Bottom, l::ui::UILayoutH::Parent, l::ui::UILayoutV::Parent);
                row->SetPosition(ImVec2(0.0f, 0.0f));
                row->GetContainerArea().mMargin = ioSize;
                node4->Add(row);

                float estimatedWidth = 0.0f;
                auto plot = CreateContainer(uiManager, l::ui::UIContainer_DrawFlag, l::ui::UIRenderType::NodeOutputGraph, l::ui::UIAlignH::Center, l::ui::UIAlignV::Bottom, l::ui::UILayoutH::Parent, l::ui::UILayoutV::Parent);
                plot->SetPosition(ImVec2(estimatedWidth, 0.0f));
                plot->SetSize(ImVec2(100, 100));
                plot->SetNodeId(node.GetId());
                plot->SetChannelId(0);
                estimatedWidth += 100;
                row->Add(plot);

                sizeEstimate.x = sizeEstimate.x < estimatedWidth ? estimatedWidth : sizeEstimate.x;
                sizeEstimate.y += 100.0f;
            }

        }

        sizeEstimate.x += node4->GetContainerArea().mMargin * 2 + 2.0f;
        node4->SetSize(sizeEstimate);

        return node4;
    };

}
