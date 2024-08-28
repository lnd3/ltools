#include "nodegraph/NodeGraphSchema.h"

#include "logging/Log.h"

#include <set>

namespace l::nodegraph {

    void NodeGraphSchema::SetCustomCreator(std::function<CustomCreateFunctionType> customCreator) {
        mCreateCustomNode = customCreator;
    }

    int32_t NodeGraphSchema::NewNode(int32_t typeId) {
        l::nodegraph::NodeGraphBase* node = nullptr;
        switch (typeId) {
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
        case 50:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphLogicalAnd>();
            break;
        case 51:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphLogicalOr>();
            break;
        case 52:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphLogicalXor>();
            break;
        case 100:
            node = mMainNodeGraph.NewNode<l::nodegraph::GraphFilterLowpass>();
            break;
        default:
            ASSERT(typeId < 10000) << "Custom node id's begin at id 1000";
            ASSERT(mCreateCustomNode != nullptr) << "Custom nodes needs a handler to create them. It's missing.";
            node = mCreateCustomNode(typeId, mMainNodeGraph);
            break;
        };

        return node == nullptr ? 0 : node->GetId();
    }

    NodeGraphBase* NodeGraphSchema::GetNode(int32_t nodeId) {
        return mMainNodeGraph.GetNode(nodeId);
    }

    void NodeGraphSchema::ForEachNodeType(std::function<void(std::string_view, const std::vector<UINodeDesc>&)> cb) const {
        for (auto it : mRegisteredNodeTypes) {
            cb(it.first, it.second);
        }
    }

    void NodeGraphSchema::RegisterNodeType(const std::string& typeGroup, int32_t uniqueTypeId, std::string_view typeName) {
        UINodeDesc nodeDesc;
        nodeDesc.mId = uniqueTypeId;
        nodeDesc.mName = typeName;
        mRegisteredNodeTypes[typeGroup].push_back(UINodeDesc{ uniqueTypeId, std::string(typeName) });

    }

}