#pragma once

#include "logging/LoggingAll.h"

#include <string>
#include <vector>
#include <map>
#include <typeinfo>
#include <type_traits>
#include <memory>

namespace l::nodegraph {

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

    bool IsValidInOutNum(int8_t inoutNum, size_t inoutSize);

    struct NodeGraphOutput {
        float mOutput = 0.0f;
        std::string mName;
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
        int8_t mInputFromOutputChannel = 0;
        std::string mName;

        void Reset();
        bool HasInput();
        float Get();
    };

    class NodeGraphBase {
    public:
        NodeGraphBase(std::string_view name = "");
        virtual ~NodeGraphBase() = default;

        virtual void Reset();
        virtual void SetId(int32_t id) { mId = id; }
        virtual int32_t GetId() const { return mId; }

        virtual void Update();

        virtual void SetNumInputs(int8_t numInputs);
        virtual void SetNumOutputs(int8_t outputCount);

        virtual int8_t GetNumInputs();
        virtual int8_t GetNumOutputs();

        virtual float Get(int8_t outputChannel);

        virtual std::string_view GetName();
        virtual std::string_view GetInputName(int8_t inputChannel);
        virtual std::string_view GetOutputName(int8_t outputChannel);
        virtual void SetInputName(int8_t inputChannel, std::string_view name);
        virtual void SetOutputName(int8_t outputChannel, std::string_view name);

        virtual bool ClearInput(int8_t inputChannel);

        virtual bool SetInput(int8_t inputChannel, NodeGraphBase& source, int8_t sourceOutputChannel);
        virtual bool SetInput(int8_t inputChannel, NodeGraphGroup& source, int8_t sourceOutputChannel, bool nodeIsInsideGroup);
        virtual bool SetInput(int8_t inputChannel, float constant);
        virtual bool SetInput(int8_t inputChannel, float* floatPtr);

        virtual bool RemoveSource(void* source);
    protected:
        void PreUpdate();
        virtual void ProcessOperation();

        bool mProcessUpdateHasRun = false;
        std::vector<NodeGraphInput> mInputs;
        std::vector<NodeGraphOutput> mOutputs;

        int32_t mId;
        std::string mName;
    };

    class NodeGraphOp {
    public:
        NodeGraphOp(int8_t numInputs = 1, int8_t numOutputs = 1) : 
            mNumInputs(numInputs), 
            mNumOutputs(numOutputs) 
        {}

        std::string defaultInStrings[3] = { "In 1", "In 2", "In 3" };
        std::string defaultOutStrings[3] = { "Out 1", "Out 2", "Out 3" };

        virtual ~NodeGraphOp() = default;
        virtual void Reset() {}
        virtual void Process(std::vector<NodeGraphInput>& mInputs, std::vector<NodeGraphOutput>& outputs) = 0;

        virtual void SetNumInputs(int8_t numInputs);
        virtual void SetNumOutputs(int8_t numOutputs);
        int8_t GetNumInputs();
        int8_t GetNumOutputs();
        
        virtual std::string_view GetInputName(int8_t inputChannel) {
            return defaultInStrings[inputChannel];
        };
        virtual std::string_view GetOutputName(int8_t outputChannel) {
            return defaultOutStrings[outputChannel];
        }
        virtual std::string_view GetName() {
            return "";
        }

    protected:
        int8_t mNumInputs;
        int8_t mNumOutputs;
    };

    class GraphDataCopy : public NodeGraphOp {
    public:
        GraphDataCopy(int8_t numChannels = 1) :
            NodeGraphOp(numChannels, numChannels)
        {}
        virtual ~GraphDataCopy() = default;

        void Process(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
    };

    template<class T, class = std::enable_if_t<std::is_base_of_v<NodeGraphOp, T>>>
    class NodeGraph : public NodeGraphBase {
    public:
        NodeGraph(std::string_view name = "") :
            NodeGraphBase(name)
        {
            SetNumInputs(mOperation.GetNumInputs());
            SetNumOutputs(mOperation.GetNumOutputs());
        }

        virtual void SetNumInputs(int8_t numInputs) {
            mInputs.resize(numInputs);
            mOperation.SetNumInputs(numInputs);
        }

        virtual void SetNumOutputs(int8_t numOutputs) {
            mOutputs.resize(numOutputs);
            mOperation.SetNumOutputs(numOutputs);
        }

        virtual void Reset() override {
            NodeGraphBase::Reset();
            mOperation.Reset();
        }

        virtual void ProcessOperation() override {
            NodeGraphBase::ProcessOperation();
            mOperation.Process(mInputs, mOutputs);
        }

        virtual std::string_view GetInputName(int8_t inputChannel) {
            auto& customName = mInputs.at(inputChannel).mName;
            if (!customName.empty()) {
                return customName;
            }
            return mOperation.GetInputName(inputChannel);
        }

        virtual std::string_view GetOutputName(int8_t outputChannel) {
            auto& customName = mOutputs.at(outputChannel).mName;
            if (!customName.empty()) {
                return customName;
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
        NodeGraphGroup() {
            SetNumInputs(1);
            SetNumOutputs(1);
        }
        ~NodeGraphGroup() = default;

        void SetNumInputs(int8_t numInputs);
        void SetNumOutputs(int8_t outputCount);
        void SetInput(int8_t inputChannel, NodeGraphBase& source, int8_t sourceOutputChannel);
        void SetInput(int8_t inputChannel, NodeGraphGroup& source, int8_t sourceOutputChannel, bool useSourceInternalInput = false);
        void SetInput(int8_t inputChannel, float constant);
        void SetInput(int8_t inputChannel, float* floatPtr);

        void SetOutput(int8_t outputChannel, NodeGraphBase& source, int8_t sourceOutputChannel);
        void SetOutput(int8_t outputChannel, NodeGraphGroup& source, int8_t sourceOutputChannel);

        float Get(int8_t outputChannel);
        NodeGraphBase& GetInputNode();
        NodeGraphBase& GetOutputNode();

        NodeGraphBase* GetNode(int32_t id);
        bool RemoveNode(int32_t id);

        template<class T, class = std::enable_if_t<std::is_base_of_v<NodeGraphOp, T>>>
        l::nodegraph::NodeGraphBase* NewNode() {
            mNodes.push_back(std::make_unique<l::nodegraph::NodeGraph<T>>());
            return mNodes.back().get();
        }

        void Update();
    protected:
        NodeGraph<GraphDataCopy> mInputNode;
        NodeGraph<GraphDataCopy> mOutputNode;

        std::vector<std::unique_ptr<NodeGraphBase>> mNodes;
    };

}

