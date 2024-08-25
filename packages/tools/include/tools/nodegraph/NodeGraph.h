#pragma once

#include "logging/LoggingAll.h"

#include <string>
#include <vector>
#include <map>
#include <typeinfo>
#include <type_traits>

namespace l::nodegraph {

    enum class DataType {
        FLOAT32,
        INT32,
        BITFIELD32
    };

    enum class InputType {
        INPUT_NODE,
        INPUT_CONSTANT,
        INPUT_VALUE,
        //INPUT_ARRAY // TODO: is it possible  to process batches for example for audio processing?
    };

    bool IsValidInOutNum(int8_t inoutNum, size_t inoutSize);

    struct NodeGraphOutput {
        float mOutput = 0.0f;
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
        InputType mInputType = InputType::INPUT_NODE;
        int8_t mInputFromOutputChannel = 0;

        float Get();
    };

    class NodeGraphBase {
    public:
        NodeGraphBase(std::string_view name = "");
        virtual ~NodeGraphBase() = default;

        virtual void Reset();

        void Update();

        virtual void SetNumInputs(int8_t numInputs);
        virtual void SetNumOutputs(int8_t outputCount);

        virtual float Get(int8_t outputChannel);

        virtual void SetInput(int8_t inputChannel, NodeGraphBase& source, int8_t sourceOutputChannel = 0);
        virtual void SetInput(int8_t inputChannel, NodeGraphGroup& source, int8_t sourceOutputChannel = 0, bool useSourceInternalInput = true);
        virtual void SetInput(int8_t inputChannel, float constant);
        virtual void SetInput(int8_t inputChannel, float* floatPtr);
    protected:
        void PreUpdate();
        virtual void ProcessOperation();

        bool mProcessUpdateHasRun = false;
        std::vector<NodeGraphInput> mInputs;
        std::vector<NodeGraphOutput> mOutputs;

        std::string mName;
    };

    class GraphOp {
    public:
        GraphOp(int8_t numInputs = 1, int8_t numOutputs = 1) : 
            mNumInputs(numInputs), 
            mNumOutputs(numOutputs) 
        {}

        virtual ~GraphOp() = default;
        virtual void Reset() {}
        virtual void Process(std::vector<NodeGraphInput>& mInputs, std::vector<NodeGraphOutput>& outputs) = 0;

        virtual void SetNumInputs(int8_t numInputs);
        virtual void SetNumOutputs(int8_t numOutputs);
        int8_t GetNumInputs();
        int8_t GetNumOutputs();
    protected:
        int8_t mNumInputs;
        int8_t mNumOutputs;
    };

    class GraphDataCopy : public GraphOp {
    public:
        GraphDataCopy(int8_t numChannels = 1) :
            GraphOp(numChannels, numChannels)
        {}
        virtual ~GraphDataCopy() = default;

        void Process(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
    };

    template<class T, class = std::enable_if_t<std::is_base_of_v<GraphOp, T>>>
    class NodeGraph : public NodeGraphBase {
    public:
        NodeGraph(std::string_view name = "") :
            NodeGraphBase(name)
        {
            SetNumInputs(mOperation.GetNumInputs());
            SetNumOutputs(mOperation.GetNumOutputs());
        }

        virtual ~NodeGraph() = default;

        void SetNumInputs(int8_t numInputs) {
            mInputs.resize(numInputs);
            mOperation.SetNumInputs(numInputs);
        }

        void SetNumOutputs(int8_t numOutputs) {
            mOutputs.resize(numOutputs);
            mOperation.SetNumOutputs(numOutputs);
        }

        void Reset() override {
            NodeGraphBase::Reset();
            mOperation.Reset();
        }

        void ProcessOperation() override {
            NodeGraphBase::ProcessOperation();
            mOperation.Process(mInputs, mOutputs);
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

        void Update();
    protected:
        NodeGraph<GraphDataCopy> mInputNode;
        NodeGraph<GraphDataCopy> mOutputNode;

        std::vector<std::unique_ptr<NodeGraphBase>> mNodes;
    };

    class NodeGraphSchema {
    public:
        NodeGraphSchema() = default;
        ~NodeGraphSchema() = default;

        template<class T, class = std::enable_if<std::is_base_of_v<NodeGraphBase, T>>>
        void NewNode(std::string_view name = "") {
            mNodes.emplace_back(std::make_unique<NodeGraph<T>>(name));
        }

    protected:
        std::vector<std::unique_ptr<NodeGraphBase>> mNodes;

    };

}

