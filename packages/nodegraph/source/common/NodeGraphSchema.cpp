#include "nodegraph/NodeGraphSchema.h"

#include "logging/Log.h"

#include <set>

namespace l::nodegraph {

    void NodeGraphSchema::SetCustomCreator(std::function<CustomCreateFunctionType> customCreator) {
        mCreateCustomNode = customCreator;
    }

    void NodeGraphSchema::SetKeyState(l::hid::KeyState* keyState) {
        mKeyState = keyState;
    }

    void NodeGraphSchema::SetAudioOutput(l::audio::AudioStream* audioOutput) {
        mAudioOutput = audioOutput;
    }

    int32_t NodeGraphSchema::NewNode(int32_t typeId) {
        l::nodegraph::NodeGraphBase* node = nullptr;
        switch (typeId) {
        case 0:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphSourceConstants>(OutputType::Default, 0);
            break;
        case 1:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphSourceConstants>(OutputType::Default, 1);
            break;
        case 2:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphSourceConstants>(OutputType::Default, 2);
            break;
        case 3:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphSourceConstants>(OutputType::Default, 3);
            break;
        case 4:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphSourceKeyboard, l::hid::KeyState>(OutputType::Default, 1, mKeyState);
            break;
        case 5:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphSourceTime>(OutputType::Default);
            break;
        case 6:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphSourceSine>(OutputType::Default);
            break;
        case 50:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphNumericAdd>(OutputType::Default);
            break;
        case 51:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphNumericSubtract>(OutputType::Default);
            break;
        case 52:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphNumericNegate>(OutputType::Default);
            break;
        case 53:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphNumericMultiply>(OutputType::Default);
            break;
        case 54:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphNumericIntegral>(OutputType::Default);
            break;
        case 100:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphLogicalAnd>(OutputType::Default);
            break;
        case 101:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphLogicalOr>(OutputType::Default);
            break;
        case 102:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphLogicalXor>(OutputType::Default);
            break;
        case 150:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphFilterLowpass>(OutputType::Default);
            break;
        case 151:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphFilterEnvelope>(OutputType::Default);
            break;
        case 200:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphOutputDebug>(OutputType::ExternalOutput, 1);
            break;
        case 201:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphOutputSpeaker>(OutputType::ExternalOutput, mAudioOutput);
            break;
        case 250:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphEffectReverb1>(OutputType::Default);
            break;
        case 251:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphEffectReverb2>(OutputType::Default);
            break;
        default:
            ASSERT(typeId < 10000) << "Custom node id's begin at id 1000";
            ASSERT(mCreateCustomNode != nullptr) << "Custom nodes needs a handler to create them. It's missing.";
            node = mCreateCustomNode(typeId, mMainNodeGraph);
            break;
        };

        return node == nullptr ? 0 : node->GetId();
    }

    bool NodeGraphSchema::RemoveNode(int32_t nodeId) {
        return mMainNodeGraph.RemoveNode(nodeId);
    }

    NodeGraphBase* NodeGraphSchema::GetNode(int32_t nodeId) {
        return mMainNodeGraph.GetNode(nodeId);
    }

    void NodeGraphSchema::ForEachNodeType(std::function<void(std::string_view, const std::vector<UINodeDesc>&)> cb) const {
        for (auto it : mRegisteredNodeTypes) {
            cb(it.first, it.second);
        }
    }

    void NodeGraphSchema::RegisterNodeType(const std::string& typeGroup, int32_t uniqueTypeId, std::string_view typeName) {
        UINodeDesc nodeDesc;
        nodeDesc.mId = uniqueTypeId;
        nodeDesc.mName = typeName;
        mRegisteredNodeTypes[typeGroup].push_back(UINodeDesc{ uniqueTypeId, std::string(typeName) });
    }

    void NodeGraphSchema::Tick(float time, float deltaTime) {
        mMainNodeGraph.Tick(time, deltaTime);
    }

    void NodeGraphSchema::ProcessSubGraph() {
        mMainNodeGraph.ProcessSubGraph(true);
    }


}