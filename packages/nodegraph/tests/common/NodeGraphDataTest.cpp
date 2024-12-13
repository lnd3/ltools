#include "testing/Test.h"
#include "logging/Log.h"

#include "nodegraph/core/NodeGraphData.h"
#include "nodegraph/core/NodeGraphInput.h"
#include "nodegraph/core/NodeGraphOutput.h"
#include "nodegraph/core/NodeGraphBase.h"

using namespace l;
using namespace l::nodegraph;


class TestOp : public NodeGraphOp2 {
public:
    TestOp(NodeGraphBase* node) :
        NodeGraphOp2(node, "TestOp")
    {
        AddInput2(InputIterationType::SAMPLED, "In 0", 0.0f, 1, 0.0f, 1.0f);
        AddInput2(InputIterationType::SAMPLED_RWA, "In 1", 0.0f, 1, 0.0f, 1.0f);
        AddOutput("Out 0");
        AddOutput("Out 1");
    }

    virtual ~TestOp() = default;
    virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
        mInputManager.BatchUpdate(inputs, numSamples);
        auto out0 = outputs.at(0).GetIterator(numSamples, 4.0f);
        auto out1 = outputs.at(1).GetIterator(numSamples, 4.0f);

        for (int i = 0; i < numSamples; i++) {
            float value0 = mInputManager.GetValueNext(0);
            float value1 = mInputManager.GetValueNext(1);
            *out0++ = value0;
            *out1++ = value1;

            LOG(LogInfo) << "Node input(" << i << "): " << value0 << ", " << value1;
        }
    }
protected:
};



TEST(NodeGraphData, Sampler) {

    NodeGraph<TestOp> node0;
    NodeGraph<TestOp> node1;

    node0.SetInput(0, 0.0f, 2);
    node0.SetInput(1, 0.0f, 2);
    node1.SetInput(0, node0, 0);
    node1.SetInput(1, node0, 1);

    {
        auto input0 = node0.GetInputOf(0).GetIterator(32);
        auto input1 = node0.GetInputOf(1).GetIterator(32);
        for (int i = 0; i < 32; i++) {
            *input0++ = static_cast<float>(i > 24 ? 24 : i);
            *input1++ = static_cast<float>(i > 24 ? 24 : i);
        }
    }

    node1.ProcessSubGraph(32);

	return 0;
}


class TestOp2 : public NodeGraphOp2 {
public:
    TestOp2(NodeGraphBase* node) :
        NodeGraphOp2(node, "TestOp")
    {
        AddInput2(InputIterationType::CUSTOM_INTERP_TWEEN, "In 0", 0.0f, 1, 0.0f, 1.0f);
        AddInput2(InputIterationType::CUSTOM_INTERP_TWEEN_MS, "In 1", 0.0f, 1, 0.0f, 1.0f);
        AddOutput("Out 0");
        AddOutput("Out 1");
    }

    virtual ~TestOp2() = default;
    virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
        mInputManager.BatchUpdate(inputs, numSamples);
        auto out0 = outputs.at(0).GetIterator(numSamples, 4.0f);
        auto out1 = outputs.at(1).GetIterator(numSamples, 4.0f);

        mInputManager.SetDuration(0, 24.0f, 0.1f);
        mInputManager.SetDuration(1, 0.5f, 0.1f);

        for (int i = 0; i < numSamples; i++) {
            float value0 = mInputManager.GetValueNext(0);
            float value1 = mInputManager.GetValueNext(1);
            *out0++ = value0;
            *out1++ = value1;

            LOG(LogInfo) << "Node input(" << i << "): " << value0 << ", " << value1;
        }
    }
protected:
};

TEST(NodeGraphData, Tweening) {

    NodeGraph<TestOp> node0;
    NodeGraph<TestOp> node1;

    node0.SetInput(0, 0.0f, 2);
    node0.SetInput(1, 0.0f, 2);
    node1.SetInput(0, node0, 0);
    node1.SetInput(1, node0, 1);

    {
        auto input0 = node0.GetInputOf(0).GetIterator(32);
        auto input1 = node0.GetInputOf(1).GetIterator(32);
        for (int i = 0; i < 32; i++) {
            *input0++ = static_cast<float>(i > 24 ? 24 : i);
            *input1++ = static_cast<float>(i > 24 ? 24 : i);
        }
    }

    node1.ProcessSubGraph(32);

    return 0;
}

