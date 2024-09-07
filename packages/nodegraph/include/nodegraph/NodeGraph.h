#pragma once

#include "logging/LoggingAll.h"

#include <string>
#include <vector>
#include <map>
#include <typeinfo>
#include <type_traits>
#include <memory>

#include "math/MathConstants.h"

namespace l::nodegraph {

    int32_t CreateUniqueId();

    enum class DataType {
        FLOAT32,
        INT32,
        BITFIELD32
    };

    enum class InputType {
        INPUT_EMPTY,
        INPUT_NODE,
        INPUT_CONSTANT,
        INPUT_VALUE,
        //INPUT_ARRAY // TODO: is it possible  to process batches for example for audio processing?
    };

    enum class InputBound {
        INPUT_DONTCHANGE,
        INPUT_UNBOUNDED,
        INPUT_0_TO_1,
        INPUT_NEG_1_POS_1,
        INPUT_0_100,
        INPUT_CUSTOM,
    };

    enum class OutputType {
        Default, // node will be processed if it is connected to the groups output by some route
        ExternalOutput, // node does not have meaningful output for other nodes but should still be processed (ex speaker output only has input)
        ExternalVisualOutput,
    };

    bool IsValidInOutNum(int8_t inoutNum, size_t inoutSize);

    class NodeGraphOutput {
    public:
        NodeGraphOutput() = default;

        float mOutput = 0.0f;
        std::unique_ptr<std::vector<float>> mOutputBuf = nullptr;
        std::unique_ptr<std::string> mName = nullptr;
        bool mOutputPolled = false;

        float& GetOutput(int32_t numSamples = 1) {
            if (!mOutputBuf) {
                if (numSamples <= 1) {
                    mOutputPolled = true;
                    return mOutput;
                }
                else {
                    mOutputBuf = std::make_unique<std::vector<float>>();
                }
            }
            if (static_cast<int32_t>(mOutputBuf->size()) < numSamples) {
                mOutputBuf->resize(numSamples);
            }
            mOutputPolled = true;
            return *mOutputBuf->data();
        }

        int32_t GetOutputSize() {
            if (!mOutputBuf) {
                return 1;
            }
            else {
                return static_cast<int32_t>(mOutputBuf->size());
            }
        }

        bool IsOutputPolled() {
            return mOutputPolled;
        }

        void ResetOutputPollState() {
            mOutputPolled = false;
        }
    };

    class NodeGraphBase;
    class NodeGraphGroup;

    union Input {
        NodeGraphBase* mInputNode = nullptr;
        float* mInputFloat;
        float mInputFloatConstant;
        int32_t* mInputInt;
        int32_t mInputIntConstant;
    };

    struct NodeGraphInput {
        Input mInput;
        InputType mInputType = InputType::INPUT_EMPTY;

        float mBoundMin = -l::math::constants::FLTMAX;
        float mBoundMax = l::math::constants::FLTMAX;
        InputBound mInputBound = InputBound::INPUT_UNBOUNDED;

        int8_t mInputFromOutputChannel = 0;
        std::unique_ptr<std::string> mName;

        void Reset();
        bool HasInput();
        float Get();
        float& Get(int32_t numSamples);
    };

    class NodeGraphBase {
    public:
        NodeGraphBase(OutputType outputType) : mId(CreateUniqueId()), mOutputType(outputType) {
            mInputs.resize(1);
            mOutputs.resize(1);
        }
        virtual ~NodeGraphBase() {
            LOG(LogInfo) << "Node graph base destroyed";
        }

        virtual void Reset();
        virtual void SetId(int32_t id) { mId = id; }
        virtual int32_t GetId() const { return mId; }

        void ClearProcessFlags();
        virtual void ProcessSubGraph(int32_t numSamples = 1, bool recomputeSubGraphCache = true);
        virtual void Tick(int32_t tickCount, float elapsed);

        virtual void SetNumInputs(int8_t numInputs);
        virtual void SetNumOutputs(int8_t outputCount);
        virtual void SetNumConstants(int8_t numConstants);

        virtual int8_t GetNumInputs();
        virtual int8_t GetNumOutputs();
        virtual int8_t GetNumConstants();

        virtual float& GetOutput(int8_t outputChannel, int32_t numSamples = 1);
        virtual float GetInput(int8_t inputChannel);

        virtual int32_t GetOutputSize(int8_t outputChannel);

        virtual std::string_view GetName();
        virtual std::string_view GetInputName(int8_t inputChannel);
        virtual std::string_view GetOutputName(int8_t outputChannel);
        virtual void SetInputName(int8_t inputChannel, std::string_view name);
        virtual void SetOutputName(int8_t outputChannel, std::string_view name);

        virtual bool ClearInput(int8_t inputChannel);

        virtual bool SetInput(int8_t inputChannel, NodeGraphBase& source, int8_t sourceOutputChannel);
        virtual bool SetInput(int8_t inputChannel, NodeGraphGroup& source, int8_t sourceOutputChannel);
        virtual bool SetInput(int8_t inputChannel, float constant);
        virtual bool SetInput(int8_t inputChannel, float* floatPtr);

        virtual bool SetInputBound(int8_t inputChannel, InputBound bound = InputBound::INPUT_DONTCHANGE, float boundMin = 0.0f, float boundMax = 0.0f);
        virtual bool RemoveInput(void* source);

        virtual bool IsDataVisible(int8_t num);
        virtual bool IsDataEditable(int8_t num);
        virtual bool IsOutputPolled(int8_t outputChannel);

        virtual OutputType GetOutputType();

    protected:
        virtual void ProcessOperation(int32_t numSamples = 1);

        bool mProcessUpdateHasRun = false;
        int32_t mLastTickCount = 0;
        std::vector<NodeGraphInput> mInputs;
        std::vector<NodeGraphOutput> mOutputs;

        int32_t mId = -1;
        OutputType mOutputType;

        std::string mName;
        int8_t mInputCount = 0;
        int8_t mConstantCount = 0;
    };

    class NodeGraphOp {
    public:
        NodeGraphOp(NodeGraphBase* node, int32_t numInputs = 1, int32_t numOutputs = 1, int32_t numConstants = 0) :
            mNode(node),
            mNumInputs(static_cast<int8_t>(numInputs)), 
            mNumOutputs(static_cast<int8_t>(numOutputs)),
            mNumConstants(static_cast<int8_t>(numConstants))
        {}
        virtual ~NodeGraphOp() {
            LOG(LogInfo) << "Node operation destroyed";
        }

        std::string defaultInStrings[4] = { "In 1", "In 2", "In 3", "In 4" };
        std::string defaultOutStrings[4] = { "Out 1", "Out 2", "Out 3", "Out 4" };

        virtual void Reset() {}
        virtual void Process(int32_t, std::vector<NodeGraphInput>&, std::vector<NodeGraphOutput>&) {};
        virtual void Tick(int32_t, float) {}

        virtual void SetNumInputs(int8_t numInputs);
        virtual void SetNumOutputs(int8_t numOutputs);
        virtual void SetNumConstants(int8_t numConstants);
        int8_t GetNumInputs();
        int8_t GetNumOutputs();
        int8_t GetNumConstants();

        virtual bool IsDataVisible(int8_t num);
        virtual bool IsDataEditable(int8_t num);
        virtual std::string_view GetInputName(int8_t inputChannel);
        virtual std::string_view GetOutputName(int8_t outputChannel);
        virtual std::string_view GetName();

    protected:
        NodeGraphBase* mNode;
        int8_t mNumInputs = 0;
        int8_t mNumOutputs = 0;
        int8_t mNumConstants = 0;
    };

    class GraphDataCopy : public NodeGraphOp {
    public:
        GraphDataCopy(NodeGraphBase* node) :
            NodeGraphOp(node, 0)
        {}
        virtual ~GraphDataCopy() = default;

        void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
    };

    template<class T, class... Params>
    class NodeGraph : public NodeGraphBase {
    public:
        NodeGraph(OutputType outputType = OutputType::Default, Params&&... params) :
            NodeGraphBase(outputType),
            mOperation(this, std::forward<Params>(params)...)
        {
            SetNumInputs(mOperation.GetNumInputs());
            SetNumOutputs(mOperation.GetNumOutputs());
            SetNumConstants(mOperation.GetNumConstants());
        }
        virtual ~NodeGraph() {
            LOG(LogInfo) << "Node destroyed";
        }

        virtual void SetNumInputs(int8_t numInputs) {
            NodeGraphBase::SetNumInputs(numInputs);
            mOperation.SetNumInputs(numInputs);
        }

        virtual void SetNumOutputs(int8_t numOutputs) {
            NodeGraphBase::SetNumOutputs(numOutputs);
            mOperation.SetNumOutputs(numOutputs);
        }

        virtual void SetNumConstants(int8_t numConstants) {
            NodeGraphBase::SetNumConstants(numConstants);
            mOperation.SetNumConstants(numConstants);
            for (int8_t i = mInputCount; i < mInputCount + mConstantCount; i++) {
                SetInput(i, 0.0f);
            }
        }

        virtual bool IsDataVisible(int8_t num) override {
            return mOperation.IsDataVisible(num);
        }

        virtual bool IsDataEditable(int8_t num) override {
            return mOperation.IsDataEditable(num);
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

    class NodeGraphGroup {
    public:
        NodeGraphGroup() :
            mInputNode(OutputType::Default),
            mOutputNode(OutputType::Default)
        {
            SetNumInputs(1);
            SetNumOutputs(1);
            mOutputNodes.push_back(&mOutputNode);
        }
        ~NodeGraphGroup() {
            LOG(LogInfo) << "Node group destroyed";
        }

        void SetNumInputs(int8_t numInputs);
        void SetNumOutputs(int8_t outputCount);
        void SetInput(int8_t inputChannel, NodeGraphBase& source, int8_t sourceOutputChannel);
        void SetInput(int8_t inputChannel, NodeGraphGroup& source, int8_t sourceOutputChannel);
        void SetInput(int8_t inputChannel, float constant);
        void SetInput(int8_t inputChannel, float* floatPtr);

        void SetOutput(int8_t outputChannel, NodeGraphBase& source, int8_t sourceOutputChannel);
        void SetOutput(int8_t outputChannel, NodeGraphGroup& source, int8_t sourceOutputChannel);

        float GetOutput(int8_t outputChannel);
        NodeGraphBase& GetInputNode();
        NodeGraphBase& GetOutputNode();

        bool ContainsNode(int32_t id);
        NodeGraphBase* GetNode(int32_t id);

        template<class T, class U = void, class = std::enable_if_t<std::is_base_of_v<NodeGraphOp, T>>>
        NodeGraph<T, U>* GetTypedNode(int32_t id) {
            auto p = GetNode(id);
            return reinterpret_cast<NodeGraph<T, U>*>(p);
        }

        bool RemoveNode(int32_t id);

        template<class T, class = std::enable_if_t<std::is_base_of_v<NodeGraphOp, T>>, class... Params>
        l::nodegraph::NodeGraphBase* NewNode(OutputType nodeType, Params&&... params) {
            mNodes.push_back(std::make_unique<l::nodegraph::NodeGraph<T, Params...>>(nodeType, std::forward<Params>(params)...));
            auto nodePtr = mNodes.back().get();
            if (nodeType == OutputType::ExternalOutput || nodeType == OutputType::ExternalVisualOutput) {
                mOutputNodes.push_back(nodePtr);
            }
            return nodePtr;
        }

        void ClearProcessFlags();
        void ProcessSubGraph(int32_t numSamples, bool recomputeSubGraphCache = true);
        void Tick(int32_t tickCount, float elapsed);
    protected:
        NodeGraph<GraphDataCopy> mInputNode;
        NodeGraph<GraphDataCopy> mOutputNode;

        std::vector<std::unique_ptr<NodeGraphBase>> mNodes;
        std::vector<NodeGraphBase*> mOutputNodes;

        int32_t mLastTickCount = 0;
    };

}

