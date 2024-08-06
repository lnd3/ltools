#pragma once

#include <stdint.h>
#include <vector>
#include <functional>

namespace l::network {

    class NetworkSerializer {

    public:
        NetworkSerializer() : mPrevSize(0), mTotalSize(0) {
        }
        ~NetworkSerializer() {}

        void AddFloat(float value);
        void AddInt(int32_t value);
        void AddUInt(uint32_t value);
        void AddData(void* src, size_t size);

        std::vector<unsigned char>& GetData();

        size_t GetSize();

        bool Serialize(std::function<bool(NetworkSerializer&, std::vector<unsigned char>&)> serializer);
        
        static float MoveFloat(std::vector<unsigned char>& data);
        static int32_t MoveInt(std::vector<unsigned char>& data);
        static uint32_t MoveUInt(std::vector<unsigned char>& data);
        static void MoveData(void* dst, std::vector<unsigned char>& data, size_t size);

        static bool Deserialize(std::vector<unsigned char>& data, std::function<bool(std::vector<unsigned char>&, size_t)> deserializer);
    protected:
        std::vector<unsigned char> mData;

        size_t mPrevSize;
        size_t mTotalSize;
    };

}