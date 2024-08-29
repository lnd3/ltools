#include "rendering/ui/UICreator.h"

#include <memory>

namespace l::ui {


    UIHandle CreateUINode(UIStorage& uiStorage, l::nodegraph::NodeGraphBase& node, ImVec2 p) {

        auto numInputChannels = node.GetNumInputs();
        auto numOutputChannels = node.GetNumOutputs();
        auto numConstants = node.GetNumConstants();

        auto node4 = CreateSplit(uiStorage, l::ui::UIContainer_ResizeFlag | l::ui::UIContainer_MoveFlag | l::ui::UIContainer_DrawFlag | UIContainer_SelectFlag, l::ui::UIRenderType::RectFilled, l::ui::UISplitMode::AppendV);
        node4->SetPosition(p);
        node4->GetContainerArea().mMargin = 0.0f;
        node4->SetNodeId(node.GetId());
        ImVec2 titleSize = ImGui::CalcTextSize(node.GetName().data());
        titleSize.x += node4->GetContainerArea().mMargin * 2 + 2.0f;
        node4->SetSize(ImVec2(titleSize.x < 100.0f ? 100.0f : titleSize.x, 22.0f + 19.0f * (numInputChannels > numOutputChannels ? numInputChannels : numOutputChannels)));
        node4->SetColor(ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
        {
            float ioSize = 4.0f;
            float ioOffsetV = 1.6f;

            auto row0 = CreateContainer(uiStorage, l::ui::UIContainer_DrawFlag, l::ui::UIRenderType::RectFilled, l::ui::UIAlignH::Left, l::ui::UIAlignV::Top, l::ui::UILayoutH::Parent, l::ui::UILayoutV::Fixed);
            row0->SetSize(ImVec2(1.0f, 18.0f));
            row0->GetContainerArea().mMargin = 2.0f;
            node4->Add(row0);
            {
                auto connector1Text = CreateContainer(uiStorage, l::ui::UIContainer_DrawFlag, l::ui::UIRenderType::Text, l::ui::UIAlignH::Center, l::ui::UIAlignV::Middle);
                connector1Text->SetDisplayName(node.GetName());
                connector1Text->GetContainerArea().mRender.mColor = ImColor(ImVec4(0.5f, 1.0f, 0.4f, 1.0f));
                row0->Add(connector1Text);
            }
            for (int8_t i = 0; i < numInputChannels || i < numOutputChannels; i++) {
                auto row = CreateContainer(uiStorage, 0, l::ui::UIRenderType::Rect, l::ui::UIAlignH::Left, l::ui::UIAlignV::Top, l::ui::UILayoutH::Parent, l::ui::UILayoutV::Fixed);
                row->GetContainerArea().mMargin = ioSize;
                node4->Add(row);
                {
                    if (i < numInputChannels) {
                        auto in = CreateContainer(uiStorage, l::ui::UIContainer_InputFlag | l::ui::UIContainer_DrawFlag, l::ui::UIRenderType::CircleFilled, l::ui::UIAlignH::Left);
                        in->SetPosition(ImVec2(-ioSize, ioSize * ioOffsetV));
                        in->SetSize(ImVec2(ioSize, ioSize));
                        in->GetContainerArea().mMargin = 0.0f;
                        in->SetNodeId(node.GetId());
                        in->SetChannelId(i);
                        row->Add(in);
                        auto inText = CreateContainer(uiStorage, l::ui::UIContainer_DrawFlag, l::ui::UIRenderType::Text, l::ui::UIAlignH::Left);
                        inText->SetPosition(ImVec2(0.0f, 0.0f));
                        inText->SetDisplayName(node.GetInputName(i));
                        row->Add(inText);
                    }

                    if (i < numOutputChannels - numConstants) {
                        auto out = CreateContainer(uiStorage, l::ui::UIContainer_OutputFlag | l::ui::UIContainer_DrawFlag, l::ui::UIRenderType::CircleFilled, l::ui::UIAlignH::Right);
                        out->SetPosition(ImVec2(ioSize * 2.0f, ioSize * ioOffsetV));
                        out->SetSize(ImVec2(ioSize, ioSize));
                        out->GetContainerArea().mMargin = 0.0f;
                        out->SetNodeId(node.GetId());
                        out->SetChannelId(i);
                        row->Add(out);
                        auto outText = CreateContainer(uiStorage, l::ui::UIContainer_DrawFlag, l::ui::UIRenderType::Text, l::ui::UIAlignH::Right);
                        outText->SetPosition(ImVec2(0.0f, 0.0f));
                        outText->SetDisplayName(node.GetOutputName(i));
                        row->Add(outText);
                    }
                    else if (i < numOutputChannels) {
                        auto inText = CreateContainer(uiStorage, l::ui::UIContainer_DrawFlag, l::ui::UIRenderType::NodeOutputValue, l::ui::UIAlignH::Left);
                        inText->SetPosition(ImVec2(0.0f, 0.0f));
                        inText->SetDisplayName("");
                        inText->SetNodeId(node.GetId());
                        inText->SetChannelId(i);
                        row->Add(inText);

                        auto out = CreateContainer(uiStorage, l::ui::UIContainer_OutputFlag | l::ui::UIContainer_DrawFlag, l::ui::UIRenderType::CircleFilled, l::ui::UIAlignH::Right);
                        out->SetPosition(ImVec2(ioSize * 2.0f, ioSize * ioOffsetV));
                        out->SetSize(ImVec2(ioSize, ioSize));
                        out->GetContainerArea().mMargin = 0.0f;
                        out->SetNodeId(node.GetId());
                        out->SetChannelId(i);
                        row->Add(out);
                        auto outText = CreateContainer(uiStorage, l::ui::UIContainer_DrawFlag, l::ui::UIRenderType::Text, l::ui::UIAlignH::Right);
                        outText->SetPosition(ImVec2(0.0f, 0.0f));
                        outText->SetDisplayName(node.GetOutputName(i));
                        row->Add(outText);
                    }
                }
            }
        }
        return node4;
    };

}
