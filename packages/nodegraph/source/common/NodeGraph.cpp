#include "nodegraph/NodeGraph.h"

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

    int8_t NodeGraphBase::GetNumInputs() {
        return static_cast<int8_t>(mInputs.size());
    }

    int8_t NodeGraphBase::GetNumOutputs() {
        return static_cast<int8_t>(mOutputs.size());
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
        input = NodeGraphInput{ std::move(newInput), InputType::INPUT_NODE, sourceOutputChannel, ""};
        return true;
    }

    bool NodeGraphBase::SetInput(int8_t inputChannel, NodeGraphGroup& source, int8_t sourceChannel, bool nodeIsInsideGroup) {
        Input newInput;
        if (nodeIsInsideGroup) {
            newInput.mInputNode = &source.GetInputNode();
        }
        else {
            newInput.mInputNode = &source.GetOutputNode();
        }

        auto& input = mInputs.at(inputChannel);
        if (!IsValidInOutNum(sourceChannel, newInput.mInputNode->mOutputs.size()) ||
            !IsValidInOutNum(inputChannel, mInputs.size()) || 
            input.HasInput()){
            return false;
        }

        if (nodeIsInsideGroup) {
            source.GetInputNode().SetInputName(sourceChannel, input.mName);
        }

        input = NodeGraphInput{ std::move(newInput), InputType::INPUT_NODE, sourceChannel, "" };
        return true;
    }

    bool NodeGraphBase::SetInput(int8_t inputChannel, float constant) {
        auto& input = mInputs.at(inputChannel);
        if (!IsValidInOutNum(inputChannel, mInputs.size()) || input.HasInput()) {
            return false;
        }

        Input newInput;
        newInput.mInputFloatConstant = constant;
        input = NodeGraphInput{ std::move(newInput), InputType::INPUT_CONSTANT, 0, "" };
        return true;
    }

    bool NodeGraphBase::SetInput(int8_t inputChannel, float* floatPtr) {
        auto& input = mInputs.at(inputChannel);
        if (!IsValidInOutNum(inputChannel, mInputs.size()) || input.HasInput()) {
            return false;
        }

        Input newInput;
        newInput.mInputFloat = floatPtr;
        input = NodeGraphInput{ std::move(newInput), InputType::INPUT_VALUE, 0, "" };
        return true;
    }

    bool NodeGraphBase::RemoveSource(void* source) {
        int32_t sourceRemoved = 0;
        for (auto& it : mInputs) {
            if ((it.mInputType == InputType::INPUT_NODE && it.mInput.mInputNode == source) ||
                (it.mInputType == InputType::INPUT_VALUE && it.mInput.mInputFloat == source)) {
                it.Reset();
                sourceRemoved++;
            }
        }
        return sourceRemoved > 0 ? true : false;
    }

    std::string_view NodeGraphBase::GetName() {
        return mName;
    }

    std::string_view NodeGraphBase::GetInputName(int8_t inputChannel) {
        return mInputs.at(inputChannel).mName;
    }

    std::string_view NodeGraphBase::GetOutputName(int8_t outputChannel) {
        return mOutputs.at(outputChannel).mName;
    }

    void NodeGraphBase::SetInputName(int8_t inputChannel, std::string_view name) {
        mInputs.at(inputChannel).mName = name;
    }

    void NodeGraphBase::SetOutputName(int8_t outputChannel, std::string_view name) {
        mOutputs.at(outputChannel).mName = name;
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

    void NodeGraphInput::Reset() {
        mInput.mInputNode = nullptr;
        mInputType = InputType::INPUT_EMPTY;
        mInputFromOutputChannel = 0;
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
        case InputType::INPUT_VALUE:
            return mInput.mInputFloat != nullptr;
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
        mOutputNode.SetOutputName(outputChannel, source.GetOutputName(sourceOutputChannel));
    }

    void NodeGraphGroup::SetOutput(int8_t outputChannel, NodeGraphGroup& source, int8_t sourceOutputChannel) {
        mOutputNode.SetInput(outputChannel, source, sourceOutputChannel, false);
        mOutputNode.SetOutputName(outputChannel, source.GetOutputNode().GetOutputName(sourceOutputChannel));
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
        auto node = GetNode(id);
        int32_t sourceCount = 0;
        for (auto& it : mNodes) {
            if (it->RemoveSource(node)) {
                sourceCount++;
            }
        }
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