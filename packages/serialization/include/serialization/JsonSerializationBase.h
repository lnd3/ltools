#pragma once

#include <logging/LoggingAll.h>
#include <various/serializer/Serializer.h>

#define JSMN_PARENT_LINKS
#include <various/jsmn.h>


#include <string_view>
#include <cassert>

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <chrono>
#include <type_traits>

namespace l::serialization {


    class JsonValue;

    class JsonIterator {
    public:
        JsonIterator(const char* json, const jsmntok_t* tokens, int count)
            : json(json), tokens(tokens), count(count), index(0) {
        }

        bool has_next() const {
            return index < count;
        }

        JsonValue next();

    private:
        const char* json;
        const jsmntok_t* tokens;
        int count;
        int index;

        friend class JsonValue;
    };

    class JsonValue {
    public:
        JsonValue(const char* json, const jsmntok_t* tokens, int count)
            : json(json), tokens(tokens), count(count) {
        }

        jsmntype_t type() const {
            return tokens[0].type;
        }

        std::string_view as_string() const {
            assert(type() == JSMN_STRING || type() == JSMN_PRIMITIVE);
            return { json + tokens[0].start, static_cast<size_t>(tokens[0].end - tokens[0].start) };
        }

        bool as_bool() const {
            auto sv = as_string();
            return sv == "true";
        }

        double as_number() const {
            auto sv = as_string();
            return std::strtod(sv.data(), nullptr);
        }

        JsonValue get(const std::string_view& key) const {
            assert(type() == JSMN_OBJECT);
            int size = tokens[0].size;
            int i = 1;
            while (i < count) {
                std::string_view k{ json + tokens[i].start, size_t(tokens[i].end - tokens[i].start) };
                if (k == key) {
                    return JsonValue(json, &tokens[i + 1], count - i - 1);
                }
                // skip key and its value
                i += 1 + skip(&tokens[i + 1]);
            }
            return JsonValue(nullptr, nullptr, 0); // invalid
        }

        JsonIterator as_array() const {
            assert(type() == JSMN_ARRAY);
            return JsonIterator(json, tokens + 1, count - 1);
        }

        bool valid() const { return tokens != nullptr && count > 0; }

    private:
        const char* json;
        const jsmntok_t* tokens;
        int count;

        static int skip(const jsmntok_t* t) {
            int n = 1;
            for (int i = 0; i < t[0].size; ++i) {
                n += skip(t + n);
            }
            return n;
        }

        friend class JsonIterator;
    };

    inline JsonValue JsonIterator::next() {
        assert(has_next());
        JsonValue value(json, tokens + index, count - index);
        index += JsonValue::skip(tokens + index);
        return value;
    }






	class JsonSerializationBase {
	public:
		JsonSerializationBase() {}
		virtual ~JsonSerializationBase() = default;

		virtual bool LoadArchiveData(std::stringstream& src) = 0;
		virtual void GetArchiveData(std::stringstream& dst) = 0;
	};
}
