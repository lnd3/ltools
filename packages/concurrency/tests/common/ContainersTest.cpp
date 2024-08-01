#include "testing/Test.h"
#include "logging/Log.h"
#include "logging/String.h"

#include "concurrency/Containers.h"
#include "meta/Reflection.h"

using namespace l;

TEST(Containers, ConcurrentVector) {
	size_t max_threads = 19;
	size_t max_count = 100000;
	std::atomic<size_t> counter = 0;
	container::ConcurrentVector<size_t> vec;

	int64_t total_sum = 0;
	for (size_t i = 0; i < max_count * max_threads; i++) {
		total_sum += i;
	}

	auto vectorStressTester = [&vec, &counter, max_count]() {
		for (size_t i = 0; i < max_count; i++) {
			vec.push_back(counter++);
		}
		};

	{
		std::vector<std::thread> mPoolThreads;
		for (size_t i = 0; i < max_threads; i++) {
			mPoolThreads.push_back(std::thread(vectorStressTester));
		}

		for (size_t i = 0; i < max_threads; i++) {
			mPoolThreads[i].join();
		}
	}

	int64_t threaded_sum = 0;
	for (auto v : vec) {
		threaded_sum += v;
	}

	TEST_EQ(vec.size(), max_threads * max_count, "Failed to push back 10000 values");

	TEST_EQ(total_sum, threaded_sum, "Failed to validate increment sum");

	return 0;
}

TEST(Containers, Polymorphic) {

	class Base {
	public:
		Base() = default;
		virtual ~Base() {
			std::cout << "~Base()" << std::endl;
		}
		int base{ 0 };
	};

	class Derived : public Base {
	public:
		Derived() {
			base = 1;
			derived = 1;
		}
		virtual ~Derived() {
			std::cout << "~Derived()" << std::endl;
		}
		int derived;
	};

	class DoubleDerived : public Derived {
	public:
		DoubleDerived() {
			derived = 2;
			base = 2;
			doublederived = 2;
		}
		virtual ~DoubleDerived() {
			std::cout << "~DoubleDerived()" << std::endl;
		}
		int doublederived;
	};

	{
		auto hash_1 = meta::Type<Base>::hash_code();
		auto hash_2 = meta::Type<Derived>::hash_code();
		auto hash_3 = meta::Type<DoubleDerived>::hash_code();
		auto hash_4 = meta::class_hash<Base>();
		auto hash_5 = meta::class_hash<Derived>();
		auto hash_6 = meta::class_hash<Base>();

		TEST_TRUE(hash_1 != hash_2, "");
		TEST_TRUE(hash_1 != hash_3, "");
		TEST_TRUE(hash_2 != hash_3, "");
		TEST_TRUE(meta::Type<Base>::hash_code() != meta::Type<Derived>::hash_code(), "");
		TEST_TRUE(meta::Type<Base>::hash_code() != meta::Type<DoubleDerived>::hash_code(), "");
		TEST_TRUE(meta::Type<Derived>::hash_code() != meta::Type<DoubleDerived>::hash_code(), "");

		LOG(LogInfo) << meta::Type<Base>::hash_code();
		LOG(LogInfo) << meta::Type<Derived>::hash_code();
		LOG(LogInfo) << meta::Type<DoubleDerived>::hash_code();
	}

	{
		auto storage = container::vector_cc<Base>();
		storage.push_back<Derived>();
		storage.push_back<DoubleDerived>();
		auto d1 = storage.loan<Derived>(0);
		auto d2 = storage.loan<Derived>(1);
		auto d1_2 = storage.loan<DoubleDerived>(0);
		auto d2_2 = storage.loan<DoubleDerived>(1);
		auto d2_2_1 = storage.erase<DoubleDerived>(1);
		// d2 is undefined
	}

	{
		auto storage = container::map_cc<std::string, Base>();
		storage.make<Derived>("d1");
		storage.make<DoubleDerived>("d2");
		auto d1 = storage.loan<Derived>("d1");
		auto d2 = storage.loan<Derived>("d2");
		auto d1_2 = storage.loan<DoubleDerived>("d1");
		auto d2_2 = storage.loan<DoubleDerived>("d2");
		auto d2_2_1 = storage.erase<DoubleDerived>("d2");
		// d2 is undefined
	}

	{
		auto storage = container::map_cc_unique<Base>();
		storage.make<Derived>();
		storage.make<DoubleDerived>();
		{
			auto b1 = storage.get_weak<Base>();
			auto d2 = storage.get_weak<Derived>();
			auto d3 = storage.get<Derived>();
			auto dd2 = storage.get_weak<DoubleDerived>();
			auto dd3 = storage.get<DoubleDerived>();
		}
		storage.erase<Derived>();
		storage.erase<DoubleDerived>();
		storage.make<Derived>();
		storage.make<DoubleDerived>();
	}

	return 0;
}





