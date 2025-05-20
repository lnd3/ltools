#include "nodegraph/NodeGraphSchema.h"

#include "logging/Log.h"

#include <set>

namespace l::nodegraph {

    TreeMenuNode* findOrCreateChild(TreeMenuNode& node, std::string_view pathPart) {
        for (auto& child : node.mChildren) {
            if (child.GetPathPart() == pathPart)
                return &child;
        }
        node.mChildren.emplace_back(pathPart, "", -1);
        return &node.mChildren.back();
    }

    // Insert a path like "a.b.c"
    void insertPath(TreeMenuNode& root, std::string_view path, std::string_view name, int32_t nodeId) {
        TreeMenuNode* current = &root;
        for (auto part : l::string::split(path, ".")) {
            current = findOrCreateChild(*current, part);
        }
        current->mChildren.emplace_back("", name, nodeId);
    }

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
            // Data sources from within the node graph (basically constants)
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
        case 5:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphSourceText>(NodeType::Default);
            break;

            // Numeric operators
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
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphNumericDiffNorm>(NodeType::Default);
            break;
        case 60:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphNumericDiff>(NodeType::Default);
            break;

            // Logical operators
        case 100:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphLogicalAnd>(NodeType::Default);
            break;
        case 101:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphLogicalOr>(NodeType::Default);
            break;
        case 102:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphLogicalXor>(NodeType::Default);
            break;
        case 103:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphLogicalDetector>(NodeType::Default);
            break;
        case 104:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphLogicalFlipGate>(NodeType::Default);
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
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphFilterMovingAverage>(NodeType::Default);
            break;

            // Internal output like NG chart or debug view
        case 200:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphOutputDebug>(NodeType::ExternalOutput);
            break;
        case 201:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphOutputPlot>(NodeType::ExternalVisualOutput, 100);
            break;

            // External outputs like speakers or external data sinks
        case 220:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphOutputPCBeep>(NodeType::ExternalOutput);
            break;
        case 221:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphOutputSpeaker>(NodeType::ExternalOutput, mAudioOutput);
            break;

            // Signal filtering and effects
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

            // System device inputs
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
        case 308:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphInputMic>(NodeType::Default, mAudioOutput);
            break;

            // Signal generators (basically audio composition)
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

            // Signal controllers (basically audio shaping)
        case 400:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphControlEnvelope>(NodeType::Default);
            break;
        case 401:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphControlArpeggio>(NodeType::Default);
            break;

            // Bus io (basically bulk data input/output from external and to external)
        case 450:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphDataBuffer>(NodeType::Default, 1);
            break;
        case 451:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphDataBuffer>(NodeType::Default, 2);
            break;
        case 452:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphDataBuffer>(NodeType::Default, 3);
            break;
        case 453:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphDataBuffer>(NodeType::Default, 4);
            break;

            // External input
        case 500:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphDataBusDataIn>(NodeType::ExternalInput, 6);
            break;
        case 501:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphDataOCHLVDataIn>(NodeType::ExternalInput);
            break;

            // External output
        case 550:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphDataBusDataOut>(NodeType::ExternalOutput, 6);
            break;
        case 551:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphDataPlaceTrade>(NodeType::ExternalOutput, 16);
            break;

            // UI elements (basically ui buttons/checkboxes on the ui using the schema containing the nodes)
        case 600:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphUICheckbox>(NodeType::ExternalInput);
            break;
        case 601:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphUISlider>(NodeType::ExternalInput, 0.0f, 1.0f, 1.0f);
            break;
        case 602:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphUIChartLine>(NodeType::ExternalOutput);
            break;
        case 603:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphUICandleSticks>(NodeType::ExternalOutput);
            break;
        //case 1000:
        //    node = mMainNodeGraph.NewGroup();
        //    break;

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

    void NodeGraphSchema::ForEachInputNode(std::function<bool(NodeGraphBase*)> cb) {
        mMainNodeGraph.ForEachInputNode(std::move(cb));
    }

    void NodeGraphSchema::ForEachOutputNode(std::function<bool(NodeGraphBase*)> cb) {
        mMainNodeGraph.ForEachOutputNode(std::move(cb));
    }

    void NodeGraphSchema::ForEachNodeType(std::function<void(std::string_view, const std::vector<UINodeDesc>&)> cb) const {
        for (auto& it : mRegisteredNodeTypes) {
            cb(it.first, it.second);
        }
    }

    TreeMenuNode& NodeGraphSchema::GetPickerRoot() {
        return mPickerRootMenu;
    }

    void NodeGraphSchema::RegisterNodeType(const std::string& typeGroup, int32_t uniqueTypeId, std::string_view typeName) {
        if (!HasNodeType(typeGroup, uniqueTypeId)) {
            mRegisteredNodeTypes[typeGroup].push_back(UINodeDesc{ uniqueTypeId, std::string(typeName) });
        }
        insertPath(mPickerRootMenu, typeGroup, typeName, uniqueTypeId);
    }

    void NodeGraphSchema::RegisterAllOf(const std::string& typeGroup) {
        if (typeGroup == "Numeric") {
            RegisterNodeType("Numeric", 50, "Add");
            RegisterNodeType("Numeric", 51, "Subtract");
            RegisterNodeType("Numeric", 52, "Negate");
            RegisterNodeType("Numeric", 53, "Multiply");
            RegisterNodeType("Numeric", 54, "Integral");
            RegisterNodeType("Numeric", 55, "Multiply3");
            RegisterNodeType("Numeric", 56, "Multiply & Add");
            RegisterNodeType("Numeric", 57, "Round");
            RegisterNodeType("Numeric", 58, "Derivate");
            RegisterNodeType("Numeric", 59, "Difference Normalized");
            RegisterNodeType("Numeric", 60, "Difference");
        }
        else if (typeGroup == "Logic") {
            RegisterNodeType("Logic", 100, "And");
            RegisterNodeType("Logic", 101, "Or");
            RegisterNodeType("Logic", 102, "Xor");
            RegisterNodeType("Logic", 103, "Detector");
            RegisterNodeType("Logic", 104, "Flip Gate");
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
        else if (typeGroup == "Signal.Control") {
            RegisterNodeType("Signal.Control", 400, "Envelope");
            RegisterNodeType("Signal.Control", 401, "Arpeggio");
        }
        else if (typeGroup == "Signal.Filter") {
            RegisterNodeType("Signal.Filter", 150, "Lowpass");
            RegisterNodeType("Signal.Filter", 151, "Highpass");
            RegisterNodeType("Signal.Filter", 152, "Chamberlin two-pole (4 mode)");
            RegisterNodeType("Signal.Filter", 153, "Moving Average");
        }
        else if (typeGroup == "Signal.Effect") {
            RegisterNodeType("Signal.Effect", 251, "Reverb1");
            RegisterNodeType("Signal.Effect", 252, "Reverb2");
            RegisterNodeType("Signal.Effect", 254, "Limiter");
            RegisterNodeType("Signal.Effect", 255, "Envelope Follower");
            RegisterNodeType("Signal.Effect", 256, "Saturator");
            RegisterNodeType("Signal.Effect", 257, "Trance Gate");
        }
        else if (typeGroup == "NodeGraph.Source") {
            RegisterNodeType("NodeGraph.Source", 0, "Value [0,1]");
            RegisterNodeType("NodeGraph.Source", 1, "Value [-1,1]");
            RegisterNodeType("NodeGraph.Source", 2, "Value [0,100]");
            RegisterNodeType("NodeGraph.Source", 3, "Value [-inf,inf]");
            RegisterNodeType("NodeGraph.Source", 4, "Time");
            RegisterNodeType("NodeGraph.Source", 5, "Text");
        }
        else if (typeGroup == "NodeGraph.Output") {
            RegisterNodeType("NodeGraph.Output", 200, "Debug");
            RegisterNodeType("NodeGraph.Output", 201, "Plot");
        }
        else if (typeGroup == "Device.Input") {
            RegisterNodeType("Device.Input", 300, "Keyboard Piano");
            RegisterNodeType("Device.Input", 301, "Midi Keyboard");
            RegisterNodeType("Device.Input", 302, "Midi Knobs");
            RegisterNodeType("Device.Input", 303, "Midi Button Group 1");
            RegisterNodeType("Device.Input", 304, "Midi Button Group 2");
            RegisterNodeType("Device.Input", 305, "Midi Button Group 3");
            RegisterNodeType("Device.Input", 306, "Midi Button Group 4");
            RegisterNodeType("Device.Input", 307, "Midi Button Group 5");
            RegisterNodeType("Device.Input", 308, "Mic");
        }
        else if (typeGroup == "Device.Output") {
            RegisterNodeType("Device.Output", 220, "PC Beep");
            RegisterNodeType("Device.Output", 221, "Speaker");
        }
        else if (typeGroup == "External.Input") {
            RegisterNodeType("External.Input", 500, "Data In x6");
            RegisterNodeType("External.Input", 501, "OCHLV Data In");
        }
        else if (typeGroup == "External.Output") {
            RegisterNodeType("External.Output", 550, "Data Out x6");
            RegisterNodeType("External.Output", 551, "Trade Signal");
        }
        else if (typeGroup == "Cache") {
            RegisterNodeType("Cache", 450, "Buffer Cache 1");
            RegisterNodeType("Cache", 451, "Buffer Cache 2");
            RegisterNodeType("Cache", 452, "Buffer Cache 3");
            RegisterNodeType("Cache", 453, "Buffer Cache 4");
        }
        else if (typeGroup == "UI") {
            RegisterNodeType("UI", 600, "UI Checkbox");
            RegisterNodeType("UI", 601, "UI Slider");
            RegisterNodeType("UI", 602, "UI Chart Lines");
            RegisterNodeType("UI", 603, "UI Candle Sticks");
        }
        else {
            LOG(LogWarning) << "Type group does not exist: " << typeGroup;
        }
    }

    void NodeGraphSchema::Tick(int32_t tickCount, float delta) {
        mMainNodeGraph.Tick(tickCount, delta);
    }

    void NodeGraphSchema::ProcessSubGraph(int32_t numSamples, int32_t numCacheSamples) {
        mMainNodeGraph.ProcessSubGraph(numSamples, numCacheSamples);
    }

}