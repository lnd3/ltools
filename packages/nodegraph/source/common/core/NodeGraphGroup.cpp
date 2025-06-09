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

    void NodeGraphGroup::SetNumInputs(int8_t numInputs) {
        if (!mInputNode) {
            mInputNode = NewNode<GraphDataCopy>(NodeType::Default, numInputs);
        }
    }

    void NodeGraphGroup::SetNumOutputs(int8_t numOutput) {
        if (!mOutputNode) {
            mOutputNode = NewNode<GraphDataCopy>(NodeType::ExternalOutput, numOutput);
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
        int32_t sourceCount = 0;
        for (auto& it : mNodes) {
            if (it->DetachInput(node)) {
                sourceCount++;
            }
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