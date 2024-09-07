#include "physics/PhysicsAll.h"

#include "physics/BoxSweep.h"
#include "physics/VecX.h"
#include "math/MathAlgorithm.h"

#include <algorithm>

namespace {
	constexpr uint32_t kAxisDimensions = 3;
}

namespace l::physics {

bool BoxSweep::IsOverlapping(uint32_t axis, const Box& a, const Box& b) {
	// Exit if indexes are not overlapping on axis
	for (uint32_t k = 0; k < kAxisDimensions; k++) {
		// Assumes indexes are sorted according to their respective boundary values
		// If not, we may miss potential overlaps resulting in popping when overlaps 
		// are rediscovered (false positives would be excluded in narrow phase)
		// Must check both end bounds on all axis
		if ((k != axis && b.minBoundaryIndex[k] > a.maxBoundaryIndex[k]) || b.maxBoundaryIndex[k] < a.minBoundaryIndex[k]) {
			return false;
		}
	}
	return true;
}

void BoxSweep::init(uint32_t maxSize) {
	mIds = 0;
	mSize = 0;
	mMaxSize = maxSize;
	mRemoved = 0;

	mBoxes.reserve(mMaxSize);
	mBoxBoundariesX.init(CO_AXIS_X, mMaxSize * 2);
	mBoxBoundariesY.init(CO_AXIS_Y, mMaxSize * 2);
	mBoxBoundariesZ.init(CO_AXIS_Z, mMaxSize * 2);
}

uint32_t BoxSweep::add(const l::vec::Data4<float>& pos, float radius) {
	ASSERT(mSize < mMaxSize);

	uint32_t boxId = CO_NOT_AN_INDEX;

	boxId = mIds++;
	{
		auto [index0, index1] = mBoxBoundariesX.add(boxId, pos.x1 - radius, pos.x1 + radius);
		mBoxes[boxId].set(index0, index1);
	}
	{
		auto [index0, index1] = mBoxBoundariesX.add(boxId, pos.x2 - radius, pos.x2 + radius);
		mBoxes[boxId].set(index0, index1);
	}
	{
		auto [index0, index1] = mBoxBoundariesX.add(boxId, pos.x3 - radius, pos.x3 + radius);
		mBoxes[boxId].set(index0, index1);
	}

	mSize++;

	return boxId;
}

void BoxSweep::updateBox(uint32_t boxId, const l::vec::Data4<float>& pos, float radius) {
	auto& box = mBoxes.at(boxId);
	mBoxBoundariesX.updateBoundary(box.minBoundaryIndex[0], pos.x1 - radius);
	mBoxBoundariesX.updateBoundary(box.maxBoundaryIndex[0], pos.x1 + radius);
	mBoxBoundariesY.updateBoundary(box.minBoundaryIndex[1], pos.x2 - radius);
	mBoxBoundariesY.updateBoundary(box.maxBoundaryIndex[1], pos.x2 + radius);
	mBoxBoundariesZ.updateBoundary(box.minBoundaryIndex[2], pos.x3 - radius);
	mBoxBoundariesZ.updateBoundary(box.maxBoundaryIndex[2], pos.x3 + radius);
}

void BoxSweep::remove(uint32_t boxId) {
	auto& removedBox = mBoxes.at(boxId);
	mBoxBoundariesX.clear(removedBox.maxBoundaryIndex[0]);
	mBoxBoundariesX.clear(removedBox.minBoundaryIndex[0]);
	mBoxBoundariesY.clear(removedBox.maxBoundaryIndex[1]);
	mBoxBoundariesY.clear(removedBox.minBoundaryIndex[1]);
	mBoxBoundariesZ.clear(removedBox.maxBoundaryIndex[2]);
	mBoxBoundariesZ.clear(removedBox.minBoundaryIndex[2]);

	mBoxes.erase(boxId);

	mRemoved++;
	mSize--;
}

void BoxSweep::update() {

	auto changes = [&](uint32_t axis, std::span<BoundaryChange> changes) {
		for (auto& change : changes) {
			if (!change.boundaryData.valid()) {
				continue;
			}

			auto ownerId = change.boundaryData.getOwnerId();
			if (!mBoxes.contains(ownerId)) {
				continue;
			}

			auto& box = mBoxes.at(ownerId);

			if (change.boundaryData.isMaxBoundary()) {
				box.maxBoundaryIndex[axis] = change.boundaryIndex;
			}
			else {
				box.minBoundaryIndex[axis] = change.boundaryIndex;
			}
		}
	};

	auto updatePair = [&](uint32_t id0, uint32_t id1, bool add) {
		auto id = l::math::algorithm::pairIndex32(id0, id1);
		if (add) {
			if (IsOverlapping(CO_AXIS_Y, mBoxes.at(id0), mBoxes.at(id1))) {
				mOverlapPairs.emplace(id);
			}
			else {

				mOverlapPairs.erase(id);
			}
		}
		else {
			mOverlapPairs.erase(id);
		}
	};

	mOverlapPairs.clear();

	mBoxBoundariesX.sortInsertion(changes);
	mBoxBoundariesZ.sortInsertion(changes);
	mBoxBoundariesY.sortInsertion(changes, updatePair);
}

void BoxSweep::overlapAction(std::function<void(uint32_t, uint32_t)> action) {
	for (auto id : mOverlapPairs) {
		auto id0 = static_cast<uint32_t>(id & 0xffffffff);
		auto id1 = static_cast<uint32_t>(id >> 32);
		action(id0, id1);
	}
}
}
