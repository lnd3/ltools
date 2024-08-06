#pragma once

#include "logging/LoggingAll.h"

#include <string>
#include <vector>
#include <map>
#include <typeinfo>
#include <type_traits>

namespace l::graph {

    bool IsValidInOutNum(int8_t inoutNum, size_t inoutSize);

    struct GraphNodeOutput {
        float mOutput = 0.0f;
    };

    class GraphNodeBase;
    struct GraphNodeInput;
    class GraphNodeGroup;

    union Input {
        GraphNodeBase* mInputNode = nullptr;
        float* mInputFloat;
        float mInputFloatConstant;
        int32_t* mInputInt;
        int32_t mInputIntConstant;
    };

    enum class DataType {
        FLOAT32,
        INT32,
        BITFIELD32
    };

    enum class InputType {
        INPUT_NODE,
        INPUT_CONSTANT,
        INPUT_VALUE
    };

    class GraphNodeBase {
    public:
        GraphNodeBase(std::string_view name = "");
        virtual ~GraphNodeBase() = default;

        virtual void Reset();

        void Update();

        virtual void SetNumInputs(int8_t numInputs);
        virtual void SetNumOutputs(int8_t outputCount);

        virtual float Get(int8_t outputChannel);

        virtual void SetInput(int8_t inputChannel, GraphNodeBase& source, int8_t sourceOutputChannel = 0);
        virtual void SetInput(int8_t inputChannel, GraphNodeGroup& source, int8_t sourceOutputChannel = 0, bool useSourceInternalInput = true);
        virtual void SetInput(int8_t inputChannel, float constant);
        virtual void SetInput(int8_t inputChannel, float* floatPtr);
    protected:
        void PreUpdate();
        virtual void ProcessOperation();

        bool mProcessUpdateHasRun = false;
        std::vector<GraphNodeInput> mInputs;
        std::vector<GraphNodeOutput> mOutputs;

        std::string mName;
    };

    struct GraphNodeInput {
        Input mInput;
        InputType mInputType = InputType::INPUT_NODE;
        int8_t mInputFromOutputChannel = 0;

        float Get();
    };

    class GraphOp {
    public:
        GraphOp(int8_t numInputs = 1, int8_t numOutputs = 1) : 
            mNumInputs(numInputs), 
            mNumOutputs(numOutputs) 
        {}

        virtual ~GraphOp() = default;
        virtual void Reset() {}
        virtual void Process(std::vector<GraphNodeInput>& mInputs, std::vector<GraphNodeOutput>& outputs) = 0;

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

        void Process(std::vector<GraphNodeInput>& inputs, std::vector<GraphNodeOutput>& outputs) override;
    };

    template<class T, class = std::enable_if_t<std::is_base_of_v<GraphOp, T>>>
    class GraphNode : public GraphNodeBase {
    public:
        GraphNode(std::string_view name = "") :
            GraphNodeBase(name)
        {
            SetNumInputs(mOperation.GetNumInputs());
            SetNumOutputs(mOperation.GetNumOutputs());
        }

        virtual ~GraphNode() = default;

        void SetNumInputs(int8_t numInputs) {
            mInputs.resize(numInputs);
            mOperation.SetNumInputs(numInputs);
        }

        void SetNumOutputs(int8_t numOutputs) {
            mOutputs.resize(numOutputs);
            mOperation.SetNumOutputs(numOutputs);
        }

        void Reset() override {
            GraphNodeBase::Reset();
            mOperation.Reset();
        }

        void ProcessOperation() override {
            GraphNodeBase::ProcessOperation();
            mOperation.Process(mInputs, mOutputs);
        }

    protected:
        T mOperation;
    };

    class GraphNodeGroup {
    public:
        GraphNodeGroup() {
            SetNumInputs(1);
            SetNumOutputs(1);
        }
        ~GraphNodeGroup() = default;

        void SetNumInputs(int8_t numInputs);
        void SetNumOutputs(int8_t outputCount);
        void SetInput(int8_t inputChannel, GraphNodeBase& source, int8_t sourceOutputChannel);
        void SetInput(int8_t inputChannel, GraphNodeGroup& source, int8_t sourceOutputChannel, bool useSourceInternalInput = false);
        void SetInput(int8_t inputChannel, float constant);
        void SetInput(int8_t inputChannel, float* floatPtr);

        void SetOutput(int8_t outputChannel, GraphNodeBase& source, int8_t sourceOutputChannel);
        void SetOutput(int8_t outputChannel, GraphNodeGroup& source, int8_t sourceOutputChannel);

        float Get(int8_t outputChannel);
        GraphNodeBase& GetInputNode();
        GraphNodeBase& GetOutputNode();

        void Update();
    protected:
        GraphNode<GraphDataCopy> mInputNode;
        GraphNode<GraphDataCopy> mOutputNode;

        std::vector<std::unique_ptr<GraphNodeBase>> mNodes;
    };

    class GraphNodeSchema {
    public:
        GraphNodeSchema() = default;
        ~GraphNodeSchema() = default;

        template<class T, class = std::enable_if<std::is_base_of_v<GraphNodeBase, T>>>
        void NewNode(std::string_view name = "") {
            mNodes.emplace_back(std::make_unique<GraphNode<T>>(name));
        }

    protected:
        std::vector<std::unique_ptr<GraphNodeBase>> mNodes;

    };

}

