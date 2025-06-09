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

    /**********************************************************************************/

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
            if (link.HasInputNode() && link.GetInputNode() != this) {
                link.GetInputNode()->Reset();
            }
        }
    }

    void NodeGraphBase::ClearProcessFlags() {
        mProcessUpdateHasRun = false;
        for (auto& link : mInputs) {
            if (link.HasInputNode() && link.GetInputNode() != this) {
                link.GetInputNode()->ClearProcessFlags();
            }
        }
    }

    void NodeGraphBase::ProcessSubGraph(int32_t numSamples, int32_t numCacheSamples, bool recomputeSubGraphCache) {
        if (recomputeSubGraphCache) {
            ClearProcessFlags();
        }
        ProcessOperation(numSamples, numCacheSamples);
    }

    void NodeGraphBase::ProcessOperation(int32_t numSamples, int32_t numCacheSamples) {
        if (mProcessUpdateHasRun) {
            return;
        }
        for (auto& link : mInputs) {
            if (link.HasInputNode() && link.GetInputNode() != this) {
                link.GetInputNode()->ProcessOperation(numSamples, numCacheSamples);
            }
        }
        mProcessUpdateHasRun = true;
    }

    void NodeGraphBase::Tick(int32_t tickCount, float delta) {
        if (tickCount <= mLastTickCount) {
            return;
        }
        for (auto& link : mInputs) {
            if (link.HasInputNode() && link.GetInputNode() != this) {
                link.GetInputNode()->Tick(tickCount, delta);
            }
        }
        mLastTickCount = tickCount;
    }

    float& NodeGraphBase::GetInput(int8_t inputChannel, int32_t minSize, int32_t offset) {
        return mInputs.at(inputChannel).Get(minSize, offset);
    }

    std::string_view NodeGraphBase::GetInputText(int8_t inputChannel, int32_t minSize) {
        return mInputs.at(inputChannel).GetText(minSize);
    }

    float& NodeGraphBase::GetOutput(int8_t outputChannel, int32_t minSize, int32_t offset) {
        return mOutputs.at(outputChannel).Get(minSize, offset);
    }

    std::string_view NodeGraphBase::GetOutputText(int8_t outputChannel, int32_t minSize) {
        return mOutputs.at(outputChannel).GetText(minSize);
    }

    NodeGraphInput& NodeGraphBase::GetInputOf(int8_t inputChannel) {
        return mInputs.at(inputChannel);
    }

    NodeGraphOutput& NodeGraphBase::GetOutputOf(int8_t outputChannel) {
        return mOutputs.at(outputChannel);
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
        input.Clear();
        return true;
    }

    void NodeGraphBase::ClearInputs() {
        for (int8_t i = 0; i < mInputs.size();i++) {
            ClearInput(i);
        }
    }

    bool NodeGraphBase::SetInput(int8_t inputChannel, NodeGraphBase& source, int8_t sourceOutputChannel) {
        ASSERT(inputChannel >= 0 && static_cast<size_t>(inputChannel) < mInputs.size());
        if (!IsValidInOutNum(inputChannel, mInputs.size())) {
            return false;
        }
        auto& input = mInputs.at(inputChannel);
        if (!input.SetInputNode(&source, sourceOutputChannel)) {
            return false;
        }
        return true;
    }

    bool NodeGraphBase::SetInput(int8_t inputChannel, NodeGraphGroup& source, int8_t sourceChannel) {
        ASSERT(inputChannel >= 0 && static_cast<size_t>(inputChannel) < mInputs.size());
        if (!IsValidInOutNum(inputChannel, mInputs.size())) {
            return false;
        }
        auto& input = mInputs.at(inputChannel);
        if (source.ContainsNode(GetId())) {
            if (!input.SetInputNode(&source.GetInputNode(), sourceChannel)) {
                return false;
            }
        }
        else {
            if (!input.SetInputNode(&source.GetOutputNode(), sourceChannel)) {
                return false;
            }
        }
        return true;
    }

    bool NodeGraphBase::SetInput(int8_t inputChannel, float initialValue, int32_t minSize) {
        ASSERT(inputChannel >= 0 && static_cast<size_t>(inputChannel) < mInputs.size());
        if (!IsValidInOutNum(inputChannel, mInputs.size())) {
            return false;
        }
        auto& input = mInputs.at(inputChannel);
        initialValue = l::math::clamp(initialValue, input.mBoundMin, input.mBoundMax);
        if (minSize <= 1) {
            input.SetConstant(initialValue);
        }
        else {
            input.SetArray(initialValue, minSize);
        }

        return true;
    }

    bool NodeGraphBase::SetInput(int8_t inputChannel, float* floatPtr) {
        ASSERT(inputChannel >= 0 && static_cast<size_t>(inputChannel) < mInputs.size());
        if (!IsValidInOutNum(inputChannel, mInputs.size())) {
            return false;
        }
        auto& input = mInputs.at(inputChannel);
        input.SetValue(floatPtr);
        return true;
    }

    bool NodeGraphBase::SetInput(int8_t inputChannel, std::string_view text) {
        ASSERT(inputChannel >= 0 && static_cast<size_t>(inputChannel) < mInputs.size());
        if (!IsValidInOutNum(inputChannel, mInputs.size())) {
            return false;
        }

        auto& input = mInputs.at(inputChannel);
        input.SetText(text);
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
            if (it.HasInputNode(reinterpret_cast<NodeGraphBase*>(source)) ||
                it.HasInputValue(reinterpret_cast<float*>(source))
                ) {
                it.Reset();
                sourceRemoved++;
            }
        }
        return sourceRemoved > 0 ? true : false;
    }

    bool NodeGraphBase::IsOutputPolled(int8_t outputChannel) {
        return mOutputs.at(outputChannel).IsPolled();
    }

    bool NodeGraphBase::IsOutOfDate() {
        return GetOperation()->HasInputChanged();
    }

    void NodeGraphBase::NodeHasChanged(int32_t samplesWritten) {
        GetOperation()->InputHasChanged(samplesWritten);
    }

    NodeType NodeGraphBase::GetOutputType() {
        return mOutputType;
    }

    /**********************************************************************************/

    void NodeGraphOp::DefaultDataInit() {
        for (int8_t i = 0; i < static_cast<int8_t>(mDefaultInData.size()); i++) {
            auto& e = mDefaultInData.at(i);
            auto& inputFlags = std::get<4>(e);
            if (inputFlags.mText) {
                mNode->SetInput(i, "");
            }
            else {
                mNode->SetInput(i, std::get<0>(e), std::get<1>(e));
                mNode->SetInputBound(i, l::nodegraph::InputBound::INPUT_CUSTOM, std::get<2>(e), std::get<3>(e));
            }
        }
        for (int8_t i = 0; i < static_cast<int8_t>(mDefaultOutData.size()); i++) {
            auto& e = mDefaultOutData.at(i);
            auto& outputFlags = std::get<2>(e);
            if (outputFlags.mText) {
                auto output = mNode->GetOutputText(i, 1);
                *(const_cast<char*>(output.data())) = 0;
            }
            else {
                mNode->SetDefaultOutput(i, std::get<0>(e), std::get<1>(e));
            }
        }
    }

    int8_t NodeGraphOp::GetNumInputs() {
        return mNumInputs;
    }

    int8_t NodeGraphOp::GetNumOutputs() {
        return mNumOutputs;
    }

    void NodeGraphOp::InputHasChanged(int32_t) {
        mInputHasChanged = true;
    }

    bool NodeGraphOp::HasInputChanged() {
        return mInputHasChanged;
    }

    bool NodeGraphOp::IsInputDataConstant(int8_t channel) {
        if (static_cast<size_t>(channel) < mDefaultInData.size()) {
            return std::get<4>(mDefaultInData.at(channel)).mConstant;
        }
        return false;
    }

    bool NodeGraphOp::IsInputDataVisible(int8_t channel) {
        if (static_cast<size_t>(channel) < mDefaultInData.size()) {
            return std::get<4>(mDefaultInData.at(channel)).mVisible;
        }
        return false;
    }

    bool NodeGraphOp::IsInputDataEditable(int8_t channel) {
        if (static_cast<size_t>(channel) < mDefaultInData.size()) {
            return std::get<4>(mDefaultInData.at(channel)).mEditable;
        }
        return false;
    }

    bool NodeGraphOp::IsInputDataText(int8_t channel) {
        if (static_cast<size_t>(channel) < mDefaultInData.size()) {
            return std::get<4>(mDefaultInData.at(channel)).mText;
        }
        return false;
    }

    bool NodeGraphOp::IsInputDataArray(int8_t channel) {
        if (static_cast<size_t>(channel) < mDefaultInData.size()) {
            return !std::get<4>(mDefaultInData.at(channel)).mText && std::get<1>(mDefaultInData.at(channel)) > 1;
        }
        return false;
    }

    bool NodeGraphOp::IsOutputDataVisible(int8_t channel) {
        if (static_cast<size_t>(channel) < mDefaultOutData.size()) {
            return std::get<2>(mDefaultOutData.at(channel)).mVisible;
        }
        return false;
    }

    std::string_view NodeGraphOp::GetInputName(int8_t inputChannel) {
        if (static_cast<size_t>(inputChannel) < mDefaultInStrings.size()) {
            return mDefaultInStrings[inputChannel];
        }
        return "";
    }

    std::string_view NodeGraphOp::GetOutputName(int8_t outputChannel) {
        if (static_cast<size_t>(outputChannel) < mDefaultOutStrings.size()) {
            return mDefaultOutStrings[outputChannel];
        }
        return "";
    }

    std::string_view NodeGraphOp::GetName() {
        return mName;
    }

    std::string_view NodeGraphOp::GetTypeName() {
        return mTypeName;
    }

    float NodeGraphOp::GetDefaultData(int8_t inputChannel) {
        if (static_cast<size_t>(inputChannel) < mDefaultInData.size()) {
            return std::get<0>(mDefaultInData.at(inputChannel));
        }
        return 0.0f;
    }

    int32_t NodeGraphOp::AddInput(std::string_view name, float defaultValue, int32_t minSize, float boundMin, float boundMax, bool visible, bool editable) {
        InputFlags flags(false, visible, editable, false);
        mNumInputs++;
        mDefaultInStrings.push_back(std::string(name));
        mDefaultInData.push_back({ defaultValue, minSize, boundMin, boundMax, flags });
        return static_cast<int32_t>(mDefaultInData.size() - 1);
    }

    int32_t NodeGraphOp::AddOutput(std::string_view name, float defaultValue, int32_t minSize, bool visible) {
        OutputFlags flags(visible, false);
        mNumOutputs++;
        mDefaultOutStrings.push_back(std::string(name));
        mDefaultOutData.push_back({ defaultValue, minSize, flags });
        return static_cast<int32_t>(mDefaultOutData.size() - 1);
    }

    int32_t NodeGraphOp::AddConstant(std::string_view name, float defaultValue, int32_t minSize, float boundMin, float boundMax, bool visible, bool editable) {
        InputFlags flags(true, visible, editable, false);
        mNumInputs++;
        mDefaultInStrings.push_back(std::string(name));
        mDefaultInData.push_back({ defaultValue, minSize, boundMin, boundMax, flags });
        return static_cast<int32_t>(mDefaultInData.size() - 1);
    }

    int32_t NodeGraphOp::AddInput2(std::string_view name, int32_t minSize, InputFlags flags) {
        mNumInputs++;
        mDefaultInStrings.push_back(std::string(name));
        mDefaultInData.push_back({ 0.0f, minSize, -l::math::constants::FLTMAX, l::math::constants::FLTMAX, flags });
        return static_cast<int32_t>(mDefaultInData.size() - 1);
    }

    int32_t NodeGraphOp::AddOutput2(std::string_view name, int32_t minSize, OutputFlags flags) {
        mNumOutputs++;
        mDefaultOutStrings.push_back(std::string(name));
        mDefaultOutData.push_back({ 0.0f, minSize, flags });
        return static_cast<int32_t>(mDefaultOutData.size() - 1);
    }

    /**********************************************************************************/

    int32_t InputManager::AddInput(InputIterationType type, int32_t inputIndex) {
        inputIndex = static_cast<int32_t>(mInputs.size());
        mInputs.emplace_back(InputAccessor{ type, inputIndex });
        mInputs.back().SetValue(mNodeGraphOperation.GetDefaultData(static_cast<int8_t>(inputIndex)));
        mInputs.back().SetTarget(mNodeGraphOperation.GetDefaultData(static_cast<int8_t>(inputIndex)));
        return inputIndex;
    }

    int32_t InputManager::AddCustom(InputIterationType type) {
        auto inputIndex = static_cast<int32_t>(mCustom.size());
        mCustom.emplace_back(InputAccessor{ type, inputIndex });
        return inputIndex + gCustomIndexBase;
    }

    void InputManager::BatchUpdate(std::vector<NodeGraphInput>& inputs, int32_t numSamples) {
        for (auto& input : mInputs) {
            input.BatchUpdate(inputs, numSamples);
        }
        for (auto& custom : mCustom) {
            custom.BatchUpdate(inputs, numSamples);
        }
    }

    void InputManager::NodeUpdate(std::vector<NodeGraphInput>& inputs, float updateRate) {
        for (auto& input : mInputs) {
            input.NodeUpdate(inputs, updateRate);
        }
        for (auto& custom : mCustom) {
            custom.NodeUpdate(inputs, updateRate);
        }
    }

    float InputManager::GetValueNext(int32_t inputIndex) {
        if (inputIndex < gCustomIndexBase) {
            return mInputs.at(inputIndex).GetValueNext();
        }
        return mCustom.at(inputIndex - gCustomIndexBase).GetValueNext();
    }

    float InputManager::GetValue(int32_t inputIndex) {
        if (inputIndex < gCustomIndexBase) {
            return mInputs.at(inputIndex).GetValue();
        }
        return mCustom.at(inputIndex - gCustomIndexBase).GetValue();
    }

    float InputManager::GetArrayValue(int32_t inputIndex, int32_t arrayIndex) {
        if (inputIndex < gCustomIndexBase) {
            return mInputs.at(inputIndex).GetArrayValue(arrayIndex);
        }
        return mCustom.at(inputIndex - gCustomIndexBase).GetArrayValue(arrayIndex);
    }

    float* InputManager::GetArray(int32_t inputIndex) {
        if (inputIndex < gCustomIndexBase) {
            return mInputs.at(inputIndex).GetArray();
        }
        return mCustom.at(inputIndex - gCustomIndexBase).GetArray();
    }

    void InputManager::SetUpdateRate(int32_t inputIndex, float updateRate) {
        if (inputIndex < gCustomIndexBase) {
            mInputs.at(inputIndex).SetUpdateRate(updateRate);
            return;
        }
        mCustom.at(inputIndex - gCustomIndexBase).SetUpdateRate(updateRate);
    }

    void InputManager::SetDuration(int32_t inputIndex, float value, float limit) {
        if (inputIndex < gCustomIndexBase) {
            mInputs.at(inputIndex).SetDuration(value, limit);
            return;
        }
        mCustom.at(inputIndex - gCustomIndexBase).SetDuration(value, limit);
    }

    void InputManager::SetTarget(int32_t inputIndex, float value) {
        if (inputIndex < gCustomIndexBase) {
            mInputs.at(inputIndex).SetTarget(value);
            return;
        }
        mCustom.at(inputIndex - gCustomIndexBase).SetTarget(value);
    }

    void InputManager::SetValue(int32_t inputIndex, float value) {
        if (inputIndex < gCustomIndexBase) {
            mInputs.at(inputIndex).SetValue(value);
            return;
        }
        mCustom.at(inputIndex - gCustomIndexBase).SetValue(value);
    }

    /**********************************************************************************/

    void NodeGraphOp2::Tick(int32_t, float) {

    }

    int32_t NodeGraphOp2::AddInput2(
        InputIterationType type,
        std::string_view name,
        float defaultValue,
        int32_t size,
        float boundMin,
        float boundMax,
        bool visible,
        bool editable) {
        return mInputManager.AddInput(type, NodeGraphOp::AddInput(name, defaultValue, size, boundMin, boundMax, visible, editable));
    }

    int32_t NodeGraphOp2::AddConstant2(
        InputIterationType type,
        std::string_view name,
        float defaultValue,
        int32_t size,
        float boundMin,
        float boundMax,
        bool visible,
        bool editable) {
        return mInputManager.AddInput(type, NodeGraphOp::AddConstant(name, defaultValue, size, boundMin, boundMax, visible, editable));
    }

    int32_t NodeGraphOp2::AddCustom2(InputIterationType type) {
        return mInputManager.AddCustom(type);
    }

}