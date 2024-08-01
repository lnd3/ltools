#include "testing/Test.h"
#include "logging/Log.h"
#include "logging/String.h"
#include "meta/Reflection.h"

class TestClassGlobal {};
struct TestStructGlobal {};

namespace {
    class TestClassAnon {};
}

namespace TestNS {
    class TestClassA {};
}

TEST(ReflectionTest, ClassIds) {
    auto testClassGlobal = l::meta::class_name<TestClassGlobal>();
    auto testClassAnon = l::meta::class_name<TestClassAnon>();
    auto testClassA = l::meta::class_name<TestNS::TestClassA>();

    auto testStructGlobal = l::meta::class_name<TestStructGlobal>();


    TEST_TRUE(testClassGlobal.size() > 0, "");
    TEST_TRUE(testClassAnon.size() > 0, "");
    TEST_TRUE(testClassA.size() > 0, "");

    return 0;
}

TEST(ReflectionTest, ClassInfo) {
    auto type1 = l::meta::class_type<TestNS::TestClassA>();
    auto ident1 = l::meta::class_identifier<TestNS::TestClassA>();
    auto type2 = l::meta::class_type<TestClassAnon>();
    auto ident2 = l::meta::class_identifier<TestClassAnon>();

    return 0;
}

TEST(ReflectionTest, ClassName) {

    class A {};
    class B {};
    class C {};
    LOG(LogInfo) << l::meta::Type<A>::name();
    LOG(LogInfo) << l::meta::Type<B>::full_name() << "\n";
    LOG(LogInfo) << l::meta::Type<TestClassGlobal>::name() << "\n";
    LOG(LogInfo) << l::meta::Type<A>::full_name() << "\n";
    LOG(LogInfo) << l::meta::Type<C>::name() << "\n";
    LOG(LogInfo) << l::meta::Type<C>::type() << "\n";

    return 0;
}

TEST(ReflectionTest, Polymorphism) {
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
		auto hash_1 = l::meta::Type<Base>::hash_code();
		auto hash_2 = l::meta::Type<Derived>::hash_code();
		auto hash_3 = l::meta::Type<DoubleDerived>::hash_code();
		auto hash_4 = l::meta::class_hash<Base>();
		auto hash_5 = l::meta::class_hash<Derived>();
		auto hash_6 = l::meta::class_hash<Base>();

		TEST_TRUE(hash_1 != hash_2, "");
		TEST_TRUE(hash_1 != hash_3, "");
		TEST_TRUE(hash_2 != hash_3, "");
		TEST_TRUE(l::meta::Type<Base>::hash_code() != l::meta::Type<Derived>::hash_code(), "");
		TEST_TRUE(l::meta::Type<Base>::hash_code() != l::meta::Type<DoubleDerived>::hash_code(), "");
		TEST_TRUE(l::meta::Type<Derived>::hash_code() != l::meta::Type<DoubleDerived>::hash_code(), "");

		LOG(LogInfo) << l::meta::Type<Base>::hash_code();
		LOG(LogInfo) << l::meta::Type<Derived>::hash_code();
		LOG(LogInfo) << l::meta::Type<DoubleDerived>::hash_code();
	}

	return 0;
}



