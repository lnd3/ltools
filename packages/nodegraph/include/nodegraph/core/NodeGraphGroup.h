#pragma once

#include "logging/LoggingAll.h"

#include <string>
#include <vector>
#include <map>
#include <typeinfo>
#include <type_traits>
#include <memory>

#include "math/MathConstants.h"

#include "nodegraph/core/NodeGraphBase.h"

namespace l::nodegraph {

    class GraphDataCopy : public NodeGraphOp {
    public:
        GraphDataCopy(NodeGraphBase* node) :
            NodeGraphOp(node, 0)
        {}
        virtual ~GraphDataCopy() = default;

        void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
    };

    class NodeGraphGroup {
    public:
        NodeGraphGroup() :
            mInputNode(OutputType::Default),
            mOutputNode(OutputType::Default)
        {
            SetNumInputs(1);
            SetNumOutputs(1);
            mOutputNodes.push_back(&mOutputNode);
        }
        ~NodeGraphGroup() {
            LOG(LogInfo) << "Node group destroyed";
        }

        void SetNumInputs(int8_t numInputs);
        void SetNumOutputs(int8_t outputCount);
        void SetInput(int8_t inputChannel, NodeGraphBase& source, int8_t sourceOutputChannel);
        void SetInput(int8_t inputChannel, NodeGraphGroup& source, int8_t sourceOutputChannel);
        void SetInput(int8_t inputChannel, float constant);
        void SetInput(int8_t inputChannel, float* floatPtr);

        void SetOutput(int8_t outputChannel, NodeGraphBase& source, int8_t sourceOutputChannel);
        void SetOutput(int8_t outputChannel, NodeGraphGroup& source, int8_t sourceOutputChannel);

        float GetOutput(int8_t outputChannel);
        NodeGraphBase& GetInputNode();
        NodeGraphBase& GetOutputNode();

        bool ContainsNode(int32_t id);
        NodeGraphBase* GetNode(int32_t id);

        template<class T, class U = void, class = std::enable_if_t<std::is_base_of_v<NodeGraphOp, T>>>
        NodeGraph<T, U>* GetTypedNode(int32_t id) {
            auto p = GetNode(id);
            return reinterpret_cast<NodeGraph<T, U>*>(p);
        }

        bool RemoveNode(int32_t id);

        template<class T, class = std::enable_if_t<std::is_base_of_v<NodeGraphOp, T>>, class... Params>
        l::nodegraph::NodeGraphBase* NewNode(OutputType nodeType, Params&&... params) {
            mNodes.push_back(std::make_unique<l::nodegraph::NodeGraph<T, Params...>>(nodeType, std::forward<Params>(params)...));
            auto nodePtr = mNodes.back().get();
            if (nodeType == OutputType::ExternalOutput || nodeType == OutputType::ExternalVisualOutput) {
                mOutputNodes.push_back(nodePtr);
            }
            return nodePtr;
        }

        void ClearProcessFlags();
        void ProcessSubGraph(int32_t numSamples, bool recomputeSubGraphCache = true);
        void Tick(int32_t tickCount, float elapsed);
    protected:
        NodeGraph<GraphDataCopy> mInputNode;
        NodeGraph<GraphDataCopy> mOutputNode;

        std::vector<std::unique_ptr<NodeGraphBase>> mNodes;
        std::vector<NodeGraphBase*> mOutputNodes;

        int32_t mLastTickCount = 0;
    };

}

