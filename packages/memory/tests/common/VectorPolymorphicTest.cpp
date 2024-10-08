#include "testing/Test.h"
#include "logging/Log.h"

#include "memory/VectorPolymorphic.h"

using namespace l;

TEST(Container, Polymorphic) {

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
		static_assert(sizeof(Base) <= 16);
		static_assert(sizeof(Derived) <= 24);
		static_assert(sizeof(DoubleDerived) <= 32);
		auto storage = container::VectorPolymorphic<32>(256);
		storage.push_back(Base());
		storage.push_back(Derived());
		storage.push_back(DoubleDerived());

		auto a = storage[0].get<Base>();
		TEST_TRUE(a != nullptr, "");
		TEST_TRUE(a->base == 0, "");

		auto b = storage[1].get<Derived>();
		TEST_TRUE(b != nullptr, "");
		TEST_TRUE(b->base == 1, "");
		TEST_TRUE(b->derived == 1, "");

		auto c = storage[2].get<DoubleDerived>();
		TEST_TRUE(c != nullptr, "");
		TEST_TRUE(c->base == 2, "");
		TEST_TRUE(c->derived == 2, "");
		TEST_TRUE(c->doublederived == 2, "");
	}

	return 0;
}

