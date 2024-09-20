#include "testing/Test.h"
#include "logging/Log.h"

#include "nodegraph/core/NodeGraphGroup.h"
#include "nodegraph/NodeGraphSchema.h"

using namespace l;
using namespace l::nodegraph;

TEST(NodeGraph, BasicFunction) {

	NodeGraph<GraphNumericAdd> node;

	float in2 = 2.3f;

	node.SetInput(0, 1.8f);
	node.SetInput(1, &in2);

	node.ProcessSubGraph();
	TEST_FUZZY(node.GetOutput(0), 4.1f, 0.0001f, "");

	return 0;
}

TEST(NodeGraph, SimpleAddNetwork) {

	NodeGraph<GraphNumericAdd> node1;
	NodeGraph<GraphNumericAdd> node2;
	NodeGraph<GraphNumericAdd> nodeFinal;

	float in1 = 1.8f;
	float in3 = 5.2f;
	float in4 = 3.3f;

	node1.SetInput(0, &in1);
	node1.SetInput(1, 2.3f);
	node2.SetInput(0, &in3);
	node2.SetInput(1, &in4);

	nodeFinal.SetInput(0, node1, 0);
	nodeFinal.SetInput(1, node2, 0);

	nodeFinal.ProcessSubGraph();
	TEST_FUZZY(nodeFinal.GetOutput(0), 12.6f, 0.0001f, "");

	return 0;
}

TEST(NodeGraph, BasicMathematicalOperations) {
	NodeGraph<GraphNumericAdd> node1;
	NodeGraph<GraphNumericMultiply> node2;
	NodeGraph<GraphNumericSubtract> node3;
	NodeGraph<GraphNumericNegate> nodeOutput;

	float in1 = 1.8f;
	float in3 = 2.0f;
	float in4 = 1.3f;

	node1.SetInput(0, &in1);
	node1.SetInput(1, 2.3f);

	node2.SetInput(0, node1, 0);
	node2.SetInput(1, &in3); // (in1 + in2) * in3

	node3.SetInput(0, node2, 0);
	node3.SetInput(1, &in4); // (in1 + in2) * in3 - in4

	nodeOutput.SetInput(0, node3, 0); // - (in1 + in2) * in3 - in4

	nodeOutput.ProcessSubGraph();
	TEST_FUZZY(nodeOutput.GetOutput(0), -6.9f, 0.0001f, "");

	return 0;
}

TEST(NodeGraph, NumericIntegral) {
	NodeGraph<GraphNumericIntegral> nodeIntegral;

	float input;
	nodeIntegral.SetInput(0, &input);

	float oneRev = 2.0f * 3.14f / 30.0f;
	for (int i = 0; i < 30; i++) {
		input = sinf(2.0f * i * oneRev);
		nodeIntegral.ProcessSubGraph();
		//LOG(LogInfo) << nodeIntegral.Get(0);
	}

	TEST_FUZZY(nodeIntegral.GetOutput(0), 0.00323272f, 0.0001f, "");

	return 0;
}

TEST(NodeGraph, FilterLowpass) {
	NodeGraph<GraphFilterLowpass> nodeLowpass;

	float cutoff = 0.8f;
	float resonance = 0.9f;
	float input = 1.3f;

	nodeLowpass.SetInput(1, &input);
	nodeLowpass.SetInput(2, &cutoff);
	nodeLowpass.SetInput(3, &resonance);

	float oneRev = 2.0f * 3.14f / 30.0f;
	for (int i = 0; i < 30; i++) {
		input = sinf(2.0f * i * oneRev);
		nodeLowpass.ProcessSubGraph();
		//LOG(LogInfo) << nodeLowpass.GetOutput(0);
	}

	TEST_FUZZY(nodeLowpass.GetOutput(0), -0.201134, 0.0001f, "");

	return 0;
}

TEST(NodeGraph, GraphGroups) {

	NodeGraphGroup group;

	float cutoff = 0.8f;
	float resonance = 0.0001f;
	float input1 = 0.3f;
	float input2 = 0.2f;

	{ // create group
		group.SetNumInputs(4);
		group.SetNumOutputs(2);

		group.SetInput(0, &cutoff);
		group.SetInput(1, &resonance);
		group.SetInput(2, &input1);
		group.SetInput(3, &input2);

		auto nodeLowpass1 = group.NewNode<GraphFilterLowpass>(OutputType::Default);
		auto nodeLowpass2 = group.NewNode<GraphFilterLowpass>(OutputType::Default);

		// left, right
		nodeLowpass1->SetInput(1, group, 2);
		nodeLowpass2->SetInput(1, group, 3);

		// cutoff
		nodeLowpass1->SetInput(2, group, 0);
		nodeLowpass2->SetInput(2, group, 0);

		// resonance
		nodeLowpass1->SetInput(3, group, 1);
		nodeLowpass2->SetInput(3, group, 1);

		group.SetOutput(0, *nodeLowpass1, 0);
		group.SetOutput(1, *nodeLowpass2, 0);
	}

	NodeGraphGroup group2;

	{ // wire sequential group with a simple copy node
		group2.SetNumInputs(2);
		group2.SetNumOutputs(2);
		group2.SetInput(0, group, 0);
		group2.SetInput(1, group, 1);

		auto copyNode = group2.NewNode<GraphDataCopy>(OutputType::Default, 2);
		copyNode->SetInput(0, group2, 0);
		copyNode->SetInput(1, group2, 1);

		group2.SetOutput(0, *copyNode, 0);
		group2.SetOutput(1, *copyNode, 1);
	}

	// only update the last group/node and all dependent nodes will update in the graph to produce an output
	group2.ProcessSubGraph(1);

	float output1 = group2.GetOutput(0);
	float output2 = group2.GetOutput(1);

	TEST_FUZZY(output1, 0.149999559f, 0.00001, "");
	TEST_FUZZY(output2, 0.0999997109f, 0.00001, "");

	return 0;
}

TEST(NodeGraph, SchemaBasic) {

	NodeGraphSchema ng;

	std::vector<int32_t> nodeIds;

	for (int32_t i = 0; i < 1000; i++) {
		auto nodeId = ng.NewNode(i);
		if (nodeId > 0) {
			nodeIds.push_back(nodeId);
		}
	}

	int32_t tick = 0;
	for (int32_t j = 0; j < 10; j++) {
		for (int32_t i = 0; i < 10; i++) {
			ng.Tick(tick++, 0.001f);
			ng.ProcessSubGraph(100);
		}
	}
	
	for (auto nodeId : nodeIds) {
		TEST_TRUE(ng.RemoveNode(nodeId), "");
	}


	return 0;
}

