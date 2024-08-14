#include "tools/nodegraph/NodeGraph.h"

#include "logging/Log.h"

namespace l::nodegraph {

    bool IsValidInOutNum(int8_t inoutNum, size_t inoutSize) {
        return inoutNum >= 0 && inoutSize < 256u && inoutNum < static_cast<int8_t>(inoutSize);
    }

    NodeGraphBase::NodeGraphBase(std::string_view name) :
        mName(name)
    {
        mInputs.resize(1);
        mOutputs.resize(1);
    }

    void NodeGraphBase::SetNumInputs(int8_t numInputs) {
        mInputs.resize(numInputs);
    }

    void NodeGraphBase::SetNumOutputs(int8_t outputCount) {
        mOutputs.resize(outputCount);
    }

    void NodeGraphBase::Reset() {
        for (auto& link : mInputs) {
            if (link.mInputType == InputType::INPUT_NODE && link.mInput.mInputNode != nullptr) {
                link.mInput.mInputNode->Reset();
            }
        }
    }

    void NodeGraphBase::PreUpdate() {
        mProcessUpdateHasRun = false;
        for (auto& link : mInputs) {
            if (link.mInputType == InputType::INPUT_NODE && link.mInput.mInputNode != nullptr) {
                link.mInput.mInputNode->PreUpdate();
            }
        }
    }

    void NodeGraphBase::Update() {
        PreUpdate();
        ProcessOperation();
    }

    void NodeGraphBase::ProcessOperation() {
        if (mProcessUpdateHasRun) {
            return;
        }
        for (auto& link : mInputs) {
            if (link.mInputType == InputType::INPUT_NODE && link.mInput.mInputNode != nullptr) {
                link.mInput.mInputNode->ProcessOperation();
            }
        }
        mProcessUpdateHasRun = true;
    }

    float NodeGraphBase::Get(int8_t outputChannel) {
        ASSERT(IsValidInOutNum(outputChannel, mOutputs.size()));
        return mOutputs.at(outputChannel).mOutput;
    }

    void NodeGraphBase::SetInput(int8_t inputChannel, NodeGraphBase& source, int8_t sourceOutputChannel) {
        ASSERT(IsValidInOutNum(inputChannel, mInputs.size()));
        Input input;
        input.mInputNode = &source;
        mInputs.at(inputChannel) = NodeGraphInput{ std::move(input), InputType::INPUT_NODE, sourceOutputChannel };
    }

    void NodeGraphBase::SetInput(int8_t inputChannel, NodeGraphGroup& source, int8_t sourceOutputChannel, bool useSourceInternalInput) {
        ASSERT(IsValidInOutNum(inputChannel, mInputs.size()));
        Input input;
        if (useSourceInternalInput) {
            input.mInputNode = &source.GetInputNode();
        }
        else {
            input.mInputNode = &source.GetOutputNode();
        }
        mInputs.at(inputChannel) = NodeGraphInput{ std::move(input), InputType::INPUT_NODE, sourceOutputChannel };
    }

    void NodeGraphBase::SetInput(int8_t inputChannel, float constant) {
        ASSERT(IsValidInOutNum(inputChannel, mInputs.size()));
        Input input;
        input.mInputFloatConstant = constant;
        mInputs.at(inputChannel) = NodeGraphInput{ std::move(input), InputType::INPUT_CONSTANT, 0 };
    }

    void NodeGraphBase::SetInput(int8_t inputChannel, float* floatPtr) {
        ASSERT(IsValidInOutNum(inputChannel, mInputs.size()));
        Input input;
        input.mInputFloat = floatPtr;
        mInputs.at(inputChannel) = NodeGraphInput{ std::move(input), InputType::INPUT_VALUE, 0 };
    }

    void GraphOp::SetNumInputs(int8_t numInputs) {
        mNumInputs = numInputs;
    }

    void GraphOp::SetNumOutputs(int8_t numOutputs) {
        mNumOutputs = numOutputs;
    }

    int8_t GraphOp::GetNumInputs() {
        return mNumInputs;
    }

    int8_t GraphOp::GetNumOutputs() {
        return mNumOutputs;
    }

    void GraphDataCopy::Process(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        for (size_t i = 0; i < inputs.size() && i < outputs.size(); i++) {
            outputs.at(i).mOutput = inputs.at(i).Get();
        }
    }

    float NodeGraphInput::Get() {
        if (mInputType == InputType::INPUT_NODE) {
            if (mInput.mInputNode != nullptr) {
                return mInput.mInputNode->Get(mInputFromOutputChannel);
            }
        }
        else if (mInputType == InputType::INPUT_CONSTANT) {
            return mInput.mInputFloatConstant;
        }
        else if (mInputType == InputType::INPUT_VALUE) {
            return *mInput.mInputFloat;
        }
        else {
            // error
        }
        return 0.0f;
    }

    void NodeGraphGroup::SetNumInputs(int8_t numInputs) {
        mInputNode.SetNumInputs(numInputs);
        mInputNode.SetNumOutputs(numInputs);
    }

    void NodeGraphGroup::SetNumOutputs(int8_t outputCount) {
        mOutputNode.SetNumInputs(outputCount);
        mOutputNode.SetNumOutputs(outputCount);
    }

    void NodeGraphGroup::SetInput(int8_t inputChannel, NodeGraphBase& source, int8_t sourceOutputChannel) {
        mInputNode.SetInput(inputChannel, source, sourceOutputChannel);
    }

    void NodeGraphGroup::SetInput(int8_t inputChannel, NodeGraphGroup& source, int8_t sourceOutputChannel, bool useSourceInternalInput) {
        mInputNode.SetInput(inputChannel, source, sourceOutputChannel, useSourceInternalInput);
    }

    void NodeGraphGroup::SetInput(int8_t inputChannel, float constant) {
        mInputNode.SetInput(inputChannel, constant);
    }

    void NodeGraphGroup::SetInput(int8_t inputChannel, float* floatPtr) {
        mInputNode.SetInput(inputChannel, floatPtr);
    }

    void NodeGraphGroup::SetOutput(int8_t outputChannel, NodeGraphBase& source, int8_t sourceOutputChannel) {
        mOutputNode.SetInput(outputChannel, source, sourceOutputChannel);
    }

    void NodeGraphGroup::SetOutput(int8_t outputChannel, NodeGraphGroup& source, int8_t sourceOutputChannel) {
        mOutputNode.SetInput(outputChannel, source, sourceOutputChannel, false);
    }

    float NodeGraphGroup::Get(int8_t outputChannel) {
        return mOutputNode.Get(outputChannel);
    }

    NodeGraphBase& NodeGraphGroup::GetInputNode() {
        return mInputNode;
    }

    NodeGraphBase& NodeGraphGroup::GetOutputNode() {
        return mOutputNode;
    }

    void NodeGraphGroup::Update() {
        mOutputNode.Update();
    }

}