#pragma once

#include <memory>
#include <functional>
#include <string>
#include <unordered_map>
#include <map>
#include <vector>
#include <algorithm>			// For lower_bound
#include <typeinfo>
#include <optional>
#include <iterator> // For std::forward_iterator_tag
#include <cstddef>  // For std::ptrdiff_t

#include "logging/Log.h"
#include "meta/Reflection.h"

namespace l::memory {

	struct MemoryBlock {
		void* mBase = nullptr;
		uint64_t mSize = 0u;
	};

	class Arena {
	public:
		Arena();
		Arena(void* ptr, uint64_t size);
		~Arena();

		std::vector<MemoryBlock> mBlocks;
	};

	std::unique_ptr<Arena> CreateArena(void* ptr = nullptr, uint64_t size = 0);

	// push some bytes onto the 'stack' - the way to allocate
	void* ArenaPush(Arena* arena, uint64_t size);
	void* ArenaPushZero(Arena* arena, uint64_t size);

	// some macro helpers that I've found nice:
#define PushArray(arena, type, count) (type *)ArenaPush((arena), sizeof(type)*(count))
#define PushArrayZero(arena, type, count) (type *)ArenaPushZero((arena), sizeof(type)*(count))
#define PushStruct(arena, type) PushArray((arena), (type), 1)
#define PushStructZero(arena, type) PushArrayZero((arena), (type), 1)

// pop some bytes off the 'stack' - the way to free
	void ArenaPop(Arena* arena, uint64_t size);

	// get the # of bytes currently allocated.
	uint64_t ArenaGetPos(Arena* arena);

	// also some useful popping helpers:
	void ArenaSetPosBack(Arena* arena, uint64_t pos);
	void ArenaClear(Arena* arena);
}
