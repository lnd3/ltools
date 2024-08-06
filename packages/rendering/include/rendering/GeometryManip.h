#pragma once

#include "logging/LoggingAll.h"
#include "physics/GridMap.h"
#include "physics/VecExt.h"
#include "physics/Algorithm.h"
#include "various/linmathext.h"

#include <vector>
#include <string_view>
#include <optional>
#include <unordered_map>
#include <functional>

namespace l {
namespace rendering {

	typedef uint32_t AttribType;
	constexpr AttribType ATTRIB_TYPE_COORD = 0;
	constexpr AttribType ATTRIB_TYPE_NORMAL = 1;
	constexpr AttribType ATTRIB_TYPE_COLOR = 2;
	constexpr AttribType ATTRIB_TYPE_UV = 3;
	constexpr AttribType ATTRIB_TYPE_TANGENT = 4;

	template<class T>
	struct AttribData {
		uint32_t mOffset = 0;
		std::vector<T> mData;
	};

	template<class T = float>
	struct Mesh {
		std::vector<AttribType> mOrder;
		std::vector<uint32_t> mIndices;
		std::unordered_map<AttribType, AttribData<T>> mAttributes;

		void SetIndices(std::vector<uint32_t>&& indices) {
			mIndices = std::move(indices);
		}

		void Set(AttribType type, std::vector<T>&& data) {
			if (data.empty()) {
				return;
			}
			mOrder.push_back(type);
			mAttributes.emplace(type, AttribData{ 0, std::move(data) });
		}

		std::optional<AttribData<T>&> Get(AttribType type) {
			if (mAttributes.contains(type)) {
				return mAttributes.at(type);
			}
			return std::nullopt;
		}

		size_t AttributeCount() {
			return mAttributes.size();
		}

	};

	template<class T>
	std::vector<T> UnwrapIndexData(const std::vector<T>& sourceIndices, size_t size) {

		std::vector<T> result;
		result.reserve(sourceIndices.size());

		auto length = sourceIndices.size();
		for (size_t i = 0; i < length / 3; i += size) {
			for (size_t j = 0; j < size; j++) { // for every index in the group do
				result.emplace_back(static_cast<T>(i + j));
			}
		}
		return result;
	}

    template<uint32_t stride, class T, class TargetType = T>
    std::vector<T> UnwrapSourceData(
        const std::vector<T>& sourceData,
        const std::vector<uint32_t>& sourceIndices,
        const std::vector<uint32_t>& order,
        uint32_t orderIndex) {

        auto strideOfData = stride;

        std::vector<TargetType> result;
        result.reserve(sourceData.size());

        auto size = order.size();
        auto length = sourceIndices.size() - size * 3;
        for (size_t i = 0; i <= length; i += size * 3) { // for every 3rd groups of indexes do
            for (size_t j = 0; j < size; j++) {
                auto index = i + j * size + order[orderIndex];
                auto sourceIndex = sourceIndices[index];
                auto sourceOffset = sourceIndex * strideOfData;

                for (size_t k = 0; k < strideOfData; k++) { // for every data entry for the index do
                    result.emplace_back(static_cast<TargetType>(sourceData[sourceOffset + k]));
                }
            }
        }
        return result;
    }

	template<uint32_t stride, class T>
	void FindMergableIndices(
		const std::vector<T>& sourceData,
		float limit = 0.1,
		std::function<void(uint32_t i, uint32_t j)> pair = nullptr) {

		l::physics::GridMap<stride, T> grid(sourceData);

		grid.FillGrid(limit);
		grid.FindPairs(limit, pair);
	}

	template<class T>
	std::vector<T> ComputeTangents(
		const std::vector<uint32_t> indices,
		const std::vector<T>& vertices,
		const std::vector<T>& normals,
		const std::vector<T>& uvs
	) {
		size_t verticeCount = vertices.size();
		size_t incideCount = indices.size();

		std::vector<float> tan1;
		tan1.resize(verticeCount);

		std::vector<float> tan2;
		tan2.resize(verticeCount);

		uint32_t stride = 3;
		uint32_t uvStride = 2;

		for (size_t i = 0; i < incideCount; i += stride) {
			size_t i1 = indices[i + 0];
			size_t i2 = indices[i + 1];
			size_t i3 = indices[i + 2];

			float x1 = vertices[stride * i2 + 0] - vertices[stride * i1 + 0];
			float x2 = vertices[stride * i3 + 0] - vertices[stride * i1 + 0];
			float y1 = vertices[stride * i2 + 1] - vertices[stride * i1 + 1];
			float y2 = vertices[stride * i3 + 1] - vertices[stride * i1 + 1];
			float z1 = vertices[stride * i2 + 2] - vertices[stride * i1 + 2];
			float z2 = vertices[stride * i3 + 2] - vertices[stride * i1 + 2];

			float s1 = uvs[uvStride * i2 + 0] - uvs[uvStride * i1 + 0];
			float s2 = uvs[uvStride * i3 + 0] - uvs[uvStride * i1 + 0];
			float t1 = uvs[uvStride * i2 + 1] - uvs[uvStride * i1 + 1];
			float t2 = uvs[uvStride * i3 + 1] - uvs[uvStride * i1 + 1];

			float r = 1.0f / (s1 * t2 - s2 * t1);
			float sdirX = (t2 * x1 - t1 * x2) * r;
			float sdirY = (t2 * y1 - t1 * y2) * r;
			float sdirZ = (t2 * z1 - t1 * z2) * r;
			float tdirX = (s1 * x2 - s2 * x1) * r;
			float tdirY = (s1 * y2 - s2 * y1) * r;
			float tdirZ = (s1 * z2 - s2 * z1) * r;

			tan1[stride * i1 + 0] += sdirX;
			tan1[stride * i1 + 1] += sdirY;
			tan1[stride * i1 + 2] += sdirZ;
			tan1[stride * i2 + 0] += sdirX;
			tan1[stride * i2 + 1] += sdirY;
			tan1[stride * i2 + 2] += sdirZ;
			tan1[stride * i3 + 0] += sdirX;
			tan1[stride * i3 + 1] += sdirY;
			tan1[stride * i3 + 2] += sdirZ;

			tan2[stride * i1 + 0] += tdirX;
			tan2[stride * i1 + 1] += tdirY;
			tan2[stride * i1 + 2] += tdirZ;
			tan2[stride * i2 + 0] += tdirX;
			tan2[stride * i2 + 1] += tdirY;
			tan2[stride * i2 + 2] += tdirZ;
			tan2[stride * i3 + 0] += tdirX;
			tan2[stride * i3 + 1] += tdirY;
			tan2[stride * i3 + 2] += tdirZ;
		}

		std::vector<float> tangent;
		tangent.resize((stride + 1)*(verticeCount / stride));

		int tangentIndex = 0;
		for (size_t i = 0; i < verticeCount; i+=stride) {
			// Gram-Schmidt orthogonalize
			T dot1 = vec3_mul_inner(&normals[i], &tan1[i]);
			vec3 tmp;
			vec3_scale(tmp, &normals[i], dot1);
			vec3_sub(tmp, &tan1[i], tmp);
			vec3_norm(tmp, tmp);
			for (size_t j = 0; j < 3; j++) {
				tangent[tangentIndex + j] = tmp[j];
			}


			vec3_mul_cross(tmp, &normals[i], &tangent[i]);
			float dot2 = vec3_mul_inner(tmp, &tan2[i]);
			if (dot2 == 0) {
				LOG(LogWarning) << "Vertex index " << i << " has an undefined bitangent handedness because:";
				dot2 = 1.0;
			}
			if (dot2 < 0) {
				tangent[tangentIndex + 3] = -1.0;
			}
			else {
				tangent[tangentIndex + 3] = 1.0;
			}

			tangentIndex += stride + 1;

			// Gram-Schmidt orthogonalize
			//tangent[a] = (t - n * Dot(n, t)).Normalize();

			// Calculate handedness
			//tangent[a].w = (Dot(Cross(n, t), tan2[a]) < 0.0F) ? -1.0F : 1.0F;
		}

		return tangent;
	}


}
}