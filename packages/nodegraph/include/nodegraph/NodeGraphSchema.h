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

        using CustomCreateFunctionType = NodeGraphBase * (int32_t type, NodeGraphGroup&);

        NodeGraphSchema() {
            mRegisteredNodeTypes.emplace("Numerical", std::vector<UINodeDesc>{
                UINodeDesc{ 0, "Add"},
                UINodeDesc{ 1, "Subtract" },
                UINodeDesc{ 2, "Negate" },
                UINodeDesc{ 3, "Multiply}" },
                UINodeDesc{ 4, "Integral" }
                });
            mRegisteredNodeTypes.emplace("Logical", std::vector<UINodeDesc>{
                UINodeDesc{ 20, "And" },
                UINodeDesc{ 21, "Or" },
                UINodeDesc{ 22, "Xor" }
                });
            mRegisteredNodeTypes.emplace("", std::vector<UINodeDesc>{
                UINodeDesc{ 40, "Lowpass Filter" }
                });
        }

        ~NodeGraphSchema() = default;

        void SetCustomCreator(std::function<CustomCreateFunctionType> customCreator);
        int32_t NewNode(int32_t type);
        NodeGraphBase* GetNode(int32_t id);
        void ForEachNodeType(std::function<void(std::string_view, const std::vector<UINodeDesc>&)> cb) const;

    protected:
        NodeGraphGroup mMainNodeGraph;

        std::function<CustomCreateFunctionType> mCreateCustomNode;

        std::map<std::string, std::vector<UINodeDesc>> mRegisteredNodeTypes;
    };

}

