#include "tools/nodegraph/NodeGraphSchema.h"

#include "logging/Log.h"

namespace l::nodegraph {

    void NodeGraphSchema::NewNode(int32_t type, std::function<void(NodeGraphBase*)> createHandler) {
        l::nodegraph::NodeGraphBase* node = nullptr;
        switch (type) {
        case 0:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphNumericAdd>();
            break;
        case 1:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphNumericSubtract>();
            break;
        case 2:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphNumericNegate>();
            break;
        case 3:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphNumericMultiply>();
            break;
        case 4:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphNumericIntegral>();
            break;
        case 20:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphLogicalAnd>();
            break;
        case 21:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphLogicalOr>();
            break;
        case 22:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphLogicalXor>();
            break;
        case 40:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphFilterLowpass>();
            break;
        default:
            break;
        };
        createHandler(node);
    }

    NodeGraphBase* NodeGraphSchema::GetNode(int32_t id) {
        return mMainNodeGraph.GetNode(id);
    }

}