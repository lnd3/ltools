#pragma once

#include "logging/LoggingAll.h"
#include "physics/GridMap.h"

#include <vector>
#include <string_view>
#include <set>
#include <optional>

namespace l {
namespace rendering {

    template<class TargetType, class SourceInnerType, uint32_t FloatToIntMultipler = 1, class SourceType>
    auto ConvertInnerType(const SourceType* src, size_t size) {
        std::vector<TargetType> result;

        if (src == nullptr) {
            return result;
        }

        SourceType* src2 = const_cast<SourceType*>(src);

        auto vSize = sizeof(SourceType);
        auto sSize = sizeof(SourceInnerType);
        auto innerCount = vSize / sSize;
        result.reserve(size * innerCount);
        for (int i = 0; i < size; i++) {
            SourceInnerType* innerTypePtr = reinterpret_cast<SourceInnerType*>(src2);
            for (int j = 0; j < innerCount; j++) {
                SourceInnerType& value = *innerTypePtr++;
                if constexpr (FloatToIntMultipler == 1) {
                    result.emplace_back(static_cast<TargetType>(value));
                }
                else {
                    result.emplace_back(static_cast<TargetType>(value * FloatToIntMultipler));
                }
            }
            src2++;
        }
        return result;
    }

    template<class TargetType, class SourceInnerType, uint32_t FloatToIntMultipler = 1, class SourceType, class SourceIndexType>
    auto ConvertIndexedInnerType(const SourceIndexType* srcIndex, size_t size, const SourceType* src) {
        std::vector<TargetType> result;

        if (src == nullptr) {
            return result;
        }

        auto src2 = const_cast<SourceType*>(src);
        auto indexPtr = const_cast<SourceIndexType*>(srcIndex);

        auto vSize = sizeof(SourceType);
        auto sSize = sizeof(SourceInnerType);
        auto innerCount = vSize / sSize;

        for (int i = 0; i < size; i++) {
            SourceIndexType index = *indexPtr;
            SourceInnerType* innerTypePtr = reinterpret_cast<SourceInnerType*>(src2 + index);
            for (int j = 0; j < innerCount; j++) {
                SourceInnerType& value = *innerTypePtr++;
                if constexpr (FloatToIntMultipler == 1) {
                    result.emplace_back(static_cast<TargetType>(value));
                }
                else {
                    result.emplace_back(static_cast<TargetType>(value * FloatToIntMultipler));
                }
            }


            indexPtr++;
        }

        return result;
    }

    template<class TargetType, class SourceInnerType, class SourceType, class IndexType>
    std::vector<TargetType> ConvertIndexedToIndexed(
        const IndexType* dstIndex,
        size_t dstIndexSize,
        const IndexType* srcIndex,
        size_t srcIndexSize,
        const SourceType* src,
        size_t srcSize
    ) {
        if (src == nullptr || srcSize == 0) {
            return {};
        }

        std::vector<TargetType> result;
        result.resize(dstIndexSize + dstIndexSize / 2);

        auto src2 = const_cast<SourceType*>(src);

        auto vSize = sizeof(SourceType);
        auto sSize = sizeof(SourceInnerType);
        auto innerCount = vSize / sSize;

        size_t dstSize = 0;

        for (int i = 0; i < srcIndexSize; i++) {
            IndexType index0 = *const_cast<IndexType*>(srcIndex + (i < srcIndexSize ? i : srcIndexSize - 1));
            IndexType index1 = *const_cast<IndexType*>(dstIndex + i);
            dstSize = index1 > dstSize ? index1 + 1 : dstSize;
            SourceInnerType* innerTypePtr = reinterpret_cast<SourceInnerType*>(src2 + index0);
            if (index1 * innerCount >= result.size()) {
                LOG(LogError) << "Index too large?";
                result.resize((index1 + index1 / 4) * innerCount);
            }
            for (int j = 0; j < innerCount; j++) {
                SourceInnerType& value = *innerTypePtr++;
                result[index1 * innerCount + j] = static_cast<TargetType>(value);
            }
        }

        result.resize(dstSize * innerCount);

        return result;
    }

    template<class SourceInnerType, class TargetType, class SourceType>
    void ConvertAndInterleave(std::vector<TargetType>& result, const SourceType* src, size_t sourceSize, size_t stride, size_t offset) {
        if (src == nullptr) {
            return;
        }

        SourceType* srcPtr = const_cast<SourceType*>(src);

        auto vSize = sizeof(SourceType);
        auto sSize = sizeof(SourceInnerType);
        auto innerCount = vSize / sSize;

        auto targetIndex = offset;

        { // result.size / stride => num vertices >= sourceSize to fix result buffer
            ASSERT(result.size() >= sourceSize * stride) << "Failed to interleave attributes, required size : " << (sourceSize * stride) << " (actual: " << result.size() << ")";
        }

        for (int i = 0; i < sourceSize; i++) {
            SourceInnerType* innerTypePtr = reinterpret_cast<SourceInnerType*>(srcPtr);
            auto targetIndexCount = targetIndex;
            for (int j = 0; j < innerCount; j++) {
                SourceInnerType& value = *innerTypePtr++;
                result[targetIndexCount++] = static_cast<TargetType>(value);
            }
            srcPtr++;
            targetIndex += stride;
        }
    }
}
}