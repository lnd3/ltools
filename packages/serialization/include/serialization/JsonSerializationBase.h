#pragma once

#include <logging/LoggingAll.h>
#include <various/serializer/Serializer.h>

#define JSMN_HEADER
#include <various/jsmn.h>

#include <string_view>
#include <cassert>
#include <cstdlib>

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
        JsonIterator(const char* json, const jsmntok_t* tokens, int remaining)
            : json(json), tokens(tokens), remaining(remaining), index(0) {
        }

        bool has_next() const {
            return index < remaining;
        }

        JsonValue next();

    private:
        const char* json;
        const jsmntok_t* tokens;
        int remaining;
        int index;

        friend class JsonValue;
    };

    class JsonValue {
    public:
        JsonValue() : mJson(nullptr), mTokens(nullptr), mCount(0) {}

        JsonValue(const char* json, const jsmntok_t* tokens, int count)
            : mJson(json), mTokens(tokens), mCount(count) {
        }

        bool valid() const;
        jsmntype_t type() const;
        std::string_view as_string() const;
        bool as_bool() const;
        double as_double() const;
        float as_float() const;
        int32_t as_int32() const;
        int64_t as_int64() const;
        JsonIterator as_array() const;
        
        JsonValue get(const std::string_view& key) const;
        
        const char* start_ptr() const;
        const char* end_ptr() const;

        const jsmntok_t* end_token() const;

        bool has(jsmntype_t t) const;
        bool has_key(const std::string_view& key) const;

        int size() const;
        int numBytes() const;

        JsonValue operator[](int index) const;

        static int skip(const jsmntok_t* t) {
            int n = 1;
            if (t->type & JSMN_OBJECT) {
                int pairs = t->size;
                const jsmntok_t* cur = t + 1;
                for (int i = 0; i < pairs; ++i) {
                    n += skip(cur); cur += skip(cur);
                    n += skip(cur); cur += skip(cur);
                }
            }
            else if (t->type & JSMN_ARRAY) {
                const jsmntok_t* cur = t + 1;
                for (int i = 0; i < t->size; ++i) {
                    n += skip(cur);
                    cur += skip(cur);
                }
            }
            return n;
        }

    private:
        const char* mJson = nullptr;
        const jsmntok_t* mTokens = nullptr;
        int mCount = 0;

        friend class JsonIterator;
    };

    inline JsonValue JsonIterator::next() {
        assert(index < remaining);
        JsonValue v(json, tokens + index, remaining - index);
        int span = JsonValue::skip(tokens + index);
        index += span;
        return v;
    }

    template<int32_t MaxTokens = 1000>
    class JsonRoot {
    public:
        JsonRoot() {}

        bool LoadJson(const char* jsondata, size_t size) {
            mJsondata = jsondata;

            jsmn_init(&mParser);

            auto ret = jsmn_parse(&mParser, mJsondata, size, mTokens, MaxTokens);
            if (ret < 0) {
                /*
                JSMN_ERROR_INVAL - bad token, JSON string is corrupted
                JSMN_ERROR_NOMEM - not enough tokens, JSON string is too large
                JSMN_ERROR_PART - JSON string is too short, expecting more JSON data
                */

                mTokenCount = 0;
                switch (ret) {
                case JSMN_ERROR_INVAL:
                    LOG(LogError) << "Failure to parse json value";
                    return false;
                case JSMN_ERROR_NOMEM:
                    LOG(LogError) << "Token buffer is to small";
                    return false;
                case JSMN_ERROR_PART:
                    //LOG(LogInfo) << "Json data is not completed";
                    return true;
                default:
                    LOG(LogError) << "Unknown error";
                    return false;
                }
            }
            else {
                mTokenCount = ret;
            }
            return true;
        }

        JsonValue GetRoot() {
            return JsonValue(mJsondata, mTokens, mTokenCount);
        }

    protected:
        const char* mJsondata = nullptr;
        jsmn_parser mParser;
        int32_t mTokenCount = 0;
        jsmntok_t mTokens[MaxTokens];
    };

	class JsonSerializationBase {
	public:
		JsonSerializationBase() {}
		virtual ~JsonSerializationBase() = default;

		virtual bool LoadArchiveData(std::stringstream& src) = 0;
		virtual void GetArchiveData(std::stringstream& dst) = 0;
	};
}
