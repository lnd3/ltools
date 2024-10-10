#include "memory/Arena.h"

#include "logging/Log.h"
#include "meta/Reflection.h"

#include <string>
#include <unordered_map>
#include <map>
#include <vector>
#include <algorithm>			// For lower_bound
#include <typeinfo>
#include <optional>
#include <iterator> // For std::forward_iterator_tag
#include <cstddef>  // For std::ptrdiff_t

namespace l::memory {

	Arena::Arena() {
		mBlocks.reserve(16);

	}

	Arena::~Arena() {

	}

	std::unique_ptr<Arena> CreateArena() {
		return std::make_unique<Arena>();
	}


	// push some bytes onto the 'stack' - the way to allocate
	void* ArenaPush(Arena* arena, uint64_t size) {
		if (arena->mBlocks.empty()) {
			arena->mBlocks.push_back({ malloc(size), size });
			return arena->mBlocks.back().mBase;
		}

		MemoryBlock& lastBlock = arena->mBlocks.back();
		if (lastBlock.mSize - (reinterpret_cast<uint64_t>(lastBlock.mBase) - reinterpret_cast<uint64_t>(arena->mBlocks.back().mBase)) >= size) {
			return reinterpret_cast<void*>(reinterpret_cast<uint64_t>(lastBlock.mBase) + size);
		}

		arena->mBlocks.push_back({ malloc(size), size });
		return arena->mBlocks.back().mBase;
	}

	void* ArenaPushZero(Arena* arena, uint64_t size) {
		void* ptr = ArenaPush(arena, size);
		memset(ptr, 0, size);
		return ptr;
	}

	// some macro helpers that I've found nice:
#define PushArray(arena, type, count) (type *)ArenaPush((arena), sizeof(type)*(count))
#define PushArrayZero(arena, type, count) (type *)ArenaPushZero((arena), sizeof(type)*(count))
#define PushStruct(arena, type) PushArray((arena), (type), 1)
#define PushStructZero(arena, type) PushArrayZero((arena), (type), 1)

// pop some bytes off the 'stack' - the way to free
	void ArenaPop(Arena* arena, uint64_t size) {
		if (arena->mBlocks.empty()) {
			LOG(LogError) << "Trying to pop from an empty arena";
			return;
		}

		MemoryBlock& lastBlock = arena->mBlocks.back();
		if (lastBlock.mSize - (reinterpret_cast<uint64_t>(lastBlock.mBase) - reinterpret_cast<uint64_t>(arena->mBlocks.back().mBase)) < size) {
			LOG(LogError) << "Trying to pop more bytes than are available in the current block";
			return;
		}

		lastBlock.mSize -= size;
	}

	// get the # of bytes currently allocated.
	uint64_t ArenaGetPos(Arena* arena) {
		if (arena->mBlocks.empty()) {
			return 0;
		}

		MemoryBlock& lastBlock = arena->mBlocks.back();
		return lastBlock.mSize - (reinterpret_cast<uint64_t>(lastBlock.mBase) - reinterpret_cast<uint64_t>(arena->mBlocks.back().mBase));
	}

	// also some useful popping helpers:
	void ArenaSetPosBack(Arena* arena, uint64_t pos) {
		if (arena->mBlocks.empty()) {
			LOG(LogError) << "Trying to set position in an empty arena";
			return;
		}

		MemoryBlock& lastBlock = arena->mBlocks.back();
		if (lastBlock.mSize - (reinterpret_cast<uint64_t>(lastBlock.mBase) - reinterpret_cast<uint64_t>(arena->mBlocks.back().mBase)) < pos) {
			LOG(LogError) << "Trying to set position past the end of the current block";
			return;
		}

		lastBlock.mSize = pos;
	}

	void ArenaClear(Arena* arena) {
		for (MemoryBlock& block : arena->mBlocks) {
			free(block.mBase);
		}
		arena->mBlocks.clear();
	}
}
