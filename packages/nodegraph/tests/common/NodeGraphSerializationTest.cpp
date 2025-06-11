#include "testing/Test.h"
#include "logging/Log.h"

#include "nodegraph/core/NodeGraphGroup.h"
#include "nodegraph/NodeGraphSchema.h"

using namespace l;
using namespace l::nodegraph;


TEST(NodeGraph, SerializationBasic) {
	NodeGraphSchema schema;

	schema.RegisterAllOperators();
	
	// loading all nodes and simply using a running index as id since we dont want id gaps during regeneration.
	auto id0 = schema.NewNode(100);
	auto id1 = schema.NewNode(100);
	auto id2 = schema.NewNode(100);

	// mapping of nodes 
	schema.GetNode(id2)->SetInput(0, *schema.GetNode(id0), 0);
	schema.GetNode(id2)->SetInput(1, *schema.GetNode(id1), 0);


	std::stringstream dst;
	l::serialization::JsonBuilder builder(true);
	builder.SetStream(&dst);

	schema.GetArchiveData(builder);
	auto d = dst.str();
	LOG(LogInfo) << "Archive:";
	LOG(LogInfo) << "\n" << d;

	l::serialization::JsonParser<100> parser;
	auto [result, error] = parser.LoadJson(d.c_str(), d.size());
	auto jsonRoot = parser.GetRoot();

	NodeGraphSchema schema2;
	schema2.LoadArchiveData(jsonRoot);

	return 0;
}


