#include "network/NetworkSerializer.h"

#include "logging/LoggingAll.h"

#include <string.h>

namespace l::network {

    void NetworkSerializer::AddFloat(float value) {
        mData.resize(mData.size() + sizeof(float));
        auto p = reinterpret_cast<float*>(mData.data() + mData.size() - sizeof(float));
        *p = value;
    }

    void NetworkSerializer::AddInt(int32_t value) {
        mData.resize(mData.size() + sizeof(int32_t));
        auto p = reinterpret_cast<int32_t*>(mData.data() + mData.size() - sizeof(int32_t));
        *p = value;
    }

    void NetworkSerializer::AddUInt(uint32_t value) {
        mData.resize(mData.size() + sizeof(uint32_t));
        auto p = reinterpret_cast<uint32_t*>(mData.data() + mData.size() - sizeof(uint32_t));
        *p = value;
    }

    void NetworkSerializer::AddData(void* src, size_t size) {
        auto existingSize = mData.size();
        mData.resize(mData.size() + size);
        memcpy(mData.data() + existingSize, src, size);
    }

    float NetworkSerializer::MoveFloat(std::vector<unsigned char>& data) {
        auto p = reinterpret_cast<float*>(data.data());
        auto value = *p;
        data.erase(data.begin(), data.begin() + sizeof(float));
        return value;
    }

    int32_t NetworkSerializer::MoveInt(std::vector<unsigned char>& data) {
        auto p = reinterpret_cast<int32_t*>(data.data());
        auto value = *p;
        data.erase(data.begin(), data.begin() + sizeof(int32_t));
        return value;
    }

    uint32_t NetworkSerializer::MoveUInt(std::vector<unsigned char>& data) {
        auto p = reinterpret_cast<uint32_t*>(data.data());
        auto value = *p;
        data.erase(data.begin(), data.begin() + sizeof(uint32_t));
        return value;
    }

    void NetworkSerializer::MoveData(void* dst, std::vector<unsigned char>& data, size_t size) {
        memcpy(dst, data.data(), size);
        data.erase(data.begin(), data.begin() + size);
    }

    std::vector<unsigned char>& NetworkSerializer::GetData() {
        return mData;
    }

    size_t NetworkSerializer::GetSize() {
        return mData.size();
    }

    bool NetworkSerializer::Serialize(std::function<bool(NetworkSerializer&, std::vector<unsigned char>&)> serialize) {
        mPrevSize = static_cast<int32_t>(mData.size());
        AddUInt(0); // add space for this chunk's size data
        auto result = serialize(*this, mData);
        auto size = mData.size() - mPrevSize - sizeof(int32_t);
        if (size <= 0 || !result) {
            ASSERT(size <= 0);
            mData.resize(mData.size() - sizeof(int32_t));
            return false;
        }
        auto dataPtr = reinterpret_cast<uint32_t*>(mData.data() + mPrevSize);
        *dataPtr = static_cast<uint32_t>(size);

        mPrevSize = static_cast<int32_t>(mData.size());
        mTotalSize += size + sizeof(int32_t); // Always a size int first in every chunk

        return true;
    }

    bool NetworkSerializer::Deserialize(std::vector<unsigned char>& data, std::function<bool(std::vector<unsigned char>&, size_t)> deserialize) {
        if (data.empty()) {
            return false;
        }
        auto chunkSize = MoveUInt(data);
        if (chunkSize == 0) {
            return false;
        }
        ASSERT(chunkSize <= data.size());

        auto size = data.size();
        auto result = deserialize(data, chunkSize);
        if(!result) {
            return false;
        }
        if (size == data.size()) {
            // user did not consume the chunk, delete it manually
            data.erase(data.begin(), data.begin() + chunkSize);
        }
        ASSERT(data.size() == size - chunkSize);
        return true;
    }

}
