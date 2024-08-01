#include "testing/Test.h"
#include "logging/Log.h"

#include "tools/graph/GraphNode.h"
#include "tools/graph/GraphOperations.h"

using namespace l;
using namespace l::graph;

TEST(GraphNode, BasicFunction) {

	GraphNode<GraphNumericAdd> node;

	float in2 = 2.3f;

	node.SetInput(0, 1.8f);
	node.SetInput(1, &in2);

	node.Update();
	TEST_FUZZY(node.Get(0), 4.1f, 0.0001f, "");

	return 0;
}

TEST(GraphNode, SimpleAddNetwork) {

	GraphNode<GraphNumericAdd> node1;
	GraphNode<GraphNumericAdd> node2;
	GraphNode<GraphNumericAdd> nodeFinal;

	float in1 = 1.8f;
	float in3 = 5.2f;
	float in4 = 3.3f;

	node1.SetInput(0, &in1);
	node1.SetInput(1, 2.3f);
	node2.SetInput(0, &in3);
	node2.SetInput(1, &in4);

	nodeFinal.SetInput(0, node1, 0);
	nodeFinal.SetInput(1, node2, 0);

	nodeFinal.Update();
	TEST_FUZZY(nodeFinal.Get(0), 12.6f, 0.0001f, "");

	return 0;
}

TEST(GraphNode, BasicMathematicalOperations) {
	GraphNode<GraphNumericAdd> node1;
	GraphNode<GraphNumericMultiply> node2;
	GraphNode<GraphNumericSubtract> node3;
	GraphNode<GraphNumericNegate> nodeOutput;

	float in1 = 1.8f;
	float in3 = 2.0f;
	float in4 = 1.3f;

	node1.SetInput(0, &in1);
	node1.SetInput(1, 2.3f);

	node2.SetInput(0, node1);
	node2.SetInput(1, &in3); // (in1 + in2) * in3

	node3.SetInput(0, node2);
	node3.SetInput(1, &in4); // (in1 + in2) * in3 - in4

	nodeOutput.SetInput(0, node3); // - (in1 + in2) * in3 - in4

	nodeOutput.Update();
	TEST_FUZZY(nodeOutput.Get(0), -6.9f, 0.0001f, "");

	return 0;
}

TEST(GraphNode, NumericIntegral) {
	GraphNode<GraphNumericIntegral> nodeIntegral;

	float input;
	nodeIntegral.SetInput(0, &input);

	float oneRev = 2.0f * 3.14f / 30.0f;
	for (int i = 0; i < 30; i++) {
		input = sinf(2.0f * i * oneRev);
		nodeIntegral.Update();
		//LOG(LogInfo) << nodeIntegral.Get(0);
	}

	TEST_FUZZY(nodeIntegral.Get(0), 0.00323272f, 0.0001f, "");

	return 0;
}

TEST(GraphNode, FilterLowpass) {
	GraphNode<GraphFilterLowpass> nodeLowpass;

	float cutoff = 0.8f;
	float resonance = 0.1f;
	float input = 1.3f;

	nodeLowpass.SetInput(0, &cutoff);
	nodeLowpass.SetInput(1, &resonance);
	nodeLowpass.SetInput(2, &input);

	float oneRev = 2.0f * 3.14f / 30.0f;
	for (int i = 0; i < 30; i++) {
		input = sinf(2.0f * i * oneRev);
		nodeLowpass.Update();
		//LOG(LogInfo) << nodeLowpass.Get(0);
	}

	TEST_FUZZY(nodeLowpass.Get(0), -0.448589f, 0.0001f, "");

	return 0;
}

TEST(GraphNode, GraphGroups) {

	GraphNodeGroup group;

	GraphNode<GraphFilterLowpass> nodeLowpass1;
	GraphNode<GraphFilterLowpass> nodeLowpass2;

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

		// cutoff
		nodeLowpass1.SetInput(0, group, 0);
		nodeLowpass2.SetInput(0, group, 0);

		// resonance
		nodeLowpass1.SetInput(1, group, 1);
		nodeLowpass2.SetInput(1, group, 1);

		// left, right
		nodeLowpass1.SetInput(2, group, 2);
		nodeLowpass2.SetInput(2, group, 3);

		group.SetOutput(0, nodeLowpass1, 0);
		group.SetOutput(1, nodeLowpass2, 0);
	}

	GraphNodeGroup group2;
	GraphNode<GraphDataCopy> copyNode;

	{ // wire sequential group with a simple copy node
		group2.SetNumInputs(2);
		group2.SetNumOutputs(2);
		group2.SetInput(0, group, 0);
		group2.SetInput(1, group, 1);

		copyNode.SetNumInputs(2);
		copyNode.SetNumOutputs(2);
		copyNode.SetInput(0, group2, 0);
		copyNode.SetInput(1, group2, 1);

		group2.SetOutput(0, copyNode, 0);
		group2.SetOutput(1, copyNode, 1);
	}

	// only update the last group/node and all dependent nodes will update in the graph to produce an output
	group2.Update();

	float output1 = group2.Get(0);
	float output2 = group2.Get(1);

	TEST_FUZZY(output1, 0.122880019f, 0.0000001, "");
	TEST_FUZZY(output2, 0.0819200128f, 0.0000001, "");

	return 0;
}

TEST(GraphNode, SchemaBasic) {

	GraphNodeSchema schema;
	schema.NewNode<GraphDataCopy>();

	return 0;
}

