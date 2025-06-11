#pragma once

#include <logging/LoggingAll.h>
#include <meta/Reflection.h>

#include <jsonxx/jsonxx.h>

#include <string>
#include <vector>
#include <map>
#include <typeinfo>
#include <type_traits>
#include <memory>

#include <math/MathConstants.h>

#include <nodegraph/core/NodeGraphData.h>
#include <nodegraph/core/NodeGraphInput.h>
#include <nodegraph/core/NodeGraphOutput.h>

namespace l::nodegraph {
    bool IsValidInOutNum(int8_t inoutNum, size_t inoutSize);

    class NodeGraphGroup;
    class NodeGraphOp;

    /**********************************************************************************/
    class NodeGraphBase {
    public:
        NodeGraphBase(int32_t id = -1, NodeType outputType = NodeType::Default) :
            mId(id),
            mOutputType(outputType),
            mTypeId(-1),
            mOperationTypeHash(l::meta::class_hash<NodeGraphBase>()),
            mProcessUpdateHasRun(false),
            mLastTickCount(0)
        {}

        virtual ~NodeGraphBase() {
            for (auto& in : mInputs) {
                in.Reset();
            }
            for (auto& out : mOutputs) {
                out.Reset();
            }
            mInputs.clear();
            mOutputs.clear();

            LOG(LogInfo) << "Node graph base destroyed";
        }

        NodeGraphBase& operator=(NodeGraphBase&& other) noexcept {
            mId = other.mId;
            mOutputType = other.mOutputType;

            mTypeId = other.mTypeId;
            mOperationTypeHash = other.mOperationTypeHash;

            mInputs = other.mInputs;
            mOutputs = other.mOutputs;

            mProcessUpdateHasRun = other.mProcessUpdateHasRun;
            mLastTickCount = other.mLastTickCount;

            return *this;
        }
        NodeGraphBase(NodeGraphBase&& other) noexcept {
            *this = std::move(other);
        }
        NodeGraphBase& operator=(const NodeGraphBase&) = delete;
        NodeGraphBase(const NodeGraphBase&) = delete;

        virtual void Reset();
        virtual void DefaultDataInit() {};
        virtual void SetId(int32_t id) { mId = id; }
        virtual void SetTypeId(int32_t typeId) { mTypeId = typeId; }
        virtual int32_t GetId() const { return mId; }
        virtual int32_t GetTypeId() const { return mTypeId; }


        void ClearProcessFlags();
        virtual void ProcessSubGraph(int32_t numSamples = 1, int32_t numCacheSamples = -1, bool recomputeSubGraphCache = true);
        virtual void Tick(int32_t tickCount, float delta);

        virtual int8_t GetNumInputs();
        virtual int8_t GetNumOutputs();

        virtual float& GetInput(int8_t inputChannel, int32_t minSize = 1, int32_t offset = 0);
        virtual std::string_view GetInputText(int8_t inputChannel, int32_t minSize = 16);

        virtual float& GetOutput(int8_t outputChannel, int32_t minSize = 1, int32_t offset = 0);
        virtual std::string_view GetOutputText(int8_t outputChannel, int32_t minSize);
        virtual NodeGraphInput& GetInputOf(int8_t inputChannel);
        virtual NodeGraphOutput& GetOutputOf(int8_t outputChannel);

        virtual int32_t GetInputSize(int8_t inputChannel);
        virtual int32_t GetOutputSize(int8_t outputChannel);

        virtual std::string_view GetName() = 0;
        virtual std::string_view GetTypeName() = 0;

        virtual std::string_view GetInputName(int8_t inputChannel) = 0;
        virtual std::string_view GetOutputName(int8_t outputChannel) = 0;
        //virtual void SetInputName(int8_t inputChannel, std::string_view name) = 0;
        //virtual void SetOutputName(int8_t outputChannel, std::string_view name) = 0;

        virtual bool ClearInput(int8_t inputChannel);
        void ClearInputs();

        virtual bool SetInput(int8_t inputChannel, NodeGraphBase& source, int8_t sourceOutputChannel);
        virtual bool SetInput(int8_t inputChannel, NodeGraphGroup& source, int8_t sourceOutputChannel);
        virtual bool SetInput(int8_t inputChannel, float initialValue, int32_t minSize = 1);
        virtual bool SetInput(int8_t inputChannel, float* floatPtr);
        virtual bool SetInput(int8_t inputChannel, std::string_view text);

        virtual void SetDefaultOutput(int8_t outputChannel, float constant, int32_t minSize = 1);

        virtual bool SetInputBound(int8_t inputChannel, InputBound bound = InputBound::INPUT_0_TO_1, float boundMin = 0.0f, float boundMax = 1.0f);
        virtual bool DetachInput(void* source);

        virtual bool IsInputDataConstant(int8_t) { return false; }
        virtual bool IsInputDataVisible(int8_t) { return false; }
        virtual bool IsInputDataEditable(int8_t) { return false; }
        virtual bool IsInputDataText(int8_t) { return false; }
        virtual bool IsInputDataArray(int8_t) { return false; }
        virtual bool IsOutputDataVisible(int8_t) { return false; }
        virtual bool IsOutputPolled(int8_t outputChannel);
        virtual bool IsOutOfDate();
        virtual void NodeHasChanged(int32_t numSamplesWritten = 1);

        virtual NodeType GetOutputType();

        template<class T>
		bool IsOfOperation() {
			return l::meta::class_hash<T>() == mOperationTypeHash;
		}

        template<class T>
        T* GetOp() {
            if (l::meta::class_hash<T>() == mOperationTypeHash) {
                return reinterpret_cast<T*>(GetOperation());
            }
            return nullptr;
        }

        void ForEachInput(std::function<void(NodeGraphInput& input)> cb) {
            for (auto& in : mInputs) {
                cb(in);
            }
        }

    protected:
        virtual void SetNumInputs(int8_t numInputs);
        virtual void SetNumOutputs(int8_t outputCount);

        virtual void ProcessOperation(int32_t numSamples = 1, int32_t numCacheSamples = 0);
        virtual NodeGraphOp* GetOperation() = 0;


        int32_t mId = -1;
        NodeType mOutputType = NodeType::Default;

        int32_t mTypeId = -1;
        size_t mOperationTypeHash = 0;

        std::vector<NodeGraphInput> mInputs;
        std::vector<NodeGraphOutput> mOutputs;

        bool mProcessUpdateHasRun = false;
        int32_t mLastTickCount = 0;

        //NodeGraphUIData mUiData;
    };

    /**********************************************************************************/

    class NodeGraphOp {
    public:
        NodeGraphOp(NodeGraphBase* node, std::string_view name) :
            mNode(node),
            mName(name)
        {}
        virtual ~NodeGraphOp() {
            LOG(LogInfo) << "Node operation destroyed";
        }

        NodeGraphOp& operator=(NodeGraphOp&& other) noexcept {
            mNode = other.mNode;
            mName = other.mName;
            mTypeName = other.mTypeName;

            mDefaultInStrings = other.mDefaultInStrings;
            mDefaultOutStrings = other.mDefaultOutStrings;
            mDefaultInData = other.mDefaultInData;
            mDefaultOutData = other.mDefaultOutData;

            mNumInputs = other.mNumInputs;
            mNumOutputs = other.mNumOutputs;
            mInputHasChanged = other.mInputHasChanged;

            return *this;
        }
        NodeGraphOp(NodeGraphOp&& other) noexcept {
            *this = std::move(other);
        }
        NodeGraphOp& operator=(const NodeGraphOp&) = delete;
        NodeGraphOp(const NodeGraphOp&) = delete;

        virtual void DefaultDataInit();
        virtual void Reset() {};
        virtual void Process(int32_t, int32_t, std::vector<NodeGraphInput>&, std::vector<NodeGraphOutput>&) {};
        virtual void Tick(int32_t /*tickCount*/, float /*delta*/) {}
        virtual void InputHasChanged(int32_t numSamplesWritten = 1);

        int8_t GetNumInputs();
        int8_t GetNumOutputs();

        virtual bool HasInputChanged();
        virtual bool IsInputDataConstant(int8_t channel);
        virtual bool IsInputDataVisible(int8_t channel);
        virtual bool IsInputDataEditable(int8_t channel);
        virtual bool IsInputDataText(int8_t channel);
        virtual bool IsInputDataArray(int8_t channel);
        virtual bool IsOutputDataVisible(int8_t channel);

        virtual std::string_view GetInputName(int8_t inputChannel);
        virtual std::string_view GetOutputName(int8_t outputChannel);
        virtual std::string_view GetName();
        virtual std::string_view GetTypeName();
        virtual float GetDefaultData(int8_t inputChannel);


    protected:
        virtual int32_t AddInput(std::string_view name, float defaultValue = 0.0f, int32_t minSize = 1, float boundMin = -l::math::constants::FLTMAX, float boundMax = l::math::constants::FLTMAX, bool visible = true, bool editable = true);
        virtual int32_t AddOutput(std::string_view name, float defaultValue = 0.0f, int32_t minSize = 1, bool visible = true);
        virtual int32_t AddConstant(std::string_view name, float defaultValue = 0.0f, int32_t minSize = 1, float boundMin = -l::math::constants::FLTMAX, float boundMax = l::math::constants::FLTMAX, bool visible = true, bool editable = true);
        virtual int32_t AddInput2(std::string_view name, int32_t minSize, InputFlags flags);
        virtual int32_t AddOutput2(std::string_view name, int32_t minSize, OutputFlags flags);

        NodeGraphBase* mNode = nullptr;
        std::string mName;
        std::string mTypeName;

        std::vector<std::string> mDefaultInStrings;
        std::vector<std::string> mDefaultOutStrings;
        std::vector<std::tuple<float, int32_t, float, float, InputFlags>> mDefaultInData;
        std::vector<std::tuple<float, int32_t, OutputFlags>> mDefaultOutData;

        int8_t mNumInputs = 0;
        int8_t mNumOutputs = 0;
        bool mInputHasChanged = true;
    };

    /**********************************************************************************/

    template<class T, class... Params>
    class NodeGraph : public NodeGraphBase {
    public:
        NodeGraph(int32_t id = -1, NodeType outputType = NodeType::Default, Params&&... params) :
            NodeGraphBase(id, outputType),
            mOperation(this, std::forward<Params>(params)...)
        {
            mOperationTypeHash = l::meta::class_hash<T>();

            SetNumInputs(mOperation.GetNumInputs());
            SetNumOutputs(mOperation.GetNumOutputs());

            DefaultDataInit();
        }
        virtual ~NodeGraph() {
            LOG(LogInfo) << "Node destroyed";
        }

        NodeGraph& operator=(NodeGraph&& other) noexcept {
            mOperation = std::move(other.mOperation);
            return *this;
        }
        NodeGraph(NodeGraph&& other) noexcept {
            *this = std::move(other);
        }
        NodeGraph(const NodeGraph&) = delete;
        NodeGraph& operator=(const NodeGraph&) = delete;

        virtual bool IsInputDataConstant(int8_t num) override {
            return mOperation.IsInputDataConstant(num);
        }

        virtual bool IsInputDataVisible(int8_t num) override {
            return mOperation.IsInputDataVisible(num);
        }

        virtual bool IsInputDataEditable(int8_t num) override {
            return mOperation.IsInputDataEditable(num);
        }

        virtual bool IsInputDataText(int8_t channel) override {
            return mOperation.IsInputDataText(channel);
        }

        virtual bool IsInputDataArray(int8_t channel) override {
            auto& input = mInputs.at(channel);
            return input.IsOfType(InputType::INPUT_ARRAY);
        }

        virtual bool IsOutputDataVisible(int8_t num) override {
            return mOperation.IsOutputDataVisible(num);
        }

        virtual void DefaultDataInit() override {
            NodeGraphBase::DefaultDataInit();
            mOperation.DefaultDataInit();
        }

        virtual void Reset() override {
            NodeGraphBase::Reset();
            mOperation.Reset();
        }

        virtual void ProcessOperation(int32_t numSamples = 1, int32_t numCacheSamples = 0) override {
            if (mProcessUpdateHasRun) {
                return;
            }

            NodeGraphBase::ProcessOperation(numSamples, numCacheSamples);
            mOperation.Process(numSamples, numCacheSamples, mInputs, mOutputs);

            mProcessUpdateHasRun = true;
        }

        virtual void Tick(int32_t tickCount, float delta) override {
            if (tickCount <= mLastTickCount) {
                return;
            }
            NodeGraphBase::Tick(tickCount, delta);
            mOperation.Tick(tickCount, delta);
            mLastTickCount = tickCount;
        }

        virtual std::string_view GetInputName(int8_t inputChannel) override {
            return mOperation.GetInputName(inputChannel);
        }

        virtual std::string_view GetOutputName(int8_t outputChannel) override {
            return mOperation.GetOutputName(outputChannel);
        }

        virtual std::string_view GetName() override {
            return mOperation.GetName();
        }

        virtual std::string_view GetTypeName() override {
            return mOperation.GetTypeName();
        }

        virtual NodeGraphOp* GetOperation() override {
            return &mOperation;
        }

    protected:
        T mOperation;
    };


    class InputManager {
    public:
        const int32_t gCustomIndexBase = 100;

        InputManager(NodeGraphOp& nodeGraphOperation) :
            mNodeGraphOperation(nodeGraphOperation)
        {
        }
        ~InputManager() {
            mInputs.clear();
            mCustom.clear();
        }

        int32_t AddInput(InputIterationType type, int32_t inputIndex = -1);
        int32_t AddCustom(InputIterationType type);
        void BatchUpdate(std::vector<NodeGraphInput>& inputs, int32_t numSamples);
        void NodeUpdate(std::vector<NodeGraphInput>& inputs, float updateRate);
        float GetValueNext(int32_t inputIndex);
        float GetValue(int32_t inputIndex);
        float GetArrayValue(int32_t inputIndex, int32_t arrayIndex);
        float* GetArray(int32_t inputIndex);
        void SetUpdateRate(int32_t inputIndex, float updateRate);
        void SetDuration(int32_t inputIndex, float value, float limit = 0.001f);
        void SetTarget(int32_t inputIndex, float value);
        void SetValue(int32_t inputIndex, float value);

    protected:
        NodeGraphOp& mNodeGraphOperation;
        std::vector<InputAccessor> mInputs;
        std::vector<InputAccessor> mCustom;
    };

    // Use this when the operation requires dynamic input parameters or when the input data can be sampled scarcily (not every sample etc)
    class NodeGraphOp2 : public NodeGraphOp {
    public:
        NodeGraphOp2(NodeGraphBase* node, std::string_view name) :
            NodeGraphOp(node, name),
            mInputManager(*this) {

        }
        virtual ~NodeGraphOp2() {

        }

        virtual void Tick(int32_t, float) override;

        int32_t AddInput3(
            InputIterationType type,
            std::string_view name, 
            float defaultValue = 0.0f, 
            int32_t size = 1, 
            float boundMin = -l::math::constants::FLTMAX, 
            float boundMax = l::math::constants::FLTMAX, 
            bool visible = true, 
            bool editable = true);

        int32_t AddConstant3(
            InputIterationType type,
            std::string_view name, 
            float defaultValue = 0.0f, 
            int32_t size = 1, 
            float boundMin = -l::math::constants::FLTMAX, 
            float boundMax = l::math::constants::FLTMAX, 
            bool visible = true, 
            bool editable = true);

        int32_t AddCustom3(
            InputIterationType type);

    protected:
        InputManager mInputManager;
        float mStepsUntilUpdate = 0.0f;
        float mUpdateRate = 16.0f;
    };
}

