// Must include this first
#include <various/jsmn.h>

#include <serialization/JsonSerializationBase.h>

#include <jsonxx/jsonxx.h>
#include <logging/LoggingAll.h>

#include <string>
#include <vector>
#include <array>
#include <iostream>
#include <ctime>

namespace l::serialization {




    bool JsonValue::valid() const { return mTokens && mCount > 0; }

    jsmntype_t JsonValue::type() const {
        return mTokens[0].type;
    }

    std::string_view JsonValue::as_string() const {
        assert(type() & JSMN_STRING || type() & JSMN_PRIMITIVE);
        return { mJson + mTokens[0].start, static_cast<size_t>(mTokens[0].end - mTokens[0].start) };
    }

    bool JsonValue::as_bool() const {
        auto s = as_string();
        return s == "true";
    }

    double JsonValue::as_double() const {
        return std::strtod(as_string().data(), nullptr);
    }

    float JsonValue::as_float() const {
        return static_cast<float>(std::strtod(as_string().data(), nullptr));
    }

    int32_t JsonValue::as_int32() const {
        return std::strtol(as_string().data(), nullptr, 10);
    }

    int64_t JsonValue::as_int64() const {
        return std::strtoll(as_string().data(), nullptr, 10);
    }

    JsonIterator JsonValue::as_array() const {
        assert(type() & JSMN_ARRAY);
        return JsonIterator(mJson, mTokens + 1, mCount - 1);
    }

    JsonValue JsonValue::get(const std::string_view& key) const {
        assert(type() & JSMN_OBJECT);
        int i = 1;
        while (i + 1 < mCount) {
            auto k = std::string_view(mJson + mTokens[i].start, mTokens[i].end - mTokens[i].start);
            int val_span = skip(&mTokens[i + 1]);

            if (k == key) {
                return JsonValue(mJson, &mTokens[i + 1], val_span);
            }
            i += 1 + val_span;
        }
        return {};
    }

    const char* JsonValue::start_ptr() const {
        return mJson + mTokens[0].start;
    }

    const char* JsonValue::end_ptr() const {
        const jsmntok_t* end = mTokens + skip(mTokens);
        return mJson + end[-1].end;
    }

    const jsmntok_t* JsonValue::end_token() const {
        return mTokens + skip(mTokens);
    }

    bool JsonValue::has(jsmntype_t t) const {
        return valid() && mTokens[0].type == t;
    }

    bool JsonValue::has_key(const std::string_view& key) const {
        if (!has(JSMN_OBJECT)) return false;
        int i = 1;
        while (i + 1 < mCount) {
            auto k = std::string_view(mJson + mTokens[i].start, mTokens[i].end - mTokens[i].start);
            if (k == key) return true;
            i += 1 + skip(&mTokens[i + 1]);
        }
        return false;
    }

    bool JsonValue::is_null(const std::string_view& key) const {
        return has_key(key) && get(key).has(JSMN_PRIMITIVE) && get(key).as_string() == "null";
    }

    int JsonValue::size() const {
        if (!valid()) return 0;
        return mTokens[0].size;
    }

    int JsonValue::numBytes() const {
        if (!valid()) return 0;
        return mTokens[0].end - mTokens[0].start;
    }

    JsonValue JsonValue::operator[](int index) const {
        assert(type() == JSMN_ARRAY);
        if (index < 0 || index >= size()) return {};

        const jsmntok_t* current = mTokens + 1;
        int i = 0;
        while (i < index) {
            int skip_n = skip(current);
            current += skip_n;
            ++i;
        }

        int span = skip(current);
        return JsonValue(mJson, current, span);
    }




}
