#include "nodegraph/core/NodeGraphBase.h"
#include "nodegraph/core/NodeGraphGroup.h"

#include "logging/Log.h"

#include "math/MathFunc.h"

namespace l::nodegraph {

    void GraphDataCopy::Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        for (size_t i = 0; i < inputs.size() && i < outputs.size(); i++) {
            auto numInputSamples = inputs.at(i).GetSize();
            if (numSamples > 1) {
                ASSERT(numInputSamples == numSamples);
            }

            auto input = &inputs.at(i).Get(numInputSamples);
            auto output = &outputs.at(i).Get(numSamples);
            for (int32_t j = 0; j < numSamples; j++) {
                *output++ = *input++;
            }
        }
    }

    void NodeGraphGroup::SetNumInputs(int8_t numInputs) {
        if (!mInputNode) {
            mInputNode = NewNode<GraphDataCopy>(OutputType::Default, numInputs);
        }
    }

    void NodeGraphGroup::SetNumOutputs(int8_t numOutput) {
        if (!mOutputNode) {
            mOutputNode = NewNode<GraphDataCopy>(OutputType::ExternalOutput, numOutput);
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
        mOutputNode->SetOutputName(outputChannel, source.GetOutputName(sourceOutputChannel));
    }

    void NodeGraphGroup::SetOutput(int8_t outputChannel, NodeGraphGroup& source, int8_t sourceOutputChannel) {
        mOutputNode->SetInput(outputChannel, source, sourceOutputChannel);
        mOutputNode->SetOutputName(outputChannel, source.GetOutputNode().GetOutputName(sourceOutputChannel));
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
        auto it = std::find_if(mNodes.begin(), mNodes.end(), [&](const std::unique_ptr<NodeGraphBase>& node) {
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
        auto it = std::find_if(mNodes.begin(), mNodes.end(), [&](const std::unique_ptr<NodeGraphBase>& node) {
            if (node->GetId() == id) {
                return true;
            }
            return false;
            });
        if (it != mNodes.end()) {
            return it->get();
        }
        return nullptr;
    }

    bool NodeGraphGroup::RemoveNode(int32_t id) {
        auto node = GetNode(id);
        int32_t sourceCount = 0;
        for (auto& it : mNodes) {
            if (it->DetachInput(node)) {
                sourceCount++;
            }
        }
        std::erase_if(mOutputNodes, [&](NodeGraphBase* nodePtr) {
            if (nodePtr == node) {
                return true;
            }
            return false;
            });
        auto count = std::erase_if(mNodes, [&](const std::unique_ptr<NodeGraphBase>& node) {
            if (node->GetId() == id) {
                return true;
            }
            return false;
            });
        return count > 0 ? true : false;
    }

    void NodeGraphGroup::ClearProcessFlags() {
        mOutputNode->ClearProcessFlags();
    }

    void NodeGraphGroup::ProcessSubGraph(int32_t numSamples) {
        for (auto& it : mOutputNodes) {
            it->ClearProcessFlags();
        }
        for (auto& it : mOutputNodes) {
            it->ProcessSubGraph(numSamples, false);
        }
    }

    void NodeGraphGroup::Tick(int32_t tickCount, float elapsed) {
        if (tickCount <= mLastTickCount) {
            return;
        }
        for (auto& it : mNodes) {
            it->Tick(tickCount, elapsed);
        }
        mLastTickCount = tickCount;
    }
}