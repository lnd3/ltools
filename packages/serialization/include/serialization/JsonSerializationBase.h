#pragma once

#include <iostream>
#include <serialization/JsonParser.h>
#include <serialization/JsonBuilder.h>

namespace l::serialization {

	class JsonSerializationBase {
	public:
		JsonSerializationBase() {}
		virtual ~JsonSerializationBase() = default;

		virtual bool LoadArchiveData(JsonValue& jsonValue) = 0;
		virtual void GetArchiveData(JsonBuilder& jsonBuilder) = 0;
	};
}
