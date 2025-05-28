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

            // Internal output like NG chart or debug view
        case 20:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphOutputDebug>(NodeType::ExternalOutput);
            break;
        case 21:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphOutputPlot>(NodeType::ExternalVisualOutput, 100);
            break;

            // Internal cache (basically bulk data input/output from external and to external)
        case 40:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphCache>(NodeType::Default, 1);
            break;
        case 41:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphCache>(NodeType::Default, 2);
            break;
        case 42:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphCache>(NodeType::Default, 3);
            break;
        case 43:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphCache>(NodeType::Default, 4);
            break;



            // Math aritmethic operators
        case 100:
            node = mMainNodeGraph.NewNode<l::nodegraph::MathAritmethicAdd>(NodeType::Default);
            break;
        case 101:
            node = mMainNodeGraph.NewNode<l::nodegraph::MathAritmethicSubtract>(NodeType::Default);
            break;
        case 102:
            node = mMainNodeGraph.NewNode<l::nodegraph::MathAritmethicMultiply>(NodeType::Default);
            break;
        case 103:
            node = mMainNodeGraph.NewNode<l::nodegraph::MathAritmethicNegate>(NodeType::Default);
            break;
        case 104:
            node = mMainNodeGraph.NewNode<l::nodegraph::MathAritmethicAbs>(NodeType::Default);
            break;
        case 105:
            node = mMainNodeGraph.NewNode<l::nodegraph::MathAritmethicLog>(NodeType::Default);
            break;
        case 106:
            node = mMainNodeGraph.NewNode<l::nodegraph::MathAritmethicMultiply3>(NodeType::Default);
            break;
        case 107:
            node = mMainNodeGraph.NewNode<l::nodegraph::MathAritmethicMultiplyAndAdd>(NodeType::Default);
            break;
        case 108:
            node = mMainNodeGraph.NewNode<l::nodegraph::MathAritmethicRound>(NodeType::Default);
            break;

            // Math logical operators
        case 121:
            node = mMainNodeGraph.NewNode<l::nodegraph::MathLogicalAnd>(NodeType::Default);
            break;
        case 122:
            node = mMainNodeGraph.NewNode<l::nodegraph::MathLogicalOr>(NodeType::Default);
            break;
        case 123:
            node = mMainNodeGraph.NewNode<l::nodegraph::MathLogicalXor>(NodeType::Default);
            break;

            // Math numerical
        case 140:
            node = mMainNodeGraph.NewNode<l::nodegraph::MathNumericalIntegral>(NodeType::Default);
            break;
        case 141:
            node = mMainNodeGraph.NewNode<l::nodegraph::MathNumericalDerivate>(NodeType::Default);
            break;
        case 142:
            node = mMainNodeGraph.NewNode<l::nodegraph::MathNumericalDiffNorm>(NodeType::Default);
            break;
        case 143:
            node = mMainNodeGraph.NewNode<l::nodegraph::MathNumericalDiff>(NodeType::Default);
            break;

            // Trading data io
        case 200:
            node = mMainNodeGraph.NewNode<l::nodegraph::DataIOOCHLVDataIn>(NodeType::ExternalInput);
            break;
        case 201:
            node = mMainNodeGraph.NewNode<l::nodegraph::DataIOPlaceTrade>(NodeType::ExternalOutput);
            break;

            // Trading detectors
        case 220:
            node = mMainNodeGraph.NewNode<l::nodegraph::TradingDetectorTrend>(NodeType::Default);
            break;
        case 221:
            node = mMainNodeGraph.NewNode<l::nodegraph::TradingDetectorTrendDiff>(NodeType::Default);
            break;

            // Trading filter
        case 240:
            node = mMainNodeGraph.NewNode<l::nodegraph::TradingFilterFlipGate>(NodeType::Default);
            break;
        case 241:
            node = mMainNodeGraph.NewNode<l::nodegraph::TradingFilterPulseInfo>(NodeType::Default);
            break;
        case 242:
            node = mMainNodeGraph.NewNode<l::nodegraph::TradingFilterVWMA>(NodeType::Default);
            break;

            // Trading indicators
        case 260:
            node = mMainNodeGraph.NewNode<l::nodegraph::TradingIndicatorOBV>(NodeType::Default);
            break;
        case 261:
            node = mMainNodeGraph.NewNode<l::nodegraph::TradingIndicatorVRSI>(NodeType::Default);
            break;
        case 262:
            node = mMainNodeGraph.NewNode<l::nodegraph::TradingIndicatorVPT>(NodeType::Default);
            break;
        case 263:
            node = mMainNodeGraph.NewNode<l::nodegraph::TradingIndicatorVPIO>(NodeType::Default);
            break;

            // Signal generators (basically audio composition)
        case 300:
            node = mMainNodeGraph.NewNode<l::nodegraph::SignalGeneratorSine>(NodeType::Default);
            break;
        case 301:
            node = mMainNodeGraph.NewNode<l::nodegraph::SignalGeneratorSineFM>(NodeType::Default);
            break;
        case 302:
            node = mMainNodeGraph.NewNode<l::nodegraph::SignalGeneratorSineFM2>(NodeType::Default);
            break;
        case 303:
            node = mMainNodeGraph.NewNode<l::nodegraph::SignalGeneratorSineFM3>(NodeType::Default);
            break;
        case 304:
            node = mMainNodeGraph.NewNode<l::nodegraph::SignalGeneratorSaw>(NodeType::Default);
            break;
        case 305:
            node = mMainNodeGraph.NewNode<l::nodegraph::SignalGeneratorSine2>(NodeType::Default);
            break;
        case 306:
            node = mMainNodeGraph.NewNode<l::nodegraph::SignalGeneratorSaw2>(NodeType::Default);
            break;

            // Signal controllers (basically audio shaping)
        case 320:
            node = mMainNodeGraph.NewNode<l::nodegraph::SignalControlEnvelope>(NodeType::Default);
            break;
        case 321:
            node = mMainNodeGraph.NewNode<l::nodegraph::SignalControlArpeggio>(NodeType::Default);
            break;

            // Signal filter
        case 340:
            node = mMainNodeGraph.NewNode<l::nodegraph::SignalFilterLowpass>(NodeType::Default);
            break;
        case 341:
            node = mMainNodeGraph.NewNode<l::nodegraph::SignalFilterHighpass>(NodeType::Default);
            break;
        case 342:
            node = mMainNodeGraph.NewNode<l::nodegraph::SignalFilterChamberlain2pole>(NodeType::Default);
            break;
        case 343:
            node = mMainNodeGraph.NewNode<l::nodegraph::SignalFilterMovingAverage>(NodeType::Default);
            break;

            // Signal effects
        case 360:
            node = mMainNodeGraph.NewNode<l::nodegraph::SignalEffectReverb1>(NodeType::Default);
            break;
        case 361:
            node = mMainNodeGraph.NewNode<l::nodegraph::SignalEffectReverb2>(NodeType::Default);
            break;
        case 362:
            node = mMainNodeGraph.NewNode<l::nodegraph::SignalEffectLimiter>(NodeType::Default);
            break;
        case 363:
            node = mMainNodeGraph.NewNode<l::nodegraph::SignalEffectEnvelopeFollower>(NodeType::Default);
            break;
        case 364:
            node = mMainNodeGraph.NewNode<l::nodegraph::SignalEffectSaturator>(NodeType::Default);
            break;
        case 365:
            node = mMainNodeGraph.NewNode<l::nodegraph::SignalEffectTranceGate>(NodeType::Default);
            break;


            // DeviceIO (midi, keyboard piano)
        case 400:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphInputKeyboardPiano>(NodeType::Default, mKeyState);
            break;
        case 401:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphInputMidiKeyboard>(NodeType::Default, mMidiManager);
            break;
        case 402:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphInputMidiKnobs>(NodeType::Default, mMidiManager);
            break;
        case 403:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphInputMidiButtons>(NodeType::Default, mMidiManager, 0);
            break;
        case 404:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphInputMidiButtons>(NodeType::Default, mMidiManager, 1);
            break;
        case 405:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphInputMidiButtons>(NodeType::Default, mMidiManager, 2);
            break;
        case 406:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphInputMidiButtons>(NodeType::Default, mMidiManager, 3);
            break;
        case 407:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphInputMidiButtons>(NodeType::Default, mMidiManager, 4);
            break;
        case 408:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphInputMic>(NodeType::Default, mAudioOutput);
            break;


            // DeviceIO (like speakers or external data sinks)
        case 420:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphOutputPCBeep>(NodeType::ExternalOutput);
            break;
        case 421:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphOutputSpeaker>(NodeType::ExternalOutput, mAudioOutput);
            break;

            // DataIO input
        case 500:
            node = mMainNodeGraph.NewNode<l::nodegraph::DataIODataIn>(NodeType::ExternalInput, 6);
            break;
        case 501:
            node = mMainNodeGraph.NewNode<l::nodegraph::DataIODataOut>(NodeType::ExternalOutput, 6);
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
        if (typeGroup == "Node Graph.Source") {
            RegisterNodeType("Node Graph.Source", 0, "Value [0,1]");
            RegisterNodeType("Node Graph.Source", 1, "Value [-1,1]");
            RegisterNodeType("Node Graph.Source", 2, "Value [0,100]");
            RegisterNodeType("Node Graph.Source", 3, "Value [-inf,inf]");
            RegisterNodeType("Node Graph.Source", 4, "Time");
            RegisterNodeType("Node Graph.Source", 5, "Text");
        }
        else if (typeGroup == "Node Graph.Output") {
            RegisterNodeType("Node Graph.Output", 20, "Debug");
            RegisterNodeType("Node Graph.Output", 21, "Plot");
        }
        else if (typeGroup == "Node Graph.Cache") {
            RegisterNodeType("Node Graph.Cache", 40, "Cache x1");
            RegisterNodeType("Node Graph.Cache", 41, "Cache x2");
            RegisterNodeType("Node Graph.Cache", 42, "Cache x3");
            RegisterNodeType("Node Graph.Cache", 43, "Cache x4");
        }
        else if (typeGroup == "Math.Aritmethic") {
            RegisterNodeType("Math.Aritmethic", 100, "Add");
            RegisterNodeType("Math.Aritmethic", 101, "Sub");
            RegisterNodeType("Math.Aritmethic", 102, "Mul");
            RegisterNodeType("Math.Aritmethic", 103, "Neg");
            RegisterNodeType("Math.Aritmethic", 104, "Abs");
            RegisterNodeType("Math.Aritmethic", 105, "Log");
            RegisterNodeType("Math.Aritmethic", 106, "Mul3");
            RegisterNodeType("Math.Aritmethic", 107, "Madd");
            RegisterNodeType("Math.Aritmethic", 108, "Round");
        }
        else if (typeGroup == "Math.Logic") {
            RegisterNodeType("Math.Logic", 120, "And");
            RegisterNodeType("Math.Logic", 121, "Or");
            RegisterNodeType("Math.Logic", 122, "Xor");
        }
        else if (typeGroup == "Math.Numerical") {
            RegisterNodeType("Math.Numerical", 140, "Integral");
            RegisterNodeType("Math.Numerical", 141, "Derivate");
            RegisterNodeType("Math.Numerical", 142, "Difference Normalized");
            RegisterNodeType("Math.Numerical", 143, "Difference");
        }
        else if (typeGroup == "Trading.Data IO") {
            RegisterNodeType("Trading.Data IO", 200, "OCHLV Data In");
            RegisterNodeType("Trading.Data IO", 201, "Place Trade");
        }
        else if (typeGroup == "Trading.Detector") {
            RegisterNodeType("Trading.Detector", 220, "Trend");
            RegisterNodeType("Trading.Detector", 221, "Trend Diff");
        }
        else if (typeGroup == "Trading.Filter") {
            RegisterNodeType("Trading.Filter", 240, "Flip Gate");
            RegisterNodeType("Trading.Filter", 241, "Pulse Info");
            RegisterNodeType("Trading.Filter", 242, "Volume Weighted Moving Average");
        }
        else if (typeGroup == "Trading.Indicator") {
            RegisterNodeType("Trading.Indicator", 260, "OBV");
            RegisterNodeType("Trading.Indicator", 261, "VRSI");
            RegisterNodeType("Trading.Indicator", 262, "VPT");
            RegisterNodeType("Trading.Indicator", 263, "VPIO");
        }
        else if (typeGroup == "Signal.Generator") {
            RegisterNodeType("Signal.Generator", 300, "Sine");
            RegisterNodeType("Signal.Generator", 301, "Sine FM 1");
            RegisterNodeType("Signal.Generator", 302, "Sine FM 2");
            RegisterNodeType("Signal.Generator", 303, "Sine FM 3");
            RegisterNodeType("Signal.Generator", 304, "Saw");
            RegisterNodeType("Signal.Generator", 305, "Sine 2");
            RegisterNodeType("Signal.Generator", 306, "Saw 2");
        }
        else if (typeGroup == "Signal.Control") {
            RegisterNodeType("Signal.Control", 320, "Envelope");
            RegisterNodeType("Signal.Control", 321, "Arpeggio");
        }
        else if (typeGroup == "Signal.Filter") {
            RegisterNodeType("Signal.Filter", 340, "Lowpass");
            RegisterNodeType("Signal.Filter", 341, "Highpass");
            RegisterNodeType("Signal.Filter", 342, "Chamberlin two-pole (4 mode)");
            RegisterNodeType("Signal.Filter", 343, "Moving Average");
        }
        else if (typeGroup == "Signal.Effect") {
            RegisterNodeType("Signal.Effect", 360, "Reverb1");
            RegisterNodeType("Signal.Effect", 361, "Reverb2");
            RegisterNodeType("Signal.Effect", 362, "Limiter");
            RegisterNodeType("Signal.Effect", 363, "Envelope Follower");
            RegisterNodeType("Signal.Effect", 364, "Saturator");
            RegisterNodeType("Signal.Effect", 365, "Trance Gate");
        }
        else if (typeGroup == "Device IO.Input") {
            RegisterNodeType("Device IO.Input", 400, "Keyboard Piano");
            RegisterNodeType("Device IO.Input", 401, "Midi Keyboard");
            RegisterNodeType("Device IO.Input", 402, "Midi Knobs");
            RegisterNodeType("Device IO.Input", 403, "Midi Button Group 1");
            RegisterNodeType("Device IO.Input", 404, "Midi Button Group 2");
            RegisterNodeType("Device IO.Input", 405, "Midi Button Group 3");
            RegisterNodeType("Device IO.Input", 406, "Midi Button Group 4");
            RegisterNodeType("Device IO.Input", 407, "Midi Button Group 5");
            RegisterNodeType("Device IO.Input", 408, "Mic");
        }
        else if (typeGroup == "Device IO.Output") {
            RegisterNodeType("Device IO.Output", 420, "PC Beep");
            RegisterNodeType("Device IO.Output", 421, "Speaker");
        }
        else if (typeGroup == "Data IO") {
            RegisterNodeType("Data IO", 500, "Data In x6");
            RegisterNodeType("Data IO", 501, "Data Out x6");
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

    void NodeGraphSchema::RegisterAllOperators() {
        RegisterAllOf("Node Graph.Source");
        RegisterAllOf("Node Graph.Output");
        RegisterAllOf("Node Graph.Cache");

        RegisterAllOf("Math.Aritmethic");
        RegisterAllOf("Math.Logic");
        RegisterAllOf("Math.Numerical");

        RegisterAllOf("Trading.Data IO");
        RegisterAllOf("Trading.Detector");
        RegisterAllOf("Trading.Filter");
        RegisterAllOf("Trading.Indicator");

        RegisterAllOf("Signal.Generator");
        RegisterAllOf("Signal.Control");
        RegisterAllOf("Signal.Filter");
        RegisterAllOf("Signal.Effect");

        RegisterAllOf("Device IO.Input");
        RegisterAllOf("Device IO.Output");

        RegisterAllOf("Data IO");

        RegisterAllOf("UI");
    }

    void NodeGraphSchema::Tick(int32_t tickCount, float delta) {
        mMainNodeGraph.Tick(tickCount, delta);
    }

    void NodeGraphSchema::ProcessSubGraph(int32_t numSamples, int32_t numCacheSamples) {
        mMainNodeGraph.ProcessSubGraph(numSamples, numCacheSamples);
    }

}