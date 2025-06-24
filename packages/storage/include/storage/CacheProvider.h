#pragma once

#include <vector>
#include <string_view>

namespace l::filecache {

	class ICacheProvider {
	public:
		ICacheProvider() = default;
		virtual ~ICacheProvider() = default;
		virtual bool PersistData(std::string_view, const std::vector<unsigned char>&) {
			return false;
		};
		virtual bool ProvideData(std::string_view, std::vector<unsigned char>&) {
			return false;
		};
	};
}