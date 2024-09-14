#include "nodegraph/core/NodeGraphBase.h"
#include "nodegraph/core/NodeGraphGroup.h"

#include "logging/Log.h"

#include "math/MathFunc.h"

namespace l::nodegraph {

    int32_t CreateUniqueId() {
        static int32_t id = 1;
        return id++;
    }

    bool IsValidInOutNum(int8_t inoutNum, size_t inoutSize) {
        return inoutNum >= 0 && inoutSize < 256u && inoutNum < static_cast<int8_t>(inoutSize);
    }

    void NodeGraphBase::SetNumInputs(int8_t numInputs) {
        mInputs.resize(numInputs);
    }

    void NodeGraphBase::SetNumOutputs(int8_t numOutputs) {
        mOutputs.resize(numOutputs);
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

    float& NodeGraphBase::GetInput(int8_t inputChannel, int32_t size) {
        return mInputs.at(inputChannel).Get(size);
    }

    float& NodeGraphBase::GetOutput(int8_t outputChannel, int32_t size) {
        return mOutputs.at(outputChannel).Get(size);
    }

    int32_t NodeGraphBase::GetInputSize(int8_t inputChannel) {
        return mInputs.at(inputChannel).GetSize();
    }

    int32_t NodeGraphBase::GetOutputSize(int8_t outputChannel) {
        return mOutputs.at(outputChannel).GetSize();
    }

    bool NodeGraphBase::ClearInput(int8_t inputChannel) {
        ASSERT(inputChannel >= 0 && static_cast<size_t>(inputChannel) < mInputs.size());
        if (!IsValidInOutNum(inputChannel, mInputs.size())) {
            return false;
        }
        auto& input = mInputs.at(inputChannel);
        if (!input.HasInputNode()) {
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
            input.HasInputNode()) {
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
                input.HasInputNode()) {
                return false;
            }
            input.mInput.mInputNode = &source.GetInputNode();
            source.GetInputNode().SetInputName(sourceChannel, input.mName ? *input.mName : "");
        }
        else {
            if (!IsValidInOutNum(sourceChannel, source.GetOutputNode().GetNumOutputs()) ||
                !IsValidInOutNum(inputChannel, mInputs.size()) ||
                input.HasInputNode()) {
                return false;
            }
            input.mInput.mInputNode = &source.GetOutputNode();
        }
        input.mInputType = InputType::INPUT_NODE;
        input.mInputFromOutputChannel = sourceChannel;
        return true;
    }

    bool NodeGraphBase::SetInput(int8_t inputChannel, float constant, int32_t size) {
        ASSERT(inputChannel >= 0 && static_cast<size_t>(inputChannel) < mInputs.size());
        if (!IsValidInOutNum(inputChannel, mInputs.size())) {
            return false;
        }
        auto& input = mInputs.at(inputChannel);
        constant = l::math::functions::clamp(constant, input.mBoundMin, input.mBoundMax);
        if (size <= 1) {
            input.mInput.mInputFloatConstant = constant;
            input.mInputType = InputType::INPUT_CONSTANT;
            input.mInputFromOutputChannel = 0;
        }
        else {
            input.mInputType = InputType::INPUT_ARRAY;
            input.mInputFromOutputChannel = 0;
            auto inputBuf = &input.Get(size);
            for (int32_t i = 0; i < size; i++) {
                *inputBuf++ = constant;
            }
            input.mInput.mInputFloatConstant = constant;
        }

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

    void NodeGraphBase::SetDefaultOutput(int8_t outputChannel, float defaultValue, int32_t size) {
        auto output = &GetOutput(outputChannel, size);
        for (int32_t i = 0; i < size; i++) {
            *output++ = defaultValue;
        }
    }

    bool NodeGraphBase::SetInputBound(int8_t inputChannel, InputBound bound, float boundMin, float boundMax) {
        ASSERT(inputChannel >= 0 && static_cast<size_t>(inputChannel) < mInputs.size());
        if (!IsValidInOutNum(inputChannel, mInputs.size())) {
            return false;
        }
        auto& input = mInputs.at(inputChannel);

        if (bound == InputBound::INPUT_CUSTOM) {
            input.mBoundMin = boundMin;
            input.mBoundMax = boundMax;
        }
        else {
            auto [min, max] = GetInputBounds(bound);
            input.mBoundMin = min;
            input.mBoundMax = max;
        }
        return true;
    }

    bool NodeGraphBase::DetachInput(void* source) {
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

    bool NodeGraphBase::IsDataConstant(int8_t) {
        return false;
    }

    bool NodeGraphBase::IsDataVisible(int8_t) {
        return false;
    }

    bool NodeGraphBase::IsDataEditable(int8_t) {
        return false;
    }

    bool NodeGraphBase::IsOutputPolled(int8_t outputChannel) {
        return mOutputs.at(outputChannel).IsPolled();
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

    void NodeGraphOp::Reset() {
        for (int8_t i = 0; i < static_cast<int8_t>(mDefaultInData.size()); i++) {
            auto& e = mDefaultInData.at(i);
            mNode->SetInput(i, std::get<0>(e), std::get<1>(e));
            mNode->SetInputBound(i, l::nodegraph::InputBound::INPUT_CUSTOM, std::get<2>(e), std::get<3>(e));
        }
        for (int8_t i = 0; i < static_cast<int8_t>(mDefaultOutData.size()); i++) {
            auto& e = mDefaultOutData.at(i);
            auto output = &mNode->GetOutput(i, std::get<1>(e));
            for (int32_t j = 0; j < std::get<1>(e);j++) {
                *output++ = std::get<0>(e);
            }
        }
    }

    int8_t NodeGraphOp::GetNumInputs() {
        return mNumInputs;
    }

    int8_t NodeGraphOp::GetNumOutputs() {
        return mNumOutputs;
    }

    bool NodeGraphOp::IsDataConstant(int8_t channel) {
        return std::get<4>(mDefaultInData.at(channel));
    }

    bool NodeGraphOp::IsDataVisible(int8_t channel) {
        return std::get<5>(mDefaultInData.at(channel));
    }

    bool NodeGraphOp::IsDataEditable(int8_t channel) {
        return std::get<6>(mDefaultInData.at(channel));
    }

    std::string_view NodeGraphOp::GetInputName(int8_t inputChannel) {
        return mDefaultInStrings[inputChannel];
    }

    std::string_view NodeGraphOp::GetOutputName(int8_t outputChannel) {
        return mDefaultOutStrings[outputChannel];
    }

    std::string_view NodeGraphOp::GetName() {
        return mName;
    }

    void NodeGraphOp::AddInput(std::string_view name, float defaultValue, int32_t size, float boundMin, float boundMax, bool visible, bool editable) {
        mNumInputs++;
        mDefaultInStrings.push_back(std::string(name));
        mDefaultInData.push_back({ defaultValue, size, boundMin, boundMax, false, visible, editable});
    }

    void NodeGraphOp::AddOutput(std::string_view name, float defaultValue, int32_t size) {
        mNumOutputs++;
        mDefaultOutStrings.push_back(std::string(name));
        mDefaultOutData.push_back({ defaultValue, size });
    }

    void NodeGraphOp::AddConstant(std::string_view name, float defaultValue, float boundMin, float boundMax, bool visible, bool editable) {
        mNumInputs++;
        mDefaultInStrings.push_back(std::string(name));
        mDefaultInData.push_back({ defaultValue, 1, boundMin, boundMax, true, visible, editable});
    }


}