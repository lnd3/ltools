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

    class NodeGraphSchema {
    public:

        using CustomCreateFunctionType = NodeGraphBase * (int32_t type, NodeGraphGroup&);

        NodeGraphSchema() = default;
        ~NodeGraphSchema() = default;

        void SetCustomCreator(std::function<CustomCreateFunctionType> customCreator);
        int32_t NewNode(int32_t type);
        NodeGraphBase* GetNode(int32_t id);

    protected:
        NodeGraphGroup mMainNodeGraph;

        std::function<CustomCreateFunctionType> mCreateCustomNode;

    };

}

