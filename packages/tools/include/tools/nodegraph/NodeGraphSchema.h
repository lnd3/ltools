#pragma once

#include "logging/LoggingAll.h"

#include "tools/nodegraph/NodeGraph.h"
#include "tools/nodegraph/NodeGraphOperations.h"

#include <string>
#include <vector>
#include <map>
#include <typeinfo>
#include <type_traits>
#include <memory>

namespace l::nodegraph {

    class NodeGraphSchema {
    public:
        NodeGraphSchema() = default;
        ~NodeGraphSchema() = default;

        void NewNode(int32_t type, std::function<void(NodeGraphBase*)> createHandler);
        NodeGraphBase* GetNode(int32_t id);

    protected:
        NodeGraphGroup mMainNodeGraph;

    };

}

