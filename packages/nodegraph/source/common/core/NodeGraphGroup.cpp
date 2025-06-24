#include "nodegraph/core/NodeGraphBase.h"
#include "nodegraph/core/NodeGraphGroup.h"

#include "logging/Log.h"

#include "math/MathFunc.h"

namespace l::nodegraph {

    void GraphDataCopy::Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        for (size_t i = 0; i < inputs.size() && i < outputs.size(); i++) {
            auto numInputSamples = inputs.at(i).GetSize();
            if (numSamples > 1) {
                ASSERT(numInputSamples == numSamples);
            }

            auto input = inputs.at(i).GetIterator(numInputSamples);
            auto output = outputs.at(i).GetIterator(numSamples);
            for (int32_t j = 0; j < numSamples; j++) {
                *output++ = *input++;
            }
        }
    }

    void NodeGraphGroup::Reset() {
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
    }

    bool NodeGraphGroup::LoadArchiveData(l::serialization::JsonValue& jsonValue) {
        if (jsonValue.has(JSMN_OBJECT) && jsonValue.has_key("Group")) {
            Reset();

            auto group = jsonValue.get("Group");
            if (group.has(JSMN_OBJECT)) {
                if (group.has_key("Nodes")) {
                    auto nodes = group.get("Nodes");
                    if (nodes.has(JSMN_ARRAY)) {
                        auto it = nodes.as_array();
                        for (; it.has_next();) {
                            auto e = it.next();
                            //LOG(LogInfo) << e.as_dbg_string();
                            if (e.has_key("TypeId") && e.has_key("NodeId")) {
                                auto typeId = e.get("TypeId").as_int32();
                                auto nodeId = e.get("NodeId").as_int32();

                                // If we load nodes the node id counter must be higher than the highest node id so we don't get duplicates later
                                if (mIds <= nodeId) {
                                    mIds = nodeId + 1;
                                }

                                //auto name = e.get("Name").as_string();
                                //auto typeName = e.get("TypeName").as_string();
                                mNodeFactory->NodeGraphNewNode(typeId, nodeId);

                                if (e.has_key("x") && e.has_key("y")) {
                                    GetNode(nodeId)->GetUIData().x = e.get("x").as_float();
                                    GetNode(nodeId)->GetUIData().y = e.get("y").as_float();
                                }
                                if (e.has_key("w") && e.has_key("w")) {
                                    GetNode(nodeId)->GetUIData().w = e.get("w").as_float();
                                    GetNode(nodeId)->GetUIData().h = e.get("h").as_float();
                                }
                            }
                        }
                    }
                }
                if (group.has_key("NodeData")) {
                    auto nodeData = group.get("NodeData");
                    if (nodeData.has(JSMN_ARRAY)) {
                        auto it = nodeData.as_array();
                        for (; it.has_next();) {
                            auto e = it.next();
                            //LOG(LogInfo) << e.as_dbg_string();
                            if (e.has_key("NodeId")) {
                                auto nodeId = e.get("NodeId").as_int32();
                                auto inputInfo = e.get("InputInfo");
                                //LOG(LogInfo) << inputInfo.as_dbg_string();

                                auto node = GetNode(nodeId);
                                ASSERT(node);
                                if (node) {
                                    if (inputInfo.has(JSMN_ARRAY)) {
                                        auto it2 = inputInfo.as_array();
                                        for (; it2.has_next();) {
                                            auto data = it2.next();
                                            if (data.has(JSMN_OBJECT)) {
                                                auto channel = data.get("Channel").as_int8();
                                                if (data.has_key("Value")) {
                                                    auto value = data.get("Value").as_float();
                                                    if (!node->SetInput(channel, value)) {
                                                        LOG(LogError) << "Failed to set channel constant data";
                                                    }
                                                }
                                                else if (data.has_key("Text")) {
                                                    auto text= data.get("Text").as_string();
                                                    if (!node->SetInput(channel, text)) {
                                                        LOG(LogError) << "Failed to set channel text data";
                                                    }
                                                }
                                                else if (data.has_key("SrcNodeId") && data.has_key("SrcChannel")) {
                                                    auto srcNodeId = data.get("SrcNodeId").as_int32();
                                                    auto srcChannel = data.get("SrcChannel").as_int8();
                                                    mNodeFactory->NodeGraphWireIO(srcNodeId, srcChannel, nodeId, channel);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        return true;
    }

    void NodeGraphGroup::GetArchiveData(l::serialization::JsonBuilder& jsonBuilder) {
        jsonBuilder.Begin("");
        jsonBuilder.Begin("Group");
        {
            jsonBuilder.Begin("Nodes", true);
            for (auto& it : mNodes) {
                jsonBuilder.Begin("");
                {
                    jsonBuilder.AddNumber("TypeId", it->GetTypeId());
                    jsonBuilder.AddNumber("NodeId", it->GetId());
                    jsonBuilder.AddString("Name", it->GetName());
                    jsonBuilder.AddString("TypeName", it->GetTypeName());
                    jsonBuilder.AddNumber("x", it->GetUIData().x);
                    jsonBuilder.AddNumber("y", it->GetUIData().y);
                    jsonBuilder.AddNumber("w", it->GetUIData().w);
                    jsonBuilder.AddNumber("h", it->GetUIData().h);
                }
                jsonBuilder.End();
            }
            jsonBuilder.End(true);

            jsonBuilder.Begin("NodeData", true);
            for (auto& it : mNodes) {
                jsonBuilder.Begin("");
                {
                    jsonBuilder.AddNumber("NodeId", it->GetId());
                    jsonBuilder.Begin("InputInfo", true);
                    {
                        for (int8_t i = 0; i < it->GetNumInputs(); i++) {
                            auto& input = it->GetInputOf(i);
                            if (input.IsOfType(InputType::INPUT_CONSTANT)) {
                                jsonBuilder.Begin("");
                                {
                                    jsonBuilder.AddNumber("Channel", i);
                                    jsonBuilder.AddNumber("Value", input.Get());
                                }
                                jsonBuilder.End();
                            } 
                            else if (input.IsOfType(InputType::INPUT_TEXT)) {
                                jsonBuilder.Begin("");
                                {
                                    jsonBuilder.AddNumber("Channel", i);
                                    jsonBuilder.AddString("Text", input.GetText());
                                }
                                jsonBuilder.End();
                            }
                            else if (input.IsOfType(InputType::INPUT_NODE)) {
                                ASSERT(input.HasInputNode());
                                auto srcNode = input.GetInputNode();
                                ASSERT(srcNode);
                                if (srcNode) {
                                    auto srcChannel = input.GetInputSrcChannel();
                                    jsonBuilder.Begin("");
                                    {
                                        jsonBuilder.AddNumber("Channel", i);
                                        jsonBuilder.AddNumber("SrcNodeId", srcNode->GetId());
                                        jsonBuilder.AddNumber("SrcChannel", srcChannel);
                                    }
                                    jsonBuilder.End();
                                }
                            }

                        }
                    }
                    jsonBuilder.End(true);
                }
                jsonBuilder.End();
            }
            jsonBuilder.End(true);
        }
        jsonBuilder.End();
        jsonBuilder.End();
    }

    void NodeGraphGroup::SetNodeFactory(NodeFactoryBase* factory) {
        mNodeFactory = factory;
    }

    void NodeGraphGroup::SetNumInputs(int8_t numInputs) {
        if (!mInputNode) {
            mInputNode = NewNode<GraphDataCopy>(0, NodeType::Default, numInputs);
        }
    }

    void NodeGraphGroup::SetNumOutputs(int8_t numOutput) {
        if (!mOutputNode) {
            mOutputNode = NewNode<GraphDataCopy>(0, NodeType::ExternalOutput, numOutput);
        }
    }

    void NodeGraphGroup::SetInput(int8_t inputChannel, NodeGraphBase& source, int8_t sourceOutputChannel) {
        mInputNode->SetInput(inputChannel, source, sourceOutputChannel);
    }

    void NodeGraphGroup::SetInput(int8_t inputChannel, NodeGraphGroup& source, int8_t sourceOutputChannel) {
        mInputNode->SetInput(inputChannel, source, sourceOutputChannel);
    }

    void NodeGraphGroup::SetInput(int8_t inputChannel, float constant) {
        mInputNode->SetInput(inputChannel, constant);
    }

    void NodeGraphGroup::SetInput(int8_t inputChannel, float* floatPtr) {
        mInputNode->SetInput(inputChannel, floatPtr);
    }

    void NodeGraphGroup::SetOutput(int8_t outputChannel, NodeGraphBase& source, int8_t sourceOutputChannel) {
        mOutputNode->SetInput(outputChannel, source, sourceOutputChannel);
        //mOutputNode->SetOutputName(outputChannel, source.GetOutputName(sourceOutputChannel));
    }

    void NodeGraphGroup::SetOutput(int8_t outputChannel, NodeGraphGroup& source, int8_t sourceOutputChannel) {
        mOutputNode->SetInput(outputChannel, source, sourceOutputChannel);
        //mOutputNode->SetOutputName(outputChannel, source.GetOutputNode().GetOutputName(sourceOutputChannel));
    }

    float& NodeGraphGroup::GetOutput(int8_t outputChannel, int32_t size) {
        return mOutputNode->GetOutput(outputChannel, size);
    }

    NodeGraphBase& NodeGraphGroup::GetInputNode() {
        return *mInputNode;
    }

    NodeGraphBase& NodeGraphGroup::GetOutputNode() {
        return *mOutputNode;
    }

    bool NodeGraphGroup::ContainsNode(int32_t id) {
        auto it = std::find_if(mNodes.begin(), mNodes.end(), [&](const NodeGraphBase* node) {
            if (node->GetId() == id) {
                return true;
            }
            return false;
            });
        if (it != mNodes.end()) {
            return true;
        }
        return false;
    }

    NodeGraphBase* NodeGraphGroup::GetNode(int32_t id) {
        auto it = std::find_if(mNodes.begin(), mNodes.end(), [&](const NodeGraphBase* node) {
            if (node->GetId() == id) {
                return true;
            }
            return false;
            });
        if (it != mNodes.end()) {
            return *it;
        }
        return nullptr;
    }

    bool NodeGraphGroup::RemoveNode(l::nodegraph::NodeGraphBase* node) {
        if (node) {
            auto id = node->GetId();
            return RemoveNode(id);
        }
        return false;
    }

    bool NodeGraphGroup::RemoveNode(int32_t id) {
        NodeGraphBase* node = nullptr;
        std::erase_if(mNodes, [&](NodeGraphBase* nodePtr) {
            if (nodePtr && nodePtr->GetId() == id) {
                node = nodePtr;
                return true;
            }
            return false;
            });

        if (!node) {
            return false;
        }
        for (auto& it : mNodes) {
            it->DetachInput(node);
        }
        std::erase_if(mInputNodes, [&](NodeGraphBase* nodePtr) {
            return nodePtr == node;
            });
        std::erase_if(mOutputNodes, [&](NodeGraphBase* nodePtr) {
            return nodePtr == node;
            });
        delete node;
        return true;
    }

    void NodeGraphGroup::ForEachNode(std::function<bool(NodeGraphBase*)> cb) {
        for (auto& it : mNodes) {
            if (!cb(it)) {
                break;
            }
        }
    }

    void NodeGraphGroup::ForEachInputNode(std::function<bool(NodeGraphBase*)> cb) {
        for (auto& it : mInputNodes) {
            if (!cb(it)) {
                break;
            }
        }
    }

    void NodeGraphGroup::ForEachOutputNode(std::function<bool(NodeGraphBase*)> cb) {
        for (auto& it : mOutputNodes) {
            if (!cb(it)) {
                break;
            }
        }
    }

    void NodeGraphGroup::ClearProcessFlags() {
        mOutputNode->ClearProcessFlags();
    }

    void NodeGraphGroup::ProcessSubGraph(int32_t numSamples, int32_t numCacheSamples) {
        for (auto& it : mOutputNodes) {
            it->ClearProcessFlags();
        }
        for (auto& it : mOutputNodes) {
            it->ProcessSubGraph(numSamples, numCacheSamples, false);
        }
    }

    void NodeGraphGroup::Tick(int32_t tickCount, float delta) {
        if (tickCount <= mLastTickCount) {
            return;
        }
        for (auto& it : mNodes) {
            it->Tick(tickCount, delta);
        }
        mLastTickCount = tickCount;
    }
}