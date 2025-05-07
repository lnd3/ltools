#pragma once

#include "logging/LoggingAll.h"

#include <string>
#include <vector>
#include <map>
#include <typeinfo>
#include <type_traits>
#include <memory>

#include "math/MathConstants.h"
#include "serialization/SerializationBase.h"

#include "nodegraph/core/NodeGraphBase.h"

namespace l::nodegraph {

    class GraphDataCopy : public NodeGraphOp {
    public:
        GraphDataCopy(NodeGraphBase* node, int32_t numInputsOutputs) :
            NodeGraphOp(node, "Copy")
        {
            for (int32_t i = 0; i < numInputsOutputs; i++) {
                AddInput("In " + std::to_string(i), 0.0f, 1);
            }
            for (int32_t i = 0; i < numInputsOutputs; i++) {
                AddOutput("Out " + std::to_string(i), 0.0f, 1);
            }
        }
        virtual ~GraphDataCopy() = default;

        void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
    };

    class NodeGraphGroup : public l::serialization::SerializationBase {
    public:
        NodeGraphGroup() : SerializationBase(0, 0, true, false, true, true, true) {}
        ~NodeGraphGroup() {
            mInputNode = nullptr;
            mOutputNode = nullptr;
            mInputNodes.clear();
            mOutputNodes.clear();
            for(auto& node : mNodes) {
                if (node) {
                    node.get()->ClearInputs();
                }
            }
            mNodes.clear();

            LOG(LogInfo) << "Node group destroyed";
        }

        NodeGraphGroup& operator=(NodeGraphGroup&&) noexcept {
            return *this;
        }
        NodeGraphGroup& operator=(const NodeGraphGroup&) noexcept {
            return *this;
        }
        NodeGraphGroup(NodeGraphGroup&& other) noexcept {
            *this = std::move(other);
        }
        NodeGraphGroup(const NodeGraphGroup& other) noexcept : SerializationBase(other) {
            *this = other;
        }

        void SetNumInputs(int8_t numInputs);
        void SetNumOutputs(int8_t outputCount);
        void SetInput(int8_t inputChannel, NodeGraphBase& source, int8_t sourceOutputChannel);
        void SetInput(int8_t inputChannel, NodeGraphGroup& source, int8_t sourceOutputChannel);
        void SetInput(int8_t inputChannel, float constant);
        void SetInput(int8_t inputChannel, float* floatPtr);

        void SetOutput(int8_t outputChannel, NodeGraphBase& source, int8_t sourceOutputChannel);
        void SetOutput(int8_t outputChannel, NodeGraphGroup& source, int8_t sourceOutputChannel);

        float& GetOutput(int8_t outputChannel, int32_t size = 1);

        NodeGraphBase& GetInputNode();
        NodeGraphBase& GetOutputNode();

        bool ContainsNode(int32_t id);
        NodeGraphBase* GetNode(int32_t id);

        bool RemoveNode(int32_t id);

        template<class T, std::enable_if_t<std::is_base_of_v<NodeGraphOp, T>, int> = 0, class... Params>
        l::nodegraph::NodeGraphBase* NewNode(NodeType nodeType, Params&&... params) {
            mNodes.emplace_back(std::make_unique<l::nodegraph::NodeGraph<T, Params...>>(nodeType, std::forward<Params>(params)...));
            auto nodePtr = mNodes.back().get();
            if (nodeType == NodeType::ExternalOutput || nodeType == NodeType::ExternalVisualOutput) {
                mOutputNodes.push_back(nodePtr);
			}
			else if (nodeType == NodeType::ExternalInput) {
                mInputNodes.push_back(nodePtr);
            }

            return nodePtr;
        }

        //l::nodegraph::NodeGraphGroup* NewGroup() {
        //    mNodes.emplace_back(std::make_unique<l::nodegraph::NodeGraphGroup>());
        //    auto groupPtr = dynamic_cast<l::nodegraph::NodeGraphGroup*>(mNodes.back().get());
        //    return groupPtr;
        //}

        void ForEachInputNode(std::function<bool(NodeGraphBase*)> cb);
        void ForEachOutputNode(std::function<bool(NodeGraphBase*)> cb);

        void ClearProcessFlags();
        void ProcessSubGraph(int32_t numSamples);
        void Tick(int32_t tickCount, float elapsed);
    protected:
        NodeGraphBase* mInputNode = nullptr;
        NodeGraphBase* mOutputNode = nullptr;

        std::vector<std::unique_ptr<NodeGraphBase>> mNodes;
        std::vector<NodeGraphBase*> mOutputNodes;
		std::vector<NodeGraphBase*> mInputNodes;

        int32_t mLastTickCount = 0;
    };

}

