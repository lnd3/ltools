#include "testing/Test.h"
#include "logging/Log.h"

#include "tools/algorithm/kdtree/KDTree.h"

using namespace l;

TEST(KDTree, Basic) {

	class Element {
	public:
		std::vector<int> components;
	};

	KDTree<4, Element> tree;

	Point<4> p;
	p[0] = 1.0;

	tree.insert(p, Element());


	return 0;
}


