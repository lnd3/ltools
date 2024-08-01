#pragma once

#include <stdint.h>
#include <set>
#include <vector>
#include <span>
#include <unordered_map>

#include "logging/LoggingAll.h"

#include "physics/BoundaryList.h"
#include "physics/VecX.h"

namespace l::physics {

#define CO_INVALID			0xf0000000
#define CO_AXIS_X			0
#define CO_AXIS_Y			1
#define CO_AXIS_Z			2

	struct Box {
		uint32_t minBoundaryIndex[3];
		uint32_t maxBoundaryIndex[3];
		uint32_t flags;

		bool valid() {
			return (minBoundaryIndex[0] & CO_INVALID) == 0;
		}

		void invalidate() {
			minBoundaryIndex[0] = CO_INVALID;
		}

		void set(uint32_t minIndex, uint32_t maxIndex) {
			minBoundaryIndex[0] = minIndex;
			minBoundaryIndex[1] = minIndex;
			minBoundaryIndex[2] = minIndex;
			maxBoundaryIndex[0] = maxIndex;
			maxBoundaryIndex[1] = maxIndex;
			maxBoundaryIndex[2] = maxIndex;
		}
	};

	class BoxSweep {
	public:
		static bool IsOverlapping(uint32_t axis, const Box& a, const Box& b);

		void init(uint32_t maxSize);
		uint32_t add(const l::vec::Data4<float>& pos, float radius);
		void updateBox(uint32_t boxId, const l::vec::Data4<float>& pos, float size);
		void remove(uint32_t index);
		void update();
		void overlapAction(std::function<void(uint32_t, uint32_t)> action);
	private:

		std::unordered_map<uint32_t, Box> mBoxes;
		BoundaryList mBoxBoundariesX;
		BoundaryList mBoxBoundariesY;
		BoundaryList mBoxBoundariesZ;
		std::set<uint64_t> mOverlapPairs;

		uint32_t mIds = 0;
		uint32_t mSize = 0;
		uint32_t mRemoved = 0;
		uint32_t mMaxSize = 0;
	};
}
