#include "testing/Test.h"
#include "logging/Log.h"

#include "memory/Containers.h"

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
		auto storage = container::vector<32>(256);
		storage.push_back(Base());
		storage.push_back(Derived());
		storage.push_back(DoubleDerived());

		for (auto it = storage.begin(), end = storage.end(); it != end; ++it) {
			Base* a = it.get<Base>();
			Derived* b = it.get<Derived>();
			DoubleDerived* c = it.get<DoubleDerived>();
			DoubleDerived* d = it.get<DoubleDerived>();
		}
	}

	return 0;
}

