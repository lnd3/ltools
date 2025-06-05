#pragma once

#include <logging/LoggingAll.h>
#include <various/serializer/Serializer.h>

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <chrono>
#include <type_traits>

namespace l::serialization {

	class JsonSerializationBase {
	public:
		JsonSerializationBase() {}
		virtual ~JsonSerializationBase() = default;

		virtual bool LoadArchiveData(std::stringstream& src) = 0;
		virtual void GetArchiveData(std::stringstream& dst) = 0;
	};
}
