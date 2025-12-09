#include "storage/SequentialCache.h"
#include "filesystem/File.h"

namespace l::filecache {

	int32_t GetClampedPosition(int32_t position, int32_t blockWidth) {
		return blockWidth * (position / blockWidth);
	}

	int32_t GetClampedPositionOffset(int32_t position, int32_t blockWidth) {
		return position - blockWidth * (position / blockWidth);
	}

	int32_t GetClampedPositionOffsetFromIndex(int32_t index, int32_t blockWidth, int32_t numBlockEntries) {
		return blockWidth * index / numBlockEntries;
	}

}
