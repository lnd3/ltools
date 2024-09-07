#pragma once

#include "logging/LoggingAll.h"

#include "nodegraph/NodeGraph.h"
#include "nodegraph/NodeGraphOperations.h"

#include "hid/Midi.h"

#include <string>
#include <vector>
#include <map>
#include <typeinfo>
#include <type_traits>
#include <memory>

namespace l::nodegraph {

    struct UINodeDesc {
        std::string_view GetName() const {
            return mName;
        }
        int32_t GetId() const {
            return mId;
        }

        int32_t mId;
        std::string mName;
    };

    class NodeGraphSchema {
    public:

        using CustomCreateFunctionType = NodeGraphBase*(int32_t, NodeGraphGroup&);

        NodeGraphSchema() {
            RegisterNodeType("Source", 0, "Value [0,1]");
            RegisterNodeType("Source", 1, "Value [-1,1]");
            RegisterNodeType("Source", 2, "Value [0,100]");
            RegisterNodeType("Source", 3, "Value [-inf,inf]");
            RegisterNodeType("Source", 4, "Time");
            RegisterNodeType("Source", 5, "Sine");
            RegisterNodeType("Source", 6, "Sine FM");
            RegisterNodeType("Source", 7, "Sine FM 2");
            RegisterNodeType("Source", 8, "Sine FM 3");
            RegisterNodeType("Numeric", 50, "Add");
            RegisterNodeType("Numeric", 51, "Subtract");
            RegisterNodeType("Numeric", 52, "Negate");
            RegisterNodeType("Numeric", 53, "Multiply");
            RegisterNodeType("Numeric", 54, "Integral");
            RegisterNodeType("Numeric", 55, "Multiply3");
            RegisterNodeType("Numeric", 56, "Multiply & Add");
            RegisterNodeType("Numeric", 57, "Round");
            RegisterNodeType("Logic", 100, "And");
            RegisterNodeType("Logic", 101, "Or");
            RegisterNodeType("Logic", 102, "Xor");
            RegisterNodeType("Filter", 150, "Lowpass");
            RegisterNodeType("Output", 200, "Debug");
            RegisterNodeType("Output", 201, "Speaker");
            RegisterNodeType("Output", 202, "Plot");
            RegisterNodeType("Effect", 250, "Envelope");
            RegisterNodeType("Effect", 251, "Reverb1");
            //RegisterNodeType("Effect", 252, "Reverb2");
            RegisterNodeType("Effect", 253, "Limiter");
            RegisterNodeType("Effect", 254, "Envelope Follower");
            RegisterNodeType("Effect", 255, "Saturator");
            RegisterNodeType("Effect", 256, "Trance Gate");
            RegisterNodeType("Input", 300, "Keyboard Piano");
            RegisterNodeType("Input", 301, "Midi Keyboard");
            RegisterNodeType("Input", 302, "Midi Knobs");
            RegisterNodeType("Input", 303, "Midi Button Group 1");
            RegisterNodeType("Input", 304, "Midi Button Group 2");
            RegisterNodeType("Input", 305, "Midi Button Group 3");
            RegisterNodeType("Input", 306, "Midi Button Group 4");
            RegisterNodeType("Input", 307, "Midi Button Group 5");
        }

        ~NodeGraphSchema() = default;

        void SetCustomCreator(std::function<CustomCreateFunctionType> customCreator);
        void SetKeyState(l::hid::KeyState* keyState);
        void SetAudioOutput(l::audio::AudioStream* audioStream);
        void SetMidiManager(l::hid::midi::MidiManager* midiManager);

        int32_t NewNode(int32_t typeId);
        bool RemoveNode(int32_t nodeId);
        NodeGraphBase* GetNode(int32_t nodeId);

        template<class T, class U = void, class = std::enable_if_t<std::is_base_of_v<NodeGraphOp, T>>>
        NodeGraph<T, U>* GetTypedNode(int32_t id) {
            return mMainNodeGraph.GetTypedNode<T, U>(id);
        }

        void ForEachNodeType(std::function<void(std::string_view, const std::vector<UINodeDesc>&)> cb) const;
        void RegisterNodeType(const std::string& typeGroup, int32_t uniqueTypeId, std::string_view typeName);
        void ProcessSubGraph(int32_t numSamples);
        void Tick(int32_t tickCount, float elapsed);
    protected:
        NodeGraphGroup mMainNodeGraph;

        std::function<CustomCreateFunctionType> mCreateCustomNode;
        l::hid::KeyState* mKeyState;
        l::audio::AudioStream* mAudioOutput;
        l::hid::midi::MidiManager* mMidiManager;

        std::map<std::string, std::vector<UINodeDesc>> mRegisteredNodeTypes;
    };

}

