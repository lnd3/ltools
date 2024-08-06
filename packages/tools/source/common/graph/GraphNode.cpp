#include "tools/graph/GraphNode.h"

#include "logging/Log.h"

namespace l::graph {

    bool IsValidInOutNum(int8_t inoutNum, size_t inoutSize) {
        return inoutNum >= 0 && inoutSize < 256u && inoutNum < static_cast<int8_t>(inoutSize);
    }

    GraphNodeBase::GraphNodeBase(std::string_view name) :
        mName(name)
    {
        mInputs.resize(1);
        mOutputs.resize(1);
    }

    void GraphNodeBase::SetNumInputs(int8_t numInputs) {
        mInputs.resize(numInputs);
    }

    void GraphNodeBase::SetNumOutputs(int8_t outputCount) {
        mOutputs.resize(outputCount);
    }

    void GraphNodeBase::Reset() {
        for (auto& link : mInputs) {
            if (link.mInputType == InputType::INPUT_NODE && link.mInput.mInputNode != nullptr) {
                link.mInput.mInputNode->Reset();
            }
        }
    }

    void GraphNodeBase::PreUpdate() {
        mProcessUpdateHasRun = false;
        for (auto& link : mInputs) {
            if (link.mInputType == InputType::INPUT_NODE && link.mInput.mInputNode != nullptr) {
                link.mInput.mInputNode->PreUpdate();
            }
        }
    }

    void GraphNodeBase::Update() {
        PreUpdate();
        ProcessOperation();
    }

    void GraphNodeBase::ProcessOperation() {
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

    float GraphNodeBase::Get(int8_t outputChannel) {
        ASSERT(IsValidInOutNum(outputChannel, mOutputs.size()));
        return mOutputs.at(outputChannel).mOutput;
    }

    void GraphNodeBase::SetInput(int8_t inputChannel, GraphNodeBase& source, int8_t sourceOutputChannel) {
        ASSERT(IsValidInOutNum(inputChannel, mInputs.size()));
        Input input;
        input.mInputNode = &source;
        mInputs.at(inputChannel) = GraphNodeInput{ std::move(input), InputType::INPUT_NODE, sourceOutputChannel };
    }

    void GraphNodeBase::SetInput(int8_t inputChannel, GraphNodeGroup& source, int8_t sourceOutputChannel, bool useSourceInternalInput) {
        ASSERT(IsValidInOutNum(inputChannel, mInputs.size()));
        Input input;
        if (useSourceInternalInput) {
            input.mInputNode = &source.GetInputNode();
        }
        else {
            input.mInputNode = &source.GetOutputNode();
        }
        mInputs.at(inputChannel) = GraphNodeInput{ std::move(input), InputType::INPUT_NODE, sourceOutputChannel };
    }

    void GraphNodeBase::SetInput(int8_t inputChannel, float constant) {
        ASSERT(IsValidInOutNum(inputChannel, mInputs.size()));
        Input input;
        input.mInputFloatConstant = constant;
        mInputs.at(inputChannel) = GraphNodeInput{ std::move(input), InputType::INPUT_CONSTANT, 0 };
    }

    void GraphNodeBase::SetInput(int8_t inputChannel, float* floatPtr) {
        ASSERT(IsValidInOutNum(inputChannel, mInputs.size()));
        Input input;
        input.mInputFloat = floatPtr;
        mInputs.at(inputChannel) = GraphNodeInput{ std::move(input), InputType::INPUT_VALUE, 0 };
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

    void GraphDataCopy::Process(std::vector<GraphNodeInput>& inputs, std::vector<GraphNodeOutput>& outputs) {
        for (size_t i = 0; i < inputs.size() && i < outputs.size(); i++) {
            outputs.at(i).mOutput = inputs.at(i).Get();
        }
    }

    float GraphNodeInput::Get() {
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

    void GraphNodeGroup::SetNumInputs(int8_t numInputs) {
        mInputNode.SetNumInputs(numInputs);
        mInputNode.SetNumOutputs(numInputs);
    }

    void GraphNodeGroup::SetNumOutputs(int8_t outputCount) {
        mOutputNode.SetNumInputs(outputCount);
        mOutputNode.SetNumOutputs(outputCount);
    }

    void GraphNodeGroup::SetInput(int8_t inputChannel, GraphNodeBase& source, int8_t sourceOutputChannel) {
        mInputNode.SetInput(inputChannel, source, sourceOutputChannel);
    }

    void GraphNodeGroup::SetInput(int8_t inputChannel, GraphNodeGroup& source, int8_t sourceOutputChannel, bool useSourceInternalInput) {
        mInputNode.SetInput(inputChannel, source, sourceOutputChannel, useSourceInternalInput);
    }

    void GraphNodeGroup::SetInput(int8_t inputChannel, float constant) {
        mInputNode.SetInput(inputChannel, constant);
    }

    void GraphNodeGroup::SetInput(int8_t inputChannel, float* floatPtr) {
        mInputNode.SetInput(inputChannel, floatPtr);
    }

    void GraphNodeGroup::SetOutput(int8_t outputChannel, GraphNodeBase& source, int8_t sourceOutputChannel) {
        mOutputNode.SetInput(outputChannel, source, sourceOutputChannel);
    }

    void GraphNodeGroup::SetOutput(int8_t outputChannel, GraphNodeGroup& source, int8_t sourceOutputChannel) {
        mOutputNode.SetInput(outputChannel, source, sourceOutputChannel, false);
    }

    float GraphNodeGroup::Get(int8_t outputChannel) {
        return mOutputNode.Get(outputChannel);
    }

    GraphNodeBase& GraphNodeGroup::GetInputNode() {
        return mInputNode;
    }

    GraphNodeBase& GraphNodeGroup::GetOutputNode() {
        return mOutputNode;
    }

    void GraphNodeGroup::Update() {
        mOutputNode.Update();
    }

}