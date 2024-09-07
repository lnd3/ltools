#include "nodegraph/NodeGraph.h"

#include "logging/Log.h"

#include "math/MathFunc.h"

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

    void NodeGraphBase::ProcessSubGraph(int32_t numSamples, bool recomputeSubGraphCache) {
        if (recomputeSubGraphCache) {
            ClearProcessFlags();
        }
        ProcessOperation(numSamples);
    }

    void NodeGraphBase::ProcessOperation(int32_t numSamples) {
        if (mProcessUpdateHasRun) {
            return;
        }
        for (auto& link : mInputs) {
            if (link.mInputType == InputType::INPUT_NODE && link.mInput.mInputNode != nullptr) {
                link.mInput.mInputNode->ProcessOperation(numSamples);
            }
        }
        mProcessUpdateHasRun = true;
    }

    void NodeGraphBase::Tick(int32_t tickCount, float elapsed) {
        if (tickCount <= mLastTickCount) {
            return;
        }
        for (auto& link : mInputs) {
            if (link.mInputType == InputType::INPUT_NODE && link.mInput.mInputNode != nullptr) {
                link.mInput.mInputNode->Tick(tickCount, elapsed);
            }
        }
        mLastTickCount = tickCount;
    }

    float& NodeGraphBase::GetOutput(int8_t outputChannel, int32_t numSamples) {
        return mOutputs.at(outputChannel).GetOutput(numSamples);
    }

    int32_t NodeGraphBase::GetOutputSize(int8_t outputChannel) {
        return mOutputs.at(outputChannel).GetOutputSize();
    }

    float NodeGraphBase::GetInput(int8_t inputChannel) {
        return mInputs.at(inputChannel).Get();
    }

    bool NodeGraphBase::ClearInput(int8_t inputChannel) {
        ASSERT(inputChannel >= 0 && static_cast<size_t>(inputChannel) < mInputs.size());
        if (!IsValidInOutNum(inputChannel, mInputs.size())) {
            return false;
        }
        auto& input = mInputs.at(inputChannel);
        if (!input.HasInput()) {
            return false;
        }
        input.mInputType = InputType::INPUT_EMPTY;
        input.mInput.mInputNode = nullptr;
        input.mInputFromOutputChannel = 0;
        return true;
    }

    bool NodeGraphBase::SetInput(int8_t inputChannel, NodeGraphBase& source, int8_t sourceOutputChannel) {
        ASSERT(inputChannel >= 0 && static_cast<size_t>(inputChannel) < mInputs.size());
        if (!IsValidInOutNum(inputChannel, mInputs.size())) {
            return false;
        }
        auto& input = mInputs.at(inputChannel);
        if (!IsValidInOutNum(sourceOutputChannel, source.mOutputs.size()) ||
            !IsValidInOutNum(inputChannel, mInputs.size()) ||
            input.HasInput()) {
            return false;
        }
        input.mInput.mInputNode = &source;
        input.mInputType = InputType::INPUT_NODE;
        input.mInputFromOutputChannel = sourceOutputChannel;
        return true;
    }

    bool NodeGraphBase::SetInput(int8_t inputChannel, NodeGraphGroup& source, int8_t sourceChannel) {
        ASSERT(inputChannel >= 0 && static_cast<size_t>(inputChannel) < mInputs.size());
        if (!IsValidInOutNum(inputChannel, mInputs.size())) {
            return false;
        }
        auto& input = mInputs.at(inputChannel);
        if (source.ContainsNode(GetId())) {
            if (!IsValidInOutNum(sourceChannel, source.GetInputNode().GetNumOutputs()) ||
                !IsValidInOutNum(inputChannel, mInputs.size()) ||
                input.HasInput()) {
                return false;
            }
            input.mInput.mInputNode = &source.GetInputNode();
            source.GetInputNode().SetInputName(sourceChannel, input.mName ? *input.mName : "");
        }
        else {
            if (!IsValidInOutNum(sourceChannel, source.GetOutputNode().GetNumOutputs()) ||
                !IsValidInOutNum(inputChannel, mInputs.size()) ||
                input.HasInput()) {
                return false;
            }
            input.mInput.mInputNode = &source.GetOutputNode();
        }
        input.mInputType = InputType::INPUT_NODE;
        input.mInputFromOutputChannel = sourceChannel;
        return true;
    }

    bool NodeGraphBase::SetInput(int8_t inputChannel, float constant) {
        ASSERT(inputChannel >= 0 && static_cast<size_t>(inputChannel) < mInputs.size());
        if (!IsValidInOutNum(inputChannel, mInputs.size())) {
            return false;
        }
        auto& input = mInputs.at(inputChannel);
        input.mInput.mInputFloatConstant = constant;
        input.mInputType = InputType::INPUT_CONSTANT;
        input.mInputFromOutputChannel = 0;

        return true;
    }

    bool NodeGraphBase::SetInput(int8_t inputChannel, float* floatPtr) {
        ASSERT(inputChannel >= 0 && static_cast<size_t>(inputChannel) < mInputs.size());
        if (!IsValidInOutNum(inputChannel, mInputs.size())) {
            return false;
        }
        auto& input = mInputs.at(inputChannel);
        input.mInput.mInputFloat = floatPtr;
        input.mInputType = InputType::INPUT_VALUE;
        input.mInputFromOutputChannel = 0;
        return true;
    }

    bool NodeGraphBase::SetInputBound(int8_t inputChannel, InputBound bound, float boundMin, float boundMax) {
        ASSERT(inputChannel >= 0 && static_cast<size_t>(inputChannel) < mInputs.size());
        if (!IsValidInOutNum(inputChannel, mInputs.size())) {
            return false;
        }
        auto& input = mInputs.at(inputChannel);
        input.mInputBound = bound;
        switch (bound) {
        case InputBound::INPUT_0_TO_1:
            input.mBoundMin = 0.0f;
            input.mBoundMax = 1.0f;
            break;
        case InputBound::INPUT_NEG_1_POS_1:
            input.mBoundMin = -1.0f;
            input.mBoundMax = 1.0f;
            break;
        case InputBound::INPUT_0_100:
            input.mBoundMin = 0.0f;
            input.mBoundMax = 100.0f;
            break;
        case InputBound::INPUT_CUSTOM:
            input.mBoundMin = boundMin;
            input.mBoundMax = boundMax;
            break;
        case InputBound::INPUT_UNBOUNDED:
            input.mBoundMin = -l::math::constants::FLTMAX;
            input.mBoundMax = l::math::constants::FLTMAX;
            break;
        case InputBound::INPUT_DONTCHANGE:
            break;
        }
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

    bool NodeGraphBase::IsOutputPolled(int8_t outputChannel) {
        return mOutputs.at(outputChannel).IsOutputPolled();
    }

    OutputType NodeGraphBase::GetOutputType() {
        return mOutputType;
    }

    std::string_view NodeGraphBase::GetName() {
        return mName;
    }

    std::string_view NodeGraphBase::GetInputName(int8_t inputChannel) {
        if (!mInputs.at(inputChannel).mName) {
            return "";
        }
        return *mInputs.at(inputChannel).mName;
    }

    std::string_view NodeGraphBase::GetOutputName(int8_t outputChannel) {
        if (!mOutputs.at(outputChannel).mName) {
            return "";
        }
        return *mOutputs.at(outputChannel).mName;
    }

    void NodeGraphBase::SetInputName(int8_t inputChannel, std::string_view name) {
        if (!mInputs.at(inputChannel).mName) {
            mInputs.at(inputChannel).mName = std::make_unique<std::string>(name);
        }
        else {
            *mInputs.at(inputChannel).mName = name;
        }
    }

    void NodeGraphBase::SetOutputName(int8_t outputChannel, std::string_view name) {
        if (!mOutputs.at(outputChannel).mName) {
            mOutputs.at(outputChannel).mName = std::make_unique<std::string>(name);
        }
        else {
            *mOutputs.at(outputChannel).mName = name;
        }
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

    void GraphDataCopy::Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
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
        float value = 0.0f;
        switch (mInputType) {
        case InputType::INPUT_NODE:
            if (mInput.mInputNode != nullptr) {
                value = mInput.mInputNode->GetOutput(mInputFromOutputChannel);
            }
            break;
        case InputType::INPUT_CONSTANT:
            value = mInput.mInputFloatConstant;
            break;
        case InputType::INPUT_VALUE:
            value = *mInput.mInputFloat;
            break;
        case InputType::INPUT_EMPTY:
            break;
        }
        return l::math::functions::clamp(value, mBoundMin, mBoundMax);
    }

    float& NodeGraphInput::Get(int32_t numSamples) {
        switch (mInputType) {
        case InputType::INPUT_NODE:
            if (mInput.mInputNode != nullptr) {
                return mInput.mInputNode->GetOutput(mInputFromOutputChannel, numSamples);
            }
            break;
        case InputType::INPUT_CONSTANT:
            return mInput.mInputFloatConstant;
        case InputType::INPUT_VALUE:
            return *mInput.mInputFloat;
        case InputType::INPUT_EMPTY:
            break;
        }
        return mInput.mInputFloatConstant;
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

    void NodeGraphGroup::SetInput(int8_t inputChannel, NodeGraphGroup& source, int8_t sourceOutputChannel) {
        mInputNode.SetInput(inputChannel, source, sourceOutputChannel);
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
        mOutputNode.SetInput(outputChannel, source, sourceOutputChannel);
        mOutputNode.SetOutputName(outputChannel, source.GetOutputNode().GetOutputName(sourceOutputChannel));
    }

    float NodeGraphGroup::GetOutput(int8_t outputChannel) {
        return mOutputNode.GetOutput(outputChannel);
    }

    NodeGraphBase& NodeGraphGroup::GetInputNode() {
        return mInputNode;
    }

    NodeGraphBase& NodeGraphGroup::GetOutputNode() {
        return mOutputNode;
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
            if (it->RemoveInput(node)) {
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
        mOutputNode.ClearProcessFlags();
    }

    void NodeGraphGroup::ProcessSubGraph(int32_t numSamples, bool) {
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