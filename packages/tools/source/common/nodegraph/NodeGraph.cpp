#include "tools/nodegraph/NodeGraph.h"

#include "logging/Log.h"

namespace l::nodegraph {

    bool IsValidInOutNum(int8_t inoutNum, size_t inoutSize) {
        return inoutNum >= 0 && inoutSize < 256u && inoutNum < static_cast<int8_t>(inoutSize);
    }

    int32_t CreateUniqueId() {
        static int32_t id = 1;
        return id++;
    }

    NodeGraphBase::NodeGraphBase(std::string_view name) :
        mId(CreateUniqueId()),
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

    bool NodeGraphBase::ClearInput(int8_t inputChannel) {
        auto& input = mInputs.at(inputChannel);
        if (!IsValidInOutNum(inputChannel, mInputs.size()) || !input.HasInput()) {
            return false;
        }
        input.mInputType = InputType::INPUT_EMPTY;
        input.mInput.mInputNode = nullptr;
        input.mInputFromOutputChannel = 0;
        return true;
    }

    bool NodeGraphBase::SetInput(int8_t inputChannel, NodeGraphBase& source, int8_t sourceOutputChannel) {
        auto& input = mInputs.at(inputChannel);
        if (!IsValidInOutNum(sourceOutputChannel, source.mOutputs.size()) || 
            !IsValidInOutNum(inputChannel, mInputs.size()) || 
            input.HasInput()) {
            return false;
        }

        Input newInput;
        newInput.mInputNode = &source;
        input = NodeGraphInput{ std::move(newInput), InputType::INPUT_NODE, sourceOutputChannel };
        return true;
    }

    bool NodeGraphBase::SetInput(int8_t inputChannel, NodeGraphGroup& source, int8_t sourceOutputChannel, bool useSourceInternalInput) {
        auto& input = mInputs.at(inputChannel);
        if (!IsValidInOutNum(sourceOutputChannel, source.GetOutputNode().mOutputs.size()) || 
            !IsValidInOutNum(inputChannel, mInputs.size()) || 
            input.HasInput()){
            return false;
        }

        Input newInput;
        if (useSourceInternalInput) {
            newInput.mInputNode = &source.GetInputNode();
        }
        else {
            newInput.mInputNode = &source.GetOutputNode();
        }
        input = NodeGraphInput{ std::move(newInput), InputType::INPUT_NODE, sourceOutputChannel };
        return true;
    }

    bool NodeGraphBase::SetInput(int8_t inputChannel, float constant) {
        auto& input = mInputs.at(inputChannel);
        if (!IsValidInOutNum(inputChannel, mInputs.size()) || input.HasInput()) {
            return false;
        }

        Input newInput;
        newInput.mInputFloatConstant = constant;
        input = NodeGraphInput{ std::move(newInput), InputType::INPUT_CONSTANT, 0 };
        return true;
    }

    bool NodeGraphBase::SetInput(int8_t inputChannel, float* floatPtr) {
        auto& input = mInputs.at(inputChannel);
        if (!IsValidInOutNum(inputChannel, mInputs.size()) || input.HasInput()) {
            return false;
        }

        Input newInput;
        newInput.mInputFloat = floatPtr;
        input = NodeGraphInput{ std::move(newInput), InputType::INPUT_VALUE, 0 };
        return true;
    }

    void NodeGraphOp::SetNumInputs(int8_t numInputs) {
        mNumInputs = numInputs;
    }

    void NodeGraphOp::SetNumOutputs(int8_t numOutputs) {
        mNumOutputs = numOutputs;
    }

    int8_t NodeGraphOp::GetNumInputs() {
        return mNumInputs;
    }

    int8_t NodeGraphOp::GetNumOutputs() {
        return mNumOutputs;
    }

    void GraphDataCopy::Process(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        for (size_t i = 0; i < inputs.size() && i < outputs.size(); i++) {
            outputs.at(i).mOutput = inputs.at(i).Get();
        }
    }

    bool NodeGraphInput::HasInput() {
        switch (mInputType) {
        case InputType::INPUT_NODE:
            if (mInput.mInputNode != nullptr) {
                return true;
            }
            break;
        case InputType::INPUT_CONSTANT:
            return true;
            break;
        case InputType::INPUT_VALUE:
            return mInput.mInputFloat != nullptr;
            break;
        case InputType::INPUT_EMPTY:
            break;
        }
        return false;
    }

    float NodeGraphInput::Get() {
        switch (mInputType) {
        case InputType::INPUT_NODE:
            if (mInput.mInputNode != nullptr) {
                return mInput.mInputNode->Get(mInputFromOutputChannel);
            }
            break;
        case InputType::INPUT_CONSTANT:
            return mInput.mInputFloatConstant;
            break;
        case InputType::INPUT_VALUE:
            return *mInput.mInputFloat;
            break;
        case InputType::INPUT_EMPTY:
            break;
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
        auto count = std::erase_if(mNodes, [&](const std::unique_ptr<NodeGraphBase>& node) {
            if (node->GetId() == id) {
                return true;
            }
            return false;
            });
        return count > 0 ? true : false;
    }

    void NodeGraphGroup::Update() {
        mOutputNode.Update();
    }

}