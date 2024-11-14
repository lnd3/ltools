#pragma once

#include "logging/String.h"

#include <string>
#include <vector>
#include <sstream>
#include <deque>


namespace l::serialization {

    class JsonBuilder {

    public:
        JsonBuilder(bool pretty = false) : mPretty(pretty) {
            BeginNesting();
        }
        ~JsonBuilder() = default;

        void Begin(std::string_view name, bool array = false);
        void End(bool array = false);
        void AddString(std::string_view name, std::string_view data);
        void AddString(std::string_view name, std::function<void(std::stringstream& json)> dataGenerator);

        template<class T>
        void AddNumber(std::string_view name, T value, bool asString = false) {
            if (mNestingItemCount.back() > 0) {
                mJson << ",";
                Indent();
            }
            mNestingItemCount.back()++;
            if (!name.empty()) {
                mJson << "\"" << name << "\":";
            }
            if (asString) {
                mJson << "\"" << std::to_string(value) << "\"";
            }
            else {
                mJson << std::to_string(value);
            }
        }
        void Reset();
        std::stringstream& GetStream();
    protected:
        void NewLine();
        void Indent();
        void BeginNesting();
        void EndNesting();

        bool mPretty = false;
        std::deque<int32_t> mNestingItemCount;
        std::stringstream mJson;
    };
}
