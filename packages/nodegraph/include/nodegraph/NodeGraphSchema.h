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
            RegisterNodeType("Numeric", 0, "Add");
            RegisterNodeType("Numeric", 1, "Subtract");
            RegisterNodeType("Numeric", 2, "Negate");
            RegisterNodeType("Numeric", 3, "Multiply");
            RegisterNodeType("Numeric", 4, "Integral");
            RegisterNodeType("Logic", 50, "And");
            RegisterNodeType("Logic", 51, "Or");
            RegisterNodeType("Logic", 52, "Xor");
            RegisterNodeType("Filter", 100, "Lowpass Filter");
        }

        ~NodeGraphSchema() = default;

        void SetCustomCreator(std::function<CustomCreateFunctionType> customCreator);
        int32_t NewNode(int32_t typeId);
        NodeGraphBase* GetNode(int32_t nodeId);
        void ForEachNodeType(std::function<void(std::string_view, const std::vector<UINodeDesc>&)> cb) const;
        void RegisterNodeType(const std::string& typeGroup, int32_t uniqueTypeId, std::string_view typeName);
    protected:
        NodeGraphGroup mMainNodeGraph;

        std::function<CustomCreateFunctionType> mCreateCustomNode;

        std::map<std::string, std::vector<UINodeDesc>> mRegisteredNodeTypes;
    };

}

