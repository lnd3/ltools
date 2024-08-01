#include <memory>
#include <sstream>
#include <conio.h>
#include <mutex>
#include <vector>
#include <functional>
#include <iostream>
#include <Windows.h>

#include "logging/Log.h"
#include "testing/Test.h"
#include "logging/String.h"
#include "ecs/experimental/Entity.h"

using namespace l;


TEST(ExperimentalEntity, Storage)
{
	{
		auto storage = ecs::Entities<8>(100);
		auto it = storage.begin();

	}

	return 0;
}

TEST(ExperimentalEntity, EntitiesMaxSize) {
	{
		auto storage = ecs::Entities<8>(100);
		auto it2 = storage.begin();

		ecs::Data<9> e0;
		ecs::Data<11> e1;
		ecs::Data<14> e2;

		LOG(LogInfo) << e0.size();
		LOG(LogInfo) << e1.size();
		LOG(LogInfo) << e2.size();

		e0.data()[e0.size() - 1] = 1;
		e1.data()[e1.size() - 1] = 1;
		e2.data()[e2.size() - 1] = 1;


		LOG(LogInfo) << "id:" << storage.push_back(std::move(e0));
		LOG(LogInfo) << "id:" << storage.push_back(std::move(e1));
		LOG(LogInfo) << "id:" << storage.push_back(std::move(e2));

		LOG(LogInfo) << "Block size: " << storage.block_size() << ", Byte count: " << storage.byte_count();

		auto it = storage.begin();

		auto a = ecs::Data<9>();
		a.fill(3);
		TEST_FALSE(storage.scary_set(it, ecs::Data<253>()), "");
		TEST_TRUE(storage.scary_set(it, a), "");

	}
	return 0;
}

TEST(ExperimentalEntity, Container) {
	{
		auto storage = ecs::Entities<3>(256);
		ecs::Element<10> e0;
		ecs::Element<6> e1;
		ecs::Element<3> e2;
		ecs::Element<4> e3;

		LOG(LogInfo) << "sizeof(ecs::Element<10>):" << std::to_string(sizeof(ecs::Element<10>));

		e0.data()[e0.size() - 1] = 1;
		e1.data()[e1.size() - 1] = 1;
		e2.data()[e2.size() - 1] = 1;
		e3.data()[e3.size() - 1] = 1;

		LOG(LogInfo) << "id:" << storage.push_back(std::move(e0));
		LOG(LogInfo) << "id:" << storage.push_back(std::move(e1));
		LOG(LogInfo) << "id:" << storage.push_back(std::move(e2));
		LOG(LogInfo) << "id:" << storage.push_back(std::move(e3));
		LOG(LogInfo) << "id:" << storage.push_back(ecs::Element<15>());

		LOG(LogInfo) << "Block size: " << storage.block_size() << ", Byte count: " << storage.byte_count();

		auto it = storage.begin();
		TEST_EQ(it.size<ecs::Element<10>>(), 1, "Wrong size");
		TEST_EQ(it.size<uint8_t>(), 10u, "Wrong size");
		it++;
		TEST_EQ(it.size<ecs::Element<6>>(), 1, "Wrong size");
		TEST_EQ(it.size<uint8_t>(), 6u, "Wrong size");
		it++;
		TEST_EQ(it.size<ecs::Element<3>>(), 1, "Wrong size");
		TEST_EQ(it.size<uint8_t>(), 3u, "Wrong size");
		it++;
		TEST_EQ(it.size<ecs::Element<4>>(), 1, "Wrong size");
		TEST_EQ(it.size<uint8_t>(), 4u, "Wrong size");

		it = storage.begin();
		it++;
		it++;
		it++;
		auto e4_v1 = it.size<uint8_t>();
		storage.erase(it);
		auto e4_v2 = it.size<uint8_t>();

		LOG(LogInfo) << "Expect an error on the next line. Passing to much data to scary_set..";
		TEST_FALSE(storage.scary_set(it, ecs::Element<5>()), "");
		TEST_TRUE(storage.scary_set(it, ecs::Element<3>()), "");
		auto e4_v3 = it.size<uint8_t>();
		storage.erase(it);

		for (auto it = storage.begin(); it != storage.end(); ++it) {
			size_t size = it.size<uint8_t>();
			auto e = it.get<ecs::Element<3>>();
			std::stringstream str("");
			str << "ptr=" << "0x" + string::to_hex(reinterpret_cast<uintptr_t>(it.get<uint8_t>())) << ", ";
			str << storage.get_id(it) << ", ";
			str << string::to_fixed_string<' '>(static_cast<int>(size), 2) + " bytes";
			str << ": (element size " + std::to_string(size);
			str << ", data: [";
			for (size_t i = 0; i < size; i++) {
				if (i > 0) str << ",";
				if (it.get<uint8_t>(i) != nullptr) {
					str << std::to_string(*it.get<uint8_t>(i));
				}
				else {
					str << "nullptr";
				}
			}
			str << "])";
			LOG(LogInfo) << str.str();
		}

		auto it0 = storage.at(0);
		TEST_TRUE(it0 == storage.begin(), "");
		auto it1 = storage.at(3);
		TEST_TRUE(it1 == storage.end(), "");
		auto it2 = storage.at(4);
		TEST_FALSE(it2 == storage.end(), "");

	}

	return 0;
}
