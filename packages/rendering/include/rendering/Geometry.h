#pragma once

#include "logging/LoggingAll.h"

#include <string_view>
#include <memory>
#include <functional>
#include <optional>
#include <tuple>
#include <span>
#include <array>
#include <vector>

#define GLAD_GL_IMPLEMENTATION
#include <glad/glad.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace l {
namespace rendering {

	enum class GpuFormats {
		FLOAT_2 = 2,
		FLOAT_3 = 3,
		FLOAT_4 = 4,
		BYTES_4 = 1
	};

	enum class VertexFormats {
		POSITION_3 = 0,
		UV_MAP0_2, // usually texture map
		UV_MAP1_2, // usually normal map
		UV_MAP2_2, // usually light map
		DIFFUSE_1,
		SPECULAR_1,
		NORMAL_3,
		NORMAL_2,
		TANGENT_4,
		TANGENT_3,
		TANGENT_2,
		BITANGENT_3,
		BITANGENT_2,
		JOINT_INDEXES_1,
		JOINT_WEIGHTS_3
	};

	GpuFormats get_format_type(VertexFormats format);
	size_t get_format_size(VertexFormats format);

	class Mesh {
	public:

		//bool AssimpImport(std::string_view file);
			
		bool Load(std::string_view file);
		bool Save(std::string_view file);

		/*
		void GetGeometries(const pugi::xml_node& geometryNode);
		*/

		void LoadFromSources(
			std::span<uint32_t> order,
			std::span<uint32_t> indices,
			std::span<float> vertices,
			std::optional<std::span<uint32_t>> normals,
			std::optional<std::span<float>> mapUV_T
		);

		void ComputeTangentsInplace(
			std::span<float> vertices,
			size_t verticeCount,
			std::span<uint32_t> indices,
			size_t indicesCount,
			size_t vertexStride,
			size_t vertexOffs,
			size_t uvOffs,
			size_t normalOffs,
			size_t tangentOffs);

	private:
		std::vector<float> mVertices;
		std::vector<uint32_t> mIndices;
		std::vector<VertexFormats> mFormat;

	};
}
}