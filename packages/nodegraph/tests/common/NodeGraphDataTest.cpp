#include "testing/Test.h"
#include "logging/Log.h"

#include "nodegraph/core/NodeGraphData.h"
#include "nodegraph/core/NodeGraphInput.h"
#include "nodegraph/core/NodeGraphOutput.h"
#include "nodegraph/core/NodeGraphBase.h"

using namespace l;
using namespace l::nodegraph;


class TestOp : public NodeGraphOp {
public:
    TestOp(NodeGraphBase* node) :
        NodeGraphOp(node, "TestOp"),
        mNodeInputManager(*this)
    {
        mNodeInputManager.AddInputBase(InputTypeBase::SAMPLED, AddInput("In", 0.0f, 1, 0.0f, 1.0f));
        AddOutput("Out");
    }

    virtual ~TestOp() = default;
    virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
        mNodeInputManager.ProcessUpdate(inputs, numSamples, 4.0f);
        auto in = inputs.at(0).GetIterator(numSamples);
        auto out = outputs.at(0).GetIterator(numSamples, 4.0f);

        for (int i = 0; i < numSamples; i++) {
            *out++ = *in++;
        }
    }
protected:
    NodeInputManager mNodeInputManager;
};



TEST(NodeGraphData, Setup) {

    NodeGraph<TestOp> node0;
    NodeGraph<TestOp> node1;

    auto input = node0.GetInputOf(0).GetIterator(16);
    for (int i = 0; i < 16; i++) {
        *input++ = i;
    }

    node1.SetInput(0, node0, 0);
    node1.ProcessSubGraph(16);

    auto output = node1.GetOutputOf(0).GetIterator();

    for (int i = 0; i < 16; i++) {
        LOG(LogInfo) << "Output(" << i << "): " << *output++;
    }

	return 0;
}


