#pragma once

#include "logging/LoggingAll.h"

#include <string>
#include <vector>
#include <map>
#include <typeinfo>
#include <type_traits>
#include <memory>

#include "math/MathConstants.h"

#include "nodegraph/core/NodeGraphInput.h"
#include "nodegraph/core/NodeGraphOutput.h"

namespace l::nodegraph {

    int32_t CreateUniqueId();
    bool IsValidInOutNum(int8_t inoutNum, size_t inoutSize);

    enum class DataType {
        FLOAT32,
        INT32,
        BITFIELD32
    };

    class NodeGraphGroup;

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

        virtual float& GetOutput(int8_t outputChannel, int32_t size = 1);
        virtual float GetInput(int8_t inputChannel);
        virtual float& GetInput(int8_t inputChannel, int32_t size);

        virtual int32_t GetOutputSize(int8_t outputChannel);

        virtual std::string_view GetName();
        virtual std::string_view GetInputName(int8_t inputChannel);
        virtual std::string_view GetOutputName(int8_t outputChannel);
        virtual void SetInputName(int8_t inputChannel, std::string_view name);
        virtual void SetOutputName(int8_t outputChannel, std::string_view name);

        virtual bool ClearInput(int8_t inputChannel);

        virtual bool SetInput(int8_t inputChannel, NodeGraphBase& source, int8_t sourceOutputChannel);
        virtual bool SetInput(int8_t inputChannel, NodeGraphGroup& source, int8_t sourceOutputChannel);
        virtual bool SetInput(int8_t inputChannel, float constant, int32_t size = -1);
        virtual bool SetInput(int8_t inputChannel, float* floatPtr);
        virtual void SetDefaultOutput(int8_t outputChannel, float constant, int32_t size = -1);

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

}

