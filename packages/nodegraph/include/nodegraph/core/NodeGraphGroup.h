#pragma once

#include "logging/LoggingAll.h"

#include <string>
#include <vector>
#include <map>
#include <typeinfo>
#include <type_traits>
#include <memory>

#include "math/MathConstants.h"
#include <serialization/JsonSerializationBase.h>
#include <serialization/JsonBuilder.h>
#include <serialization/JsonParser.h>

#include "nodegraph/core/NodeGraphBase.h"

namespace l::nodegraph {

    struct NodeIOMapping {
        int16_t mFromNodeIndex = -1;
        int16_t mToNodeIndex = -1;
        int8_t mFromOutputChannel = -1;
        int8_t mToInputChannel = -1;
    };

    struct NodeIOValues {
        int16_t mNodeIndex = -1;
        int8_t mChannel = -1;
        float mValue;
    };

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

        void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
    };

    class NodeGraphGroup : public l::serialization::JsonSerializationBase {
    public:
        NodeGraphGroup() {

        }
        ~NodeGraphGroup() {
            std::vector<int32_t> ids;
            for (auto& node : mNodes) {
                ids.push_back(node->GetId());
            }
            for (auto id : ids) {
                RemoveNode(id);
            }

            mInputNode = nullptr;
            mOutputNode = nullptr;
            mInputNodes.clear();
            mOutputNodes.clear();
            mNodes.clear();

            LOG(LogInfo) << "Node group destroyed";
        }

        NodeGraphGroup& operator=(NodeGraphGroup&& other) noexcept {
            mInputNode = other.mInputNode;
            mOutputNode = other.mOutputNode;

            mNodes = other.mNodes;
            mOutputNodes = other.mOutputNodes;
            mInputNodes = other.mInputNodes;

            mLastTickCount = other.mLastTickCount;
            mIds = other.mIds;
            return *this;
        }
        NodeGraphGroup(NodeGraphGroup&& other) noexcept {
            *this = std::move(other);
        }

        NodeGraphGroup(const NodeGraphGroup&) = delete;
        NodeGraphGroup& operator=(const NodeGraphGroup&) = delete;

        virtual bool LoadArchiveData(std::stringstream& src) override {
            l::serialization::JsonParser<1000> parser;
            auto stream = src.str();
            auto [result, error] = parser.LoadJson(stream.c_str(), stream.size());
            if (result) {
                auto root = parser.GetRoot();
                LOG(LogInfo) << "NG load: \n" << root.as_dbg_string();
                return true;
            }
            return false;
        }

        virtual void GetArchiveData(std::stringstream& dst) override {
            l::serialization::JsonBuilder builder(true);
            builder.SetStream(&dst);
            builder.Begin("");
            {
                builder.Begin("NodeGraphGroup");
                {
                    builder.Begin("Nodes", true);
                    {
                        for (auto& it : mNodes) {
                            builder.Begin("");
                            {
                                builder.AddNumber("TypeId", it->GetTypeId());
                                builder.AddString("TypeName", it->GetTypeName());
                            }
                            builder.End();
                        }
                    }
                    builder.End("Nodes");
                }
                builder.End("NodeGraphGroup");
            }
            builder.End();
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

        bool RemoveNode(l::nodegraph::NodeGraphBase* node);
        bool RemoveNode(int32_t id);

        template<class T, std::enable_if_t<std::is_base_of_v<NodeGraphOp, T>, int> = 0, class... Params>
        l::nodegraph::NodeGraphBase* NewNode(NodeType nodeType, Params&&... params) {
            l::nodegraph::NodeGraphBase* nodePtr = new l::nodegraph::NodeGraph<T, Params...>(mIds++, nodeType, std::forward<Params>(params)...);
            mNodes.push_back(nodePtr);
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
        void ProcessSubGraph(int32_t numSamples, int32_t numCacheSamples = -1);
        void Tick(int32_t tickCount, float elapsed);
    protected:
        NodeGraphBase* mInputNode = nullptr;
        NodeGraphBase* mOutputNode = nullptr;

        std::vector<NodeGraphBase*> mNodes;
        std::vector<NodeGraphBase*> mOutputNodes;
		std::vector<NodeGraphBase*> mInputNodes;

        int32_t mLastTickCount = 0;
        int32_t mIds = 1;
    };

}

