#pragma once

#include <iostream>

namespace l::serialization {

	class JsonSerializationBase {
	public:
		JsonSerializationBase() {}
		virtual ~JsonSerializationBase() = default;

		virtual bool LoadArchiveData(std::stringstream& src) = 0;
		virtual void GetArchiveData(std::stringstream& dst) = 0;
	};
}
