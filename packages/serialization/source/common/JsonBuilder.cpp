#include "serialization/JsonBuilder.h"

#include <string>
#include <vector>
#include <sstream>

namespace l::serialization {

    void JsonBuilder::Begin(std::string_view name, bool array) {
        if (mNestingItemCount.back() > 1) {
            mJson << ",";
            NewLine();
            Indent();
        }
        BeginNesting();
        if (!name.empty()) {
            mJson << "\"" << name << "\":";
        }
        mJson << (array ? "[" : "{");
        NewLine();
        Indent();
    }
    void JsonBuilder::End(bool array) {
        EndNesting();
        NewLine();
        Indent();
        mJson << (array ? "]" : "}");
    }
    void JsonBuilder::AddString(std::string_view name, std::string_view data) {
        if (mNestingItemCount.back() > 1) {
            mJson << ",";
            Indent();
        }
        mNestingItemCount.back()++;
        if (!name.empty()) {
            mJson << "\"" << name << "\":";
        }
        mJson << "\"" << data << "\"";
    }
    void JsonBuilder::AddString(std::string_view name, std::function<void(std::stringstream& json)> dataGenerator) {
        if (mNestingItemCount.back() > 1) {
            mJson << ",";
            Indent();
        }
        mNestingItemCount.back()++;
        if (!name.empty()) {
            mJson << "\"" << name << "\":";
        }
        mJson << "\"";
        dataGenerator(mJson);
        mJson << "\"";
    }
    void JsonBuilder::Reset() {
        mJson.str("");
        mJson.clear();
    }
    std::stringstream& JsonBuilder::GetStream() {
        mJson.flush();
        return mJson;
    }
    void JsonBuilder::NewLine() {
        if (!mPretty) {
            return;
        }
        mJson << "\n";
    }
    void JsonBuilder::Indent() {
        if (!mPretty) {
            return;
        }
        for (int i = 1; i < mNestingItemCount.size(); i++) {
            mJson << "  ";
        }
    }
    void JsonBuilder::BeginNesting() {
        mNestingItemCount.push_back(1);
    }
    void JsonBuilder::EndNesting() {
        mNestingItemCount.pop_back();
    }

}
