
#pragma once

#include <stdint.h>
#include <set>
#include <vector>
#include <span>
#include <unordered_map>
#include <limits>
#include <cmath>

#include "logging/LoggingAll.h"
#include "physics/VecX.h"
#include "physics/Constants.h"

namespace l::physics {

    template<class T>
    auto DistanceFromDegrees(T degree) {
        auto x = cos((degree / 360.0) * 2.0 * constants::pi);
        auto y = sin((degree / 360.0) * 2.0 * constants::pi);

        return static_cast<T>(sqrt((1.0 - x) * (1.0 - x) + y * y));
    }

    constexpr uint32_t kNeighboursPerDimension[3] = {2, 5, 13};
    constexpr int32_t kNeighbours[13][3] = {
        {0,0,0},                    // current bucket
        {1,0,0},                    // x axis
        {0,1,0}, {1,1,0}, {-1,1,0}, // y axis plus x,y diagonals
        {0,0,1}, {1,0,1}, {-1,0,1}, // z axis plus z,y diagonals
        {0,1,1}, {0,1,-1},          // y,z diagonals
        {1,1,1}, {-1,1,1}, {-1,1,-1}// x,y,z diagonals
    };
    constexpr uint32_t primes[4] = { 137, 149, 163, 181 };

    template<uint32_t stride, class T>
    int32_t ComputeHash(const T* value, T mGridPartitions, const int32_t* offset) {
        uint32_t hash = 0;
        for (uint32_t i = 0; i < stride; i++) {
            auto valueTrunc = static_cast<int32_t>(std::trunc(mGridPartitions * value[i]));
            valueTrunc += offset[i];
            hash += (valueTrunc + primes[i]) * primes[i];
        }
        return hash;
    }

    template<class T>
    T EstimateGridPartitionCount(size_t size) {
        auto estimatedSourceSize = 0.25 * std::trunc(pow(size, (1.0f / 3.0f)));
        if (estimatedSourceSize < static_cast<T>(2.0)) {
            return static_cast<T>(2.0);
        }
        return static_cast<T>(estimatedSourceSize);
    }

    template<uint32_t stride, class T>
    bool ComparePair(const T* element1, const T* element2, T limit) {
        T distance = 0.0;
        for (uint32_t i = 0; i < stride; i++) {
            auto diff = element1[i] - element2[i];
            distance += diff * diff;
        }
        if (distance < limit * limit) {
            return true;
        }
        return false;
    }

    template<uint32_t stride, class T = float>
    class GridMap {
    public:
        GridMap(const std::vector<T>& sourceData)
            : mSourceData(sourceData)
            , mGridPartitionCount(0.0) {
            for (uint32_t j = 0; j < 2 * stride; j++) {
                mGridSize[j] = static_cast<T>(0.0);
            }
        }

        ~GridMap() = default;

        void FillGrid(T maxSearchSize) {
            mGridBuckets.clear();

            for (uint32_t j = 0; j < stride; j++) {
                mGridSize[j * 2] = std::numeric_limits<T>::max();
                mGridSize[j * 2 + 1] = std::numeric_limits<T>::min();
            }

            // Find min/max point cloud volume for finding optimal grid scale
            for (uint32_t i = 0; i < mSourceData.size(); i += stride) {
                for (uint32_t j = 0; j < stride; j++) {
                    auto value = mSourceData[i + j];
                    if (value < mGridSize[2 * j]) {
                        mGridSize[2 * j] = value - static_cast<T>(0.00001);
                    }
                    if (value > mGridSize[2 * j + 1]) {
                        mGridSize[2 * j + 1] = value + static_cast<T>(0.00001);
                    }
                }
            }
            T gridSize = 0.0;
            for (uint32_t j = 0; j < stride; j++) {
                auto size = abs(mGridSize[2 * j + 1] - mGridSize[2 * j]);
                if (gridSize < size) {
                    gridSize = size;
                }
            }

            mGridPartitionCount = EstimateGridPartitionCount<T>(mSourceData.size());

            auto maxGridPartitions = gridSize / maxSearchSize;
            if (mGridPartitionCount > maxGridPartitions) {
                mGridPartitionCount = static_cast<T>(std::trunc(maxGridPartitions));
            }

            for (uint32_t sIndex = 0; sIndex < mSourceData.size(); sIndex += stride) {
                auto sElement = &mSourceData.at(sIndex);
                auto sHash = ComputeHash<stride, T>(
                    sElement,
                    mGridPartitionCount,
                    &kNeighbours[0][0]);

                if (!mGridBuckets.contains(sHash)) {
                    mGridBuckets.emplace(sHash, std::vector<uint32_t>());
                }

                mGridBuckets.at(sHash).push_back(sIndex);
            }
        }

        void FindPairs(T limit, std::function<void(uint32_t i, uint32_t j)> pair) {
            auto count = kNeighboursPerDimension[stride-1];

            for (uint32_t sIndex = 0; sIndex < mSourceData.size(); sIndex += stride) {
                auto sElement = &mSourceData.at(sIndex);

                for (uint32_t neighbourIndex = 0; neighbourIndex < count; neighbourIndex++) {
                    auto bHash = ComputeHash<stride, T>(sElement, mGridPartitionCount, &kNeighbours[neighbourIndex][0]);

                    if (mGridBuckets.contains(bHash)) {
                        auto& bList = mGridBuckets.at(bHash);
                        for (auto bIndex : bList) {
                            if (sIndex >= bIndex) {
                                continue;
                            }

                            auto bElement = &mSourceData.at(bIndex);

                            if (ComparePair<stride, T>(sElement, bElement, limit)) {
                                pair(sIndex/stride, bIndex/stride);
                            }
                        }
                    }
                }
            }
        }

    protected:
        const std::vector<T>& mSourceData;
        std::unordered_map<int32_t, std::vector<uint32_t>> mGridBuckets;
        T mGridSize[stride * 2u];
        T mGridPartitionCount;
    };

}
