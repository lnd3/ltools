#pragma once

#include "logging/LoggingAll.h"
#include "meta/Reflection.h"

#include <string>
#include <vector>
#include <map>
#include <typeinfo>
#include <type_traits>
#include <memory>

#include "math/MathConstants.h"

#include "nodegraph/core/NodeGraphData.h"
#include "nodegraph/core/NodeGraphInput.h"
#include "nodegraph/core/NodeGraphOutput.h"

namespace l::nodegraph {

    int32_t CreateUniqueId();
    bool IsValidInOutNum(int8_t inoutNum, size_t inoutSize);

    class NodeGraphGroup;


    /**********************************************************************************/
    class NodeGraphBase {
    public:
        NodeGraphBase(NodeType outputType) : 
            mId(CreateUniqueId()), 
            mOutputType(outputType),
			mOperationTypeHash(l::meta::class_hash<NodeGraphBase>())
        {}

        virtual ~NodeGraphBase() {
            LOG(LogInfo) << "Node graph base destroyed";
        }

        NodeGraphBase& operator=(NodeGraphBase&& other) noexcept {
            this->mId = other.mId;
            //this->mInputs = std::move(other.mInputs);
            //this->mOutputs = std::move(other.mOutputs);
            this->mName = std::move(other.mName);
            this->mLastTickCount = other.mLastTickCount;
            this->mOutputType = other.mOutputType;
            this->mProcessUpdateHasRun = other.mProcessUpdateHasRun;
            this->mOperationTypeHash = other.mOperationTypeHash;
            return *this;
        }
        NodeGraphBase& operator=(const NodeGraphBase& other) noexcept {
            this->mId = other.mId;
            //this->mInputs = std::move(other.mInputs);
            //this->mOutputs = std::move(other.mOutputs);
            this->mName = std::move(other.mName);
            this->mLastTickCount = other.mLastTickCount;
            this->mOutputType = other.mOutputType;
            this->mProcessUpdateHasRun = other.mProcessUpdateHasRun;
            this->mOperationTypeHash = other.mOperationTypeHash;
            return *this;
        }
        NodeGraphBase(NodeGraphBase&& other) noexcept {
            *this = std::move(other);
        }
        NodeGraphBase(const NodeGraphBase& other) noexcept {
            *this = other;
        }

        virtual void Reset();
        virtual void DefaultDataInit() {};
        virtual void SetId(int32_t id) { mId = id; }
        virtual int32_t GetId() const { return mId; }

        void ClearProcessFlags();
        virtual void ProcessSubGraph(int32_t numSamples = 1, bool recomputeSubGraphCache = true);
        virtual void Tick(int32_t tickCount, float elapsed);

        virtual int8_t GetNumInputs();
        virtual int8_t GetNumOutputs();

        virtual float& GetInput(int8_t inputChannel, int32_t size = 1);
        virtual float& GetOutput(int8_t outputChannel, int32_t size = 1);
        virtual NodeGraphInput& GetInputOf(int8_t inputChannel);
        virtual NodeGraphOutput& GetOutputOf(int8_t outputChannel);

        virtual int32_t GetInputSize(int8_t inputChannel);
        virtual int32_t GetOutputSize(int8_t outputChannel);

        virtual std::string_view GetName();
        virtual std::string_view GetInputName(int8_t inputChannel);
        virtual std::string_view GetOutputName(int8_t outputChannel);
        virtual void SetInputName(int8_t inputChannel, std::string_view name);
        virtual void SetOutputName(int8_t outputChannel, std::string_view name);

        virtual bool ClearInput(int8_t inputChannel);

        virtual bool SetInput(int8_t inputChannel, NodeGraphBase& source, int8_t sourceOutputChannel);
        virtual bool SetInput(int8_t inputChannel, NodeGraphGroup& source, int8_t sourceOutputChannel);
        virtual bool SetInput(int8_t inputChannel, float initialValue, int32_t size = 1);
        virtual bool SetInput(int8_t inputChannel, float* floatPtr);
        virtual void SetDefaultOutput(int8_t outputChannel, float constant, int32_t size = 1);

        virtual bool SetInputBound(int8_t inputChannel, InputBound bound = InputBound::INPUT_0_TO_1, float boundMin = 0.0f, float boundMax = 1.0f);
        virtual bool DetachInput(void* source);

        virtual bool IsDataConstant(int8_t num);
        virtual bool IsDataVisible(int8_t num);
        virtual bool IsDataEditable(int8_t num);
        virtual bool IsOutputPolled(int8_t outputChannel);

        virtual NodeType GetOutputType();

        template<class T>
		bool IsOfOperation() {
			return l::meta::class_hash<T>() == mOperationTypeHash;
		}

    protected:
        virtual void SetNumInputs(int8_t numInputs);
        virtual void SetNumOutputs(int8_t outputCount);

        virtual void ProcessOperation(int32_t numSamples = 1);

        bool mProcessUpdateHasRun = false;
        int32_t mLastTickCount = 0;
        std::vector<NodeGraphInput> mInputs;
        std::vector<NodeGraphOutput> mOutputs;

        int32_t mId = -1;
        NodeType mOutputType;

        std::string mName;
        size_t mOperationTypeHash;
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

        virtual void DefaultDataInit();
        virtual void Reset() {};
        virtual void Process(int32_t, std::vector<NodeGraphInput>&, std::vector<NodeGraphOutput>&) {};
        virtual void Tick(int32_t, float) {}

        int8_t GetNumInputs();
        int8_t GetNumOutputs();

        virtual bool IsDataConstant(int8_t channel);
        virtual bool IsDataVisible(int8_t channel);
        virtual bool IsDataEditable(int8_t channel);
        virtual std::string_view GetInputName(int8_t inputChannel);
        virtual std::string_view GetOutputName(int8_t outputChannel);
        virtual std::string_view GetName();
        virtual float GetDefaultData(int8_t inputChannel);

    protected:
        virtual int32_t AddInput(std::string_view name, float defaultValue = 0.0f, int32_t size = 1, float boundMin = -l::math::constants::FLTMAX, float boundMax = l::math::constants::FLTMAX, bool visible = true, bool editable = true);
        virtual int32_t AddOutput(std::string_view name, float defaultValue = 0.0f, int32_t size = 1);
        virtual int32_t AddConstant(std::string_view name, float defaultValue = 0.0f, int32_t size = 1, float boundMin = -l::math::constants::FLTMAX, float boundMax = l::math::constants::FLTMAX, bool visible = true, bool editable = true);

        NodeGraphBase* mNode;
        std::string mName;

        std::vector<std::string> mDefaultInStrings;
        std::vector<std::string> mDefaultOutStrings;
        std::vector<std::tuple<float, int32_t, float, float, bool, bool, bool>> mDefaultInData;
        std::vector<std::tuple<float, int32_t>> mDefaultOutData;

        int8_t mNumInputs = 0;
        int8_t mNumOutputs = 0;
    };

    /**********************************************************************************/

    template<class T, class... Params>
    class NodeGraph : public NodeGraphBase {
    public:
        NodeGraph(NodeType outputType = NodeType::Default, Params&&... params) :
            NodeGraphBase(outputType),
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

        virtual bool IsDataConstant(int8_t num) override {
            return mOperation.IsDataConstant(num);
        }

        virtual bool IsDataVisible(int8_t num) override {
            return mOperation.IsDataVisible(num);
        }

        virtual bool IsDataEditable(int8_t num) override {
            return mOperation.IsDataEditable(num);
        }

        virtual void DefaultDataInit() override {
            NodeGraphBase::DefaultDataInit();
            mOperation.DefaultDataInit();
        }

        virtual void Reset() override {
            NodeGraphBase::Reset();
            mOperation.Reset();
        }

        virtual void ProcessOperation(int32_t numSamples = 1) override {
            if (mProcessUpdateHasRun) {
                return;
            }

            NodeGraphBase::ProcessOperation(numSamples);
            mOperation.Process(numSamples, mInputs, mOutputs);

            mProcessUpdateHasRun = true;
        }

        virtual void Tick(int32_t tickCount, float elapsed) override {
            if (tickCount <= mLastTickCount) {
                return;
            }
            NodeGraphBase::Tick(tickCount, elapsed);
            mOperation.Tick(tickCount, elapsed);
            mLastTickCount = tickCount;
        }

        virtual std::string_view GetInputName(int8_t inputChannel) {
            auto& customName = mInputs.at(inputChannel).mName;
            if (customName && !customName->empty()) {
                return *customName;
            }
            return mOperation.GetInputName(inputChannel);
        }

        virtual std::string_view GetOutputName(int8_t outputChannel) {
            auto& customName = mOutputs.at(outputChannel).mName;
            if (customName && !customName->empty()) {
                return *customName;
            }
            return mOperation.GetOutputName(outputChannel);
        }

        virtual std::string_view GetName() {
            return mOperation.GetName();
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
        ~InputManager() = default;

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

        int32_t AddInput2(
            InputIterationType type,
            std::string_view name, 
            float defaultValue = 0.0f, 
            int32_t size = 1, 
            float boundMin = -l::math::constants::FLTMAX, 
            float boundMax = l::math::constants::FLTMAX, 
            bool visible = true, 
            bool editable = true);

        int32_t AddConstant2(
            InputIterationType type,
            std::string_view name, 
            float defaultValue = 0.0f, 
            int32_t size = 1, 
            float boundMin = -l::math::constants::FLTMAX, 
            float boundMax = l::math::constants::FLTMAX, 
            bool visible = true, 
            bool editable = true);

        int32_t AddCustom2(
            InputIterationType type);

    protected:
        InputManager mInputManager;
        float mStepsUntilUpdate = 0.0f;
        float mUpdateRate = 16.0f;
    };
}

