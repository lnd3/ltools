#pragma once

#include <stdint.h>
#include <set>
#include <vector>
#include <span>
#include <tuple>

#include "logging/LoggingAll.h"

namespace l::physics {

#define CO_NOT_AN_INDEX		0x0fffffff
#define CO_BOUNDARY_MIN		0
#define CO_BOUNDARY_MAX		0x80000000

	struct BoundaryData {
		uint32_t data;

		void clear() {
			data = CO_NOT_AN_INDEX;
		}
		
		void set(uint32_t ownerId, bool isMaxBoundary) {
			data = ownerId | (isMaxBoundary ? CO_BOUNDARY_MAX : CO_BOUNDARY_MIN);
		}

		void setOwnerId(uint32_t ownerId) {
			data = ownerId | (data & CO_BOUNDARY_MAX);
		}

		void setBoundaryType(bool isMaxBoundary) {
			data = (data & (~CO_BOUNDARY_MAX)) | (isMaxBoundary ? CO_BOUNDARY_MAX : CO_BOUNDARY_MIN);
		}

		bool valid() {
			return (data & CO_NOT_AN_INDEX) != CO_NOT_AN_INDEX;
		}

		uint32_t getOwnerId() {
			return data & (~CO_BOUNDARY_MAX);
		}

		bool isMinBoundary() {
			return (data & CO_BOUNDARY_MAX) == CO_BOUNDARY_MIN;
		}

		bool isMaxBoundary() {
			return (data & CO_BOUNDARY_MAX) == CO_BOUNDARY_MAX;
		}
	};

	struct BoundaryChange {
		uint32_t boundaryIndex;
		BoundaryData boundaryData;

		void set(uint32_t index, BoundaryData data) {
			boundaryIndex = index;
			boundaryData = data;
		}

	};

	struct Boundary {
		BoundaryData data;
		float value;

		void clear() {
			data.clear();
			value = 0.0f;
		}

		void set(uint32_t ownerId, bool isMaxBoundary, float boundaryValue) {
			data.set(ownerId, isMaxBoundary);
			value = boundaryValue;
		}

		void setValue(float boundaryValue) {
			value = boundaryValue;
		}
	};

	class BoundaryList {
	public:
		void init(uint32_t axis, uint32_t maxSize);
		uint32_t add(uint32_t id, bool isMaxBoundary, float value);
		std::tuple<uint32_t, uint32_t> add(uint32_t id, float low, float high);
		void updateBoundary(uint32_t index, float value);
		void clear(uint32_t index);
		void remove(uint32_t index);
		void sortInsertion(std::function<void(uint32_t, std::span<BoundaryChange>)> updateBoundaries, std::function<void(uint32_t, uint32_t, bool)> updatePair = nullptr);
	protected:
		void move(uint32_t index, Boundary boundary);
		void addBoundaryChange(uint32_t index, BoundaryData data);

		std::vector<Boundary> mBoundaries;
		std::vector<BoundaryChange> mBoundaryChanges;

		uint32_t mSize = 0;
		uint32_t mCleared = 0;
		uint32_t mBoundaryChangeSize = 0;
		uint32_t mAxis = 0;
	};
}
