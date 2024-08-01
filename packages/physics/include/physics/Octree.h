#pragma once

#include <algorithm>
#include <array>
#include <memory>
#include <vector>

#include "VecX.h"

namespace l::physics::octree {

class INode {
public:
    l::vec::Data4<double> mPosition;

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
