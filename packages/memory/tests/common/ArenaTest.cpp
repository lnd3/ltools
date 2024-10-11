#include "testing/Test.h"
#include "logging/Log.h"

#include "memory/Arena.h"

using namespace l;

TEST(Arena, Basic) {

	auto arena = memory::CreateArena();

	auto a = memory::ArenaPush(arena.get(), 10);

	TEST_TRUE(a != nullptr, "");

	return 0;
}

