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
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphSourceConstants>(NodeType::Default, 0);
            break;
        case 1:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphSourceConstants>(NodeType::Default, 1);
            break;
        case 2:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphSourceConstants>(NodeType::Default, 2);
            break;
        case 3:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphSourceConstants>(NodeType::Default, 3);
            break;
        case 4:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphSourceTime>(NodeType::ExternalOutput, mAudioOutput != nullptr ? mAudioOutput->GetSampleRate() : 44100, 60);
            break;

        case 50:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphNumericAdd>(NodeType::Default);
            break;
        case 51:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphNumericSubtract>(NodeType::Default);
            break;
        case 52:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphNumericNegate>(NodeType::Default);
            break;
        case 53:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphNumericMultiply>(NodeType::Default);
            break;
        case 54:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphNumericIntegral>(NodeType::Default);
            break;
        case 55:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphNumericMultiply3>(NodeType::Default);
            break;
        case 56:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphNumericMultiplyAndAdd>(NodeType::Default);
            break;
        case 57:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphNumericRound>(NodeType::Default);
            break;
        case 58:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphNumericDerivate>(NodeType::Default);
            break;
        case 59:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphNumericChange>(NodeType::Default);
            break;

        case 100:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphLogicalAnd>(NodeType::Default);
            break;
        case 101:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphLogicalOr>(NodeType::Default);
            break;
        case 102:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphLogicalXor>(NodeType::Default);
            break;

        case 150:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphFilterLowpass>(NodeType::Default);
            break;
        case 151:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphFilterHighpass>(NodeType::Default);
            break;
        case 152:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphFilterChamberlain2pole>(NodeType::Default);
            break;
        case 153:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphFilterMovingAverage>(NodeType::Default, 150);
            break;

        case 200:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphOutputDebug>(NodeType::ExternalOutput);
            break;
        case 201:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphOutputSpeaker>(NodeType::ExternalOutput, mAudioOutput);
            break;
        case 202:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphOutputPlot>(NodeType::ExternalVisualOutput, 100);
            break;
        case 203:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphOutputImGuiPlotLine>(NodeType::ExternalOutput);
            break;
        case 204:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphOutputImGuiPlotCandles>(NodeType::ExternalOutput);
            break;

        case 251:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphEffectReverb1>(NodeType::Default);
            break;
        case 252:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphEffectReverb2>(NodeType::Default);
            break;
        case 254:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphEffectLimiter>(NodeType::Default);
            break;
        case 255:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphEffectEnvelopeFollower>(NodeType::Default);
            break;
        case 256:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphEffectSaturator>(NodeType::Default);
            break;
        case 257:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphEffectTranceGate>(NodeType::Default);
            break;

        case 300:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphInputKeyboardPiano>(NodeType::Default, mKeyState);
            break;
        case 301:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphInputMidiKeyboard>(NodeType::Default, mMidiManager);
            break;
        case 302:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphInputMidiKnobs>(NodeType::Default, mMidiManager);
            break;
        case 303:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphInputMidiButtons>(NodeType::Default, mMidiManager, 0);
            break;
        case 304:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphInputMidiButtons>(NodeType::Default, mMidiManager, 1);
            break;
        case 305:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphInputMidiButtons>(NodeType::Default, mMidiManager, 2);
            break;
        case 306:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphInputMidiButtons>(NodeType::Default, mMidiManager, 3);
            break;
        case 307:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphInputMidiButtons>(NodeType::Default, mMidiManager, 4);
            break;

        case 350:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphSignalSine>(NodeType::Default);
            break;
        case 351:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphSignalSineFM>(NodeType::Default);
            break;
        case 352:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphSignalSineFM2>(NodeType::Default);
            break;
        case 353:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphSignalSineFM3>(NodeType::Default);
            break;
        case 354:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphSignalSaw>(NodeType::Default);
            break;
        case 355:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphSignalSine2>(NodeType::Default);
            break;
        case 356:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphSignalSaw2>(NodeType::Default);
            break;

        case 400:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphControlEnvelope>(NodeType::Default);
            break;
        case 401:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphControlArpeggio>(NodeType::Default);
            break;

        case 450:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphDataBusDataIn>(NodeType::ExternalInput, 6);
            break;
        case 451:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphDataBusDataOut>(NodeType::ExternalOutput, 6);
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

    bool NodeGraphSchema::HasNodeType(const std::string& typeGroup, int32_t typeId) {
        if (mRegisteredNodeTypes[typeGroup].empty()) {
            return false;
        }

        for (auto& it : mRegisteredNodeTypes[typeGroup]) {
            if (it.mId == typeId) {
                return true;
            }
        }
        return false;
    }

    void NodeGraphSchema::ForEachInputNode(std::function<void(NodeGraphBase*)> cb) {
        mMainNodeGraph.ForEachInputNode(cb);
    }

    void NodeGraphSchema::ForEachOutputNode(std::function<void(NodeGraphBase*)> cb) {
        mMainNodeGraph.ForEachOutputNode(cb);
    }

    void NodeGraphSchema::ForEachNodeType(std::function<void(std::string_view, const std::vector<UINodeDesc>&)> cb) const {
        for (auto& it : mRegisteredNodeTypes) {
            cb(it.first, it.second);
        }
    }

    void NodeGraphSchema::RegisterNodeType(const std::string& typeGroup, int32_t uniqueTypeId, std::string_view typeName) {
        if (!HasNodeType(typeGroup, uniqueTypeId)) {
            mRegisteredNodeTypes[typeGroup].push_back(UINodeDesc{ uniqueTypeId, std::string(typeName) });
        }
    }

    void NodeGraphSchema::RegisterAllOf(const std::string& typeGroup) {
        if (typeGroup == "Source") {
            RegisterNodeType("Source", 0, "Value [0,1]");
            RegisterNodeType("Source", 1, "Value [-1,1]");
            RegisterNodeType("Source", 2, "Value [0,100]");
            RegisterNodeType("Source", 3, "Value [-inf,inf]");
            RegisterNodeType("Source", 4, "Time");
        }
        else if (typeGroup == "Numeric") {
            RegisterNodeType("Numeric", 50, "Add");
            RegisterNodeType("Numeric", 51, "Subtract");
            RegisterNodeType("Numeric", 52, "Negate");
            RegisterNodeType("Numeric", 53, "Multiply");
            RegisterNodeType("Numeric", 54, "Integral");
            RegisterNodeType("Numeric", 55, "Multiply3");
            RegisterNodeType("Numeric", 56, "Multiply & Add");
            RegisterNodeType("Numeric", 57, "Round");
            RegisterNodeType("Numeric", 58, "Derivate");
            RegisterNodeType("Numeric", 59, "Change");
        }
        else if (typeGroup == "Logic") {
            RegisterNodeType("Logic", 100, "And");
            RegisterNodeType("Logic", 101, "Or");
            RegisterNodeType("Logic", 102, "Xor");
        }
        else if (typeGroup == "Filter") {
            RegisterNodeType("Filter", 150, "Lowpass");
            RegisterNodeType("Filter", 151, "Highpass");
            RegisterNodeType("Filter", 152, "Chamberlin two-pole (4 mode)");
            RegisterNodeType("Filter", 153, "Moving Average");
        }
        else if (typeGroup == "Output") {
            RegisterNodeType("Output", 200, "Debug");
            RegisterNodeType("Output", 201, "Speaker");
            RegisterNodeType("Output", 202, "Plot");
            RegisterNodeType("Output", 203, "ImGui Plot Lines");
            RegisterNodeType("Output", 204, "ImGui Plot Candles");
        }
        else if (typeGroup == "Effect") {
            RegisterNodeType("Effect", 251, "Reverb1");
            RegisterNodeType("Effect", 252, "Reverb2");
            RegisterNodeType("Effect", 254, "Limiter");
            RegisterNodeType("Effect", 255, "Envelope Follower");
            RegisterNodeType("Effect", 256, "Saturator");
            RegisterNodeType("Effect", 257, "Trance Gate");
        }
        else if (typeGroup == "Input") {
            RegisterNodeType("Input", 300, "Keyboard Piano");
            RegisterNodeType("Input", 301, "Midi Keyboard");
            RegisterNodeType("Input", 302, "Midi Knobs");
            RegisterNodeType("Input", 303, "Midi Button Group 1");
            RegisterNodeType("Input", 304, "Midi Button Group 2");
            RegisterNodeType("Input", 305, "Midi Button Group 3");
            RegisterNodeType("Input", 306, "Midi Button Group 4");
            RegisterNodeType("Input", 307, "Midi Button Group 5");
        }
        else if (typeGroup == "Signal") {
            RegisterNodeType("Signal", 350, "Sine");
            RegisterNodeType("Signal", 351, "Sine FM 1");
            RegisterNodeType("Signal", 352, "Sine FM 2");
            RegisterNodeType("Signal", 353, "Sine FM 3");
            RegisterNodeType("Signal", 354, "Saw");
            RegisterNodeType("Signal", 355, "Sine 2");
            RegisterNodeType("Signal", 356, "Saw 2");
        }
        else if (typeGroup == "Control") {
            RegisterNodeType("Control", 400, "Envelope");
            RegisterNodeType("Control", 401, "Arpeggio");
        }
        else if (typeGroup == "Bus") {
            RegisterNodeType("Bus", 450, "Bus Data In x6");
            RegisterNodeType("Bus", 451, "Bus Data Out x6");
        }
    }

    void NodeGraphSchema::Tick(int32_t tickCount, float deltaTime) {
        mMainNodeGraph.Tick(tickCount, deltaTime);
    }

    void NodeGraphSchema::ProcessSubGraph(int32_t numSamples) {
        mMainNodeGraph.ProcessSubGraph(numSamples);
    }


}