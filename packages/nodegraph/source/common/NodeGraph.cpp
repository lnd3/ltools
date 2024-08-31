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

    void NodeGraphBase::SetNumInputs(int8_t numInputs) {
        mInputCount = numInputs;
        mInputs.resize(mInputCount + mConstantCount);
    }

    void NodeGraphBase::SetNumOutputs(int8_t outputCount) {
        mOutputs.resize(outputCount);
    }

    void NodeGraphBase::SetNumConstants(int8_t numConstants) {
        mConstantCount = numConstants;
        mInputs.resize(mInputCount + mConstantCount);
    }

    int8_t NodeGraphBase::GetNumInputs() {
        ASSERT(mInputs.size() == static_cast<size_t>(mInputCount + mConstantCount));
        return static_cast<int8_t>(mInputCount);
    }

    int8_t NodeGraphBase::GetNumOutputs() {
        return static_cast<int8_t>(mOutputs.size());
    }

    int8_t NodeGraphBase::GetNumConstants() {
        ASSERT(mInputs.size() == static_cast<size_t>(mInputCount + mConstantCount));
        return static_cast<int8_t>(mConstantCount);
    }

    void NodeGraphBase::Reset() {
        for (auto& link : mInputs) {
            if (link.mInputType == InputType::INPUT_NODE && link.mInput.mInputNode != nullptr) {
                link.mInput.mInputNode->Reset();
            }
        }
    }

    void NodeGraphBase::ClearProcessFlags() {
        mProcessUpdateHasRun = false;
        for (auto& link : mInputs) {
            if (link.mInputType == InputType::INPUT_NODE && link.mInput.mInputNode != nullptr) {
                link.mInput.mInputNode->ClearProcessFlags();
            }
        }
    }

    void NodeGraphBase::ProcessSubGraph(bool recomputeSubGraphCache) {
        if (recomputeSubGraphCache) {
            ClearProcessFlags();
        }
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

    void NodeGraphBase::Tick(float time) {
        for (auto& link : mInputs) {
            if (link.mInputType == InputType::INPUT_NODE && link.mInput.mInputNode != nullptr) {
                link.mInput.mInputNode->Tick(time);
            }
        }
    }

    float& NodeGraphBase::Get(int8_t outputChannel) {
        return mOutputs.at(outputChannel).mOutput;
    }

    float NodeGraphBase::GetInput(int8_t inputChannel) {
        return mInputs.at(inputChannel).Get();
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
        ASSERT(inputChannel >= mInputCount && inputChannel < mInputCount + mConstantCount);
        auto& input = mInputs.at(inputChannel);
        if (!IsValidInOutNum(inputChannel, mInputs.size())) {
            return false;
        }

        Input newInput;
        newInput.mInputFloatConstant = constant;
        input = NodeGraphInput{ std::move(newInput), InputType::INPUT_CONSTANT, 0, "" };
        return true;
    }

    bool NodeGraphBase::SetInput(int8_t inputChannel, float* floatPtr) {
        auto& input = mInputs.at(inputChannel);
        if (!IsValidInOutNum(inputChannel, mInputs.size())) {
            return false;
        }

        Input newInput;
        newInput.mInputFloat = floatPtr;
        input = NodeGraphInput{ std::move(newInput), InputType::INPUT_VALUE, 0, "" };
        return true;
    }

    bool NodeGraphBase::RemoveInput(void* source) {
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

    bool NodeGraphBase::IsDataVisible(int8_t) {
        return false;
    }

    bool NodeGraphBase::IsDataEditable(int8_t) {
        return false;
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

    void NodeGraphOp::SetNumConstants(int8_t numConstants) {
        mNumConstants = numConstants;
    }

    int8_t NodeGraphOp::GetNumInputs() {
        return mNumInputs;
    }

    int8_t NodeGraphOp::GetNumOutputs() {
        return mNumOutputs;
    }

    int8_t NodeGraphOp::GetNumConstants() {
        return mNumConstants;
    }

    bool NodeGraphOp::IsDataVisible(int8_t) {
        return false;
    }

    bool NodeGraphOp::IsDataEditable(int8_t) {
        return false;
    }

    std::string_view NodeGraphOp::GetInputName(int8_t inputChannel) {
        return defaultInStrings[inputChannel];
    }

    std::string_view NodeGraphOp::GetOutputName(int8_t outputChannel) {
        return defaultOutStrings[outputChannel];
    }

    std::string_view NodeGraphOp::GetName() {
        return "";
    }

    void GraphDataCopy::ProcessSubGraph(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        for (size_t i = 0; i < inputs.size() && i < outputs.size(); i++) {
            outputs.at(i).mOutput = inputs.at(i).Get();
        }
    }

    void NodeGraphInput::Reset() {
        if (mInputType == InputType::INPUT_NODE || mInputType == InputType::INPUT_VALUE) {
            mInput.mInputNode = nullptr;
            mInputType = InputType::INPUT_EMPTY;
            mInputFromOutputChannel = 0;
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
            break;
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
            if (it->RemoveInput(node)) {
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

    void NodeGraphGroup::ClearProcessFlags() {
        mOutputNode.ClearProcessFlags();
    }

    void NodeGraphGroup::ProcessSubGraph(bool recomputeSubGraphCache) {
        if (recomputeSubGraphCache) {
            mOutputNode.ClearProcessFlags();
        }
        mOutputNode.ProcessSubGraph(false);
    }

    void NodeGraphGroup::Tick(float time) {
        for (auto& it : mNodes) {
            it->Tick(time);
        }
    }
}