#include "physics/PhysicsAll.h"

#include "physics/BoundaryList.h"

#include <algorithm>

namespace l::physics {

void BoundaryList::init(uint32_t axis, uint32_t maxSize) {
	mAxis = axis;
	mBoundaries.resize(maxSize * 2);
	mSize = 0;
	mCleared = 0;
	mBoundaryChanges.resize(1024);
	mBoundaryChangeSize = 0;
}

uint32_t BoundaryList::add(uint32_t id, bool isMaxBoundary, float value) {
	ASSERT(mSize < mBoundaries.size());
	if (mCleared >= 1 && !mBoundaries.at(0).data.valid()) {
		mCleared--;
		mBoundaries.at(0).set(id, isMaxBoundary, value);
		return 0;
	}
	else {
		mBoundaries.at(mSize).set(id, isMaxBoundary, value);
		return mSize++;
	}
}

std::tuple<uint32_t, uint32_t>  BoundaryList::add(uint32_t id, float low, float high) {
	ASSERT(mSize < mBoundaries.size());
	if (mCleared >= 2 && !mBoundaries.at(0).data.valid() && !mBoundaries.at(1).data.valid()) {
		mCleared -= 2;
		mBoundaries.at(0).set(id, false, low);
		mBoundaries.at(1).set(id, true, high);
		return {0, 1};
	}
	else {
		mBoundaries.at(mSize+0).set(id, false, low);
		mBoundaries.at(mSize+1).set(id, true, high);
		mSize += 2;
		return { mSize-2, mSize-1 };
	}
}

void BoundaryList::updateBoundary(uint32_t index, float value) {
	mBoundaries.at(index).setValue(value);
}

void BoundaryList::clear(uint32_t index) {
	mBoundaries.at(index).clear();
	mCleared++;
}

void BoundaryList::remove(uint32_t index) {
	ASSERT(index < mSize);
	if (index < mSize - 1) {
		move(index, mBoundaries.at(mSize - 1));
	}
	mSize--;
}

void BoundaryList::move(uint32_t dstIndex, Boundary boundary) {
	// Simple copy
	mBoundaries[dstIndex] = boundary;

	// Track change so we know who to notify about index position change
	if (boundary.data.valid()) {
		addBoundaryChange(dstIndex, boundary.data);
	}
}

void BoundaryList::sortInsertion(std::function<void(uint32_t, std::span<BoundaryChange>)> updateBoundaries, std::function<void(uint32_t, uint32_t, bool)> updatePair) {
	uint32_t startIndex = 0;
	uint32_t endIndex = mSize;

	for (uint32_t i = startIndex + 1; i < endIndex; i++) {
		auto iBoundary = mBoundaries.at(i);

		int32_t j = i - 1;
		bool done = false;

		do {
			auto jBoundary = mBoundaries.at(j);
			if (jBoundary.value <= iBoundary.value) {
				done = true;
			}
			else {
				mBoundaries.at(j).clear();
				move(j + 1, jBoundary);

				if (updatePair && iBoundary.data.valid() && jBoundary.data.valid()) {
					if (iBoundary.data.isMinBoundary()) {
						if (jBoundary.data.isMaxBoundary()) {
							updatePair(jBoundary.data.getOwnerId(), iBoundary.data.getOwnerId(), true);
						}
					}
					else {
						if (jBoundary.data.isMinBoundary()) {
							updatePair(jBoundary.data.getOwnerId(), iBoundary.data.getOwnerId(), false);
						}
					}
				}

				j = j - 1;
				if (j < 0) {
					done = true;
				}
			}
		} while (!done);

		if (static_cast<int32_t>(i) > j + 1) {
			mBoundaries.at(i).clear();
			move(j + 1, iBoundary);
		}
	}
	
	updateBoundaries(mAxis, std::span<BoundaryChange>(mBoundaryChanges.begin(), mBoundaryChangeSize));
	mBoundaryChangeSize = 0;
}

void BoundaryList::addBoundaryChange(uint32_t index, BoundaryData data) {
	ASSERT(mBoundaryChangeSize < mBoundaryChanges.size());
	mBoundaryChanges[mBoundaryChangeSize].set(index, data);
	mBoundaryChangeSize++;
}

}
