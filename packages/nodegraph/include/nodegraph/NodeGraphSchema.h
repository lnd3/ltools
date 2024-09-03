#pragma once

#include "logging/LoggingAll.h"

#include "nodegraph/NodeGraph.h"
#include "nodegraph/NodeGraphOperations.h"

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
            RegisterNodeType("Source", 4, "Keyboard");
            RegisterNodeType("Source", 5, "Time");
            RegisterNodeType("Source", 6, "Sine");
            RegisterNodeType("Numeric", 50, "Add");
            RegisterNodeType("Numeric", 51, "Subtract");
            RegisterNodeType("Numeric", 52, "Negate");
            RegisterNodeType("Numeric", 53, "Multiply");
            RegisterNodeType("Numeric", 54, "Integral");
            RegisterNodeType("Logic", 100, "And");
            RegisterNodeType("Logic", 101, "Or");
            RegisterNodeType("Logic", 102, "Xor");
            RegisterNodeType("Filter", 150, "Lowpass Filter");
            RegisterNodeType("Filter", 151, "Envelope");
            RegisterNodeType("Output", 200, "Value Debug");
            RegisterNodeType("Output", 201, "Speaker");
            RegisterNodeType("Effect", 250, "Reverb1");
            RegisterNodeType("Effect", 251, "Reverb2");
        }

        ~NodeGraphSchema() = default;

        void SetCustomCreator(std::function<CustomCreateFunctionType> customCreator);
        void SetKeyState(l::hid::KeyState* keyState);
        void SetAudioOutput(l::audio::AudioStream* audioStream);

        int32_t NewNode(int32_t typeId);
        bool RemoveNode(int32_t nodeId);
        NodeGraphBase* GetNode(int32_t nodeId);

        template<class T, class U = void, class = std::enable_if_t<std::is_base_of_v<NodeGraphOp, T>>>
        NodeGraph<T, U>* GetTypedNode(int32_t id) {
            return mMainNodeGraph.GetTypedNode<T, U>(id);
        }

        void ForEachNodeType(std::function<void(std::string_view, const std::vector<UINodeDesc>&)> cb) const;
        void RegisterNodeType(const std::string& typeGroup, int32_t uniqueTypeId, std::string_view typeName);
        void ProcessSubGraph();
        void Tick(float time, float elapsed);
    protected:
        NodeGraphGroup mMainNodeGraph;

        std::function<CustomCreateFunctionType> mCreateCustomNode;
        l::hid::KeyState* mKeyState;
        l::audio::AudioStream* mAudioOutput;

        std::map<std::string, std::vector<UINodeDesc>> mRegisteredNodeTypes;
    };

}

