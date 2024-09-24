#include "testing/Test.h"
#include "logging/Log.h"

#include "nodegraph/core/NodeGraphGroup.h"
#include "nodegraph/NodeGraphSchema.h"

using namespace l;
using namespace l::nodegraph;

TEST(NodeGraphBatching, Simple) {

	NodeGraphGroup group;
	group.SetNumOutputs(1);

	auto nodeSine = group.NewNode<GraphSignalSine2>(OutputType::Default);
	nodeSine->SetInput(1, 4.0f); // sine update rate
	nodeSine->SetInput(2, 1400.0f); // sine freq
	nodeSine->SetInput(3, 0.5f); // sine freq

	auto nodeLowpass = group.NewNode<GraphFilterLowpass>(OutputType::Default);

	nodeLowpass->SetInput(1, *nodeSine, 0);
	group.SetOutput(0, *nodeLowpass, 0);


	for (int32_t i = 0; i < 4; i++) {
		LOG(LogInfo) << "batch " << i;

		group.ProcessSubGraph(8); // 8 samples per batch
		auto output = &group.GetOutput(0, 8);

		for (int32_t j = 0; j < 8; j++) {
			LOG(LogInfo) << "sample " << (j+(i*8)) << ": " << *output++;
		}
	}

	auto output = &group.GetOutput(0, 8);
	
	TEST_FUZZY(0.012347f, output[7], 0.0001f, "");

	return 0;
}


TEST(NodeGraphBatching, Multiple) {

	return 0;
}


