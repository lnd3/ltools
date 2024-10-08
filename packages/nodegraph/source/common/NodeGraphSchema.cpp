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

    void NodeGraphSchema::SetMidiManager(l::hid::midi::MidiManager* midiManager) {
        mMidiManager = midiManager;
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
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphSourceTime>(OutputType::ExternalOutput, mAudioOutput != nullptr ? mAudioOutput->GetSampleRate() : 44100, 60);
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
        case 55:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphNumericMultiply3>(OutputType::Default);
            break;
        case 56:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphNumericMultiplyAndAdd>(OutputType::Default);
            break;
        case 57:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphNumericRound>(OutputType::Default);
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
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphFilterHighpass>(OutputType::Default);
            break;
        case 152:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphFilterChamberlain2pole>(OutputType::Default);
            break;

        case 200:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphOutputDebug>(OutputType::ExternalOutput);
            break;
        case 201:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphOutputSpeaker>(OutputType::ExternalOutput, mAudioOutput);
            break;
        case 202:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphOutputPlot>(OutputType::ExternalVisualOutput, 100);
            break;

        case 251:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphEffectReverb1>(OutputType::Default);
            break;
        case 252:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphEffectReverb2>(OutputType::Default);
            break;
        case 254:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphEffectLimiter>(OutputType::Default);
            break;
        case 255:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphEffectEnvelopeFollower>(OutputType::Default);
            break;
        case 256:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphEffectSaturator>(OutputType::Default);
            break;
        case 257:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphEffectTranceGate>(OutputType::Default);
            break;

        case 300:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphInputKeyboardPiano>(OutputType::Default, mKeyState);
            break;
        case 301:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphInputMidiKeyboard>(OutputType::Default, mMidiManager);
            break;
        case 302:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphInputMidiKnobs>(OutputType::Default, mMidiManager);
            break;
        case 303:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphInputMidiButtons>(OutputType::Default, mMidiManager, 0);
            break;
        case 304:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphInputMidiButtons>(OutputType::Default, mMidiManager, 1);
            break;
        case 305:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphInputMidiButtons>(OutputType::Default, mMidiManager, 2);
            break;
        case 306:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphInputMidiButtons>(OutputType::Default, mMidiManager, 3);
            break;
        case 307:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphInputMidiButtons>(OutputType::Default, mMidiManager, 4);
            break;

        case 350:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphSignalSine>(OutputType::Default);
            break;
        case 351:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphSignalSineFM>(OutputType::Default);
            break;
        case 352:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphSignalSineFM2>(OutputType::Default);
            break;
        case 353:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphSignalSineFM3>(OutputType::Default);
            break;
        case 354:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphSignalSaw>(OutputType::Default);
            break;
        case 355:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphSignalSine2>(OutputType::Default);
            break;
        case 356:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphSignalSaw2>(OutputType::Default);
            break;

        case 400:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphControlEnvelope>(OutputType::Default);
            break;
        case 401:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphControlArpeggio>(OutputType::Default);
            break;

        default:
            ASSERT(typeId < 10000) << "Custom node id's begin at id 1000";
            if (mCreateCustomNode) {
                node = mCreateCustomNode(typeId, mMainNodeGraph);
            }
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
        for (auto& it : mRegisteredNodeTypes) {
            cb(it.first, it.second);
        }
    }

    void NodeGraphSchema::RegisterNodeType(const std::string& typeGroup, int32_t uniqueTypeId, std::string_view typeName) {
        UINodeDesc nodeDesc;
        nodeDesc.mId = uniqueTypeId;
        nodeDesc.mName = typeName;
        mRegisteredNodeTypes[typeGroup].push_back(UINodeDesc{ uniqueTypeId, std::string(typeName) });
    }

    void NodeGraphSchema::Tick(int32_t tickCount, float deltaTime) {
        mMainNodeGraph.Tick(tickCount, deltaTime);
    }

    void NodeGraphSchema::ProcessSubGraph(int32_t numSamples) {
        mMainNodeGraph.ProcessSubGraph(numSamples);
    }


}