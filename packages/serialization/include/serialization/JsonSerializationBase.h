#pragma once

#include <iostream>
#include <serialization/JsonParser.h>
#include <serialization/JsonBuilder.h>

namespace l::serialization {

	class StreamSerializationBase {
	public:
		StreamSerializationBase() {}
		virtual ~StreamSerializationBase() = default;

		virtual bool LoadArchiveData(std::stringstream& src) = 0;
		virtual void GetArchiveData(std::stringstream& dst) = 0;
	};

	class JsonSerializationBase {
	public:
		JsonSerializationBase() {}
		virtual ~JsonSerializationBase() = default;

		virtual bool LoadArchiveData(JsonValue& jsonValue) = 0;
		virtual void GetArchiveData(JsonBuilder& jsonBuilder) = 0;
	};

}
