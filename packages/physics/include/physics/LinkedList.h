#pragma once

#include <algorithm>
#include <array>
#include <memory>
#include <vector>
#include <unordered_map>

#include "math/MathAlgorithm.h"

namespace l::physics {

template<class T>
class LinkedElement {
public:

    void swap(LinkedElement<T>& other) {
        auto& key = this->mKey;
        auto& data = this->mData;

        this->mKey = other.mKey;
        this->mData = other.mData;

        other.mKey = key;
        other.mData = data;
    }

    auto operator<=>(const LinkedElement<T>&) const = default;

    bool operator <(const LinkedElement<T>& other) const {
        return mData < other.mData;
    }

    void clear() {
        mId = 0;
        mNextIndex = 0;
    }

    bool isValid() {
        return mId > 0 && mNextIndex > 0;
    }

    void set(uint32_t id, uint32_t nextIndex) {
        mId = id;
        mNextIndex = nextIndex;
    }

    uint32_t mId;
    uint32_t mNextIndex;

    T mData;
};

template<class T = uint32_t>
class LinkedElements {
public:
    void init(uint32_t size) {
        mElements.resize(size);
        mElementsSize = 0;

        // Add root element
        auto& root = mElements.at(0);
        root.set(0, 1);
        mElementsSize++;
    }

    void add(uint32_t id, T&& data) {
        auto& elementPrev = mElements.at(mElementsSize - 1);
        auto& element = mElements.at(mElementsSize);

        elementPrev.mNextIndex = mElementsSize;

        mElementsSize++;

        element.set(id, mElementsSize);
        element.mData = data;

        // Rewire element links accordingly
    }

    T&& remove(uint32_t id) {
        if (id > 0) {
            auto& element = mElements.at(id);
            if (element.isValid()) {

            }
            element.clear();
            element.mData = {};

            mElementsSize--;
        }
    }

    auto& get(uint32_t id, uint32_t fromIndex) {

        LinkedElement<T> e;
        e.mData = id;

        uint32_t index = math::algorithm::binary_search(mElements, e, fromIndex - 10, fromIndex + 10);

        while (fromIndex < mElementsSize && mElements.at(fromIndex).mId != id) {
            fromIndex++;
        }
        if (fromIndex < mElementsSize) {
            return mElements.at(fromIndex);
        }
        return mElements.at(0);
    }


    bool has(uint32_t id) {
        auto& entry = mElements.at(id);
        return entry.isValid();
    }
protected:
    std::vector<LinkedElement<T>> mElements;
    uint32_t mElementsSize = 0;
};

}
