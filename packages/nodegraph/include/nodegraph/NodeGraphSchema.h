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
            RegisterNodeType("Source", 0, "Constant1");
            RegisterNodeType("Source", 1, "Constant2");
            RegisterNodeType("Source", 2, "Constant3");
            RegisterNodeType("Source", 3, "Constant4");
            RegisterNodeType("Source", 4, "Sine");
            RegisterNodeType("Numeric", 50, "Add");
            RegisterNodeType("Numeric", 51, "Subtract");
            RegisterNodeType("Numeric", 52, "Negate");
            RegisterNodeType("Numeric", 53, "Multiply");
            RegisterNodeType("Numeric", 54, "Integral");
            RegisterNodeType("Logic", 100, "And");
            RegisterNodeType("Logic", 101, "Or");
            RegisterNodeType("Logic", 102, "Xor");
            RegisterNodeType("Filter", 150, "Lowpass Filter");
        }

        ~NodeGraphSchema() = default;

        void SetCustomCreator(std::function<CustomCreateFunctionType> customCreator);
        int32_t NewNode(int32_t typeId);
        bool RemoveNode(int32_t nodeId);
        NodeGraphBase* GetNode(int32_t nodeId);
        void ForEachNodeType(std::function<void(std::string_view, const std::vector<UINodeDesc>&)> cb) const;
        void RegisterNodeType(const std::string& typeGroup, int32_t uniqueTypeId, std::string_view typeName);
        void Update();
    protected:
        NodeGraphGroup mMainNodeGraph;

        std::function<CustomCreateFunctionType> mCreateCustomNode;

        std::map<std::string, std::vector<UINodeDesc>> mRegisteredNodeTypes;
    };

}

