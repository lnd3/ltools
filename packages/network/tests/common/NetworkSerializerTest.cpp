#include "testing/Test.h"
#include "logging/Log.h"
#include "logging/String.h"

#include "network/NetworkSerializer.h"

using namespace l;

TEST(NetworkSerializer, Basic) {

	{
		int object[10];
		for (int i = 0; i < 10; i++) {
			object[i] = i;
		}

		network::NetworkSerializer serializer;

		serializer.Serialize([&](network::NetworkSerializer& serializer, std::vector<unsigned char>& data) {
			serializer.AddData(&object[0], sizeof(int) * 10);
			return true;
			});

		int object2[10];
		network::NetworkSerializer deserializer;
		deserializer.Deserialize(serializer.GetData(), [&](std::vector<unsigned char>& data, size_t chunkSize) {
			l::network::NetworkSerializer::MoveData(&object2[0], data, chunkSize);
			return true;
			});

		for (int i = 0; i < 10; i++) {
			TEST_TRUE(object2[i] == i, "");
		}
	}

	return 0;
}

TEST(NetworkSerializer, Complex) {

	{
		std::vector<int> object;
		for (int i = 0; i < 10; i++) {
			object.push_back(i);
		}
		network::NetworkSerializer serializer;
		for (int i = 0; i < 2; i++) {
			serializer.Serialize([&](network::NetworkSerializer& serializer, std::vector<unsigned char>& data) {
				serializer.AddData(object.data(), sizeof(int) * object.size());
				return true;
				});
		}

		TEST_TRUE(serializer.GetData().size() == sizeof(int) * 22, "");

		network::NetworkSerializer deserializer;
		std::vector<int> deserializedData;
		for (int i = 0; i < 2;) {
			auto result = deserializer.Deserialize(serializer.GetData(), [&](std::vector<unsigned char>& data, size_t chunkSize) {
				auto count = chunkSize / sizeof(int32_t);
				for (int i = 0; i < count; i++) {
					auto src = reinterpret_cast<int32_t*>(data.data() + i * sizeof(int32_t));
					deserializedData.push_back(*src);
				}
				return true;
				});
			if (!result) {
				break;
			}
		}

		for (int i = 0; i < deserializedData.size(); i++) {
			TEST_TRUE(deserializedData[i] == i % 10, "");
		}
	}

	return 0;
}