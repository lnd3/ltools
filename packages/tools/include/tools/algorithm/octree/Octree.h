#pragma once

#include <algorithm>
#include <array>
#include <memory>
#include <vector>


namespace l::algorithm::octree {

class INode {
public:
    INode* mParent = nullptr;
    INode* mNode = nullptr;
};

class NodeOctree : public INode {
public:

    std::array<std::unique_ptr<INode>, 8> mChildren;
};

class Octree : public INode {
public:
    Octree() {}
protected:
    std::unique_ptr<NodeOctree> mRoot;
};

}
