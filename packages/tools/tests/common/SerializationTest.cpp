#include "testing/Test.h"
#include "logging/Log.h"

#include "storage/LocalStore.h"
#include "various/serializer/Serializer.h"

using namespace l;

class point {
public:
	point() = default;
	point(int x, int y, std::string str) noexcept : m_x(x), m_y(y), m_str(str) {}

	friend zpp::serializer::access;
	template <typename Archive, typename Self>
	static void serialize(Archive& archive, Self& self) {
		archive(self.m_x, self.m_y, self.m_str);
	}

	int get_x() const noexcept {
		return m_x;
	}

	int get_y() const noexcept {
		return m_y;
	}

private:
	int m_x = 0;
	int m_y = 0;
	std::string m_str;
};

TEST(Serialization, ZppSerializer) {
	std::vector<unsigned char> data;
	zpp::serializer::memory_output_archive out(data);

	std::stringstream stream;
	auto path = std::filesystem::path(L"./Debug/store/test_serialization.txt");

	{
		out(point(1337, 1338, "abc"));
		out(point(2, 3, "4"));
		out(point(5, 6, "7"));

		storage::convert(stream, data);
		storage::write(path, stream);
	}

	stream.clear();
	data.clear();

	{
		storage::read(path, stream);
		storage::convert(data, stream);

		point my_point0;
		point my_point1;
		point my_point2;

		zpp::serializer::memory_input_archive in(data);
		in(my_point0, my_point1, my_point2);

		std::cout << my_point2.get_x() << ' ' << my_point2.get_y() << '\n';
	}

	return 0;
}
