#include "rendering/Geometry.h"
#include "logging/LoggingAll.h"

#include "memory/Containers.h"
#include "physics/VecExt.h"
#include "filesystem/File.h"
#include "logging/String.h"
#include "rendering/ColladaLoader.h"

#include <functional>
#include <mutex>
#include <unordered_map>
#include <atomic>
#include <span>

/*
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags
*/

namespace l {
    namespace rendering {
		GpuFormats get_format_type(VertexFormats format) {
			switch (format) {
			case VertexFormats::TANGENT_4:
				return GpuFormats::FLOAT_4;
			case VertexFormats::POSITION_3:
			case VertexFormats::NORMAL_3:
			case VertexFormats::TANGENT_3:
			case VertexFormats::BITANGENT_3:
			case VertexFormats::JOINT_WEIGHTS_3:
				return GpuFormats::FLOAT_3;
			case VertexFormats::UV_MAP0_2:
			case VertexFormats::UV_MAP1_2:
			case VertexFormats::UV_MAP2_2:
			case VertexFormats::NORMAL_2:
			case VertexFormats::TANGENT_2:
			case VertexFormats::BITANGENT_2:
				return GpuFormats::FLOAT_2;
			case VertexFormats::DIFFUSE_1:
			case VertexFormats::SPECULAR_1:
			case VertexFormats::JOINT_INDEXES_1:
				return GpuFormats::BYTES_4;
			default:
				return GpuFormats::FLOAT_4;
			};
		}

		size_t get_format_size(VertexFormats format) {
			switch (format) {
			case VertexFormats::TANGENT_4:
				return 4;
			case VertexFormats::POSITION_3:
			case VertexFormats::NORMAL_3:
			case VertexFormats::TANGENT_3:
			case VertexFormats::BITANGENT_3:
			case VertexFormats::JOINT_WEIGHTS_3:
				return 3;
			case VertexFormats::UV_MAP0_2:
			case VertexFormats::UV_MAP1_2:
			case VertexFormats::UV_MAP2_2:
			case VertexFormats::NORMAL_2:
			case VertexFormats::TANGENT_2:
			case VertexFormats::BITANGENT_2:
				return 2;
			case VertexFormats::DIFFUSE_1:
			case VertexFormats::SPECULAR_1:
			case VertexFormats::JOINT_INDEXES_1:
				return 1;
			default:
				return 4;
			};
		}

		/*
		bool Mesh::AssimpImport(std::string_view file) {
			// Create an instance of the Importer class
			Assimp::Importer importer;

			// And have it read the given file with some example postprocessing
			// Usually - if speed is not the most important aspect for you - you'll
			// probably to request more postprocessing than we do in this example.
			const aiScene* scene = importer.ReadFile(file.data(),
				aiProcess_CalcTangentSpace |
				aiProcess_Triangulate |
				aiProcess_JoinIdenticalVertices |
				aiProcess_SortByPType);

			// If the import failed, report it
			if (nullptr == scene) {
				LOG(LogError) << importer.GetErrorString();
				return false;
			}



			// We're done. Everything will be cleaned up by the importer destructor
			return true;
		}
		*/

		bool Mesh::Load(std::string_view file) {
			if (file.substr(file.size() - 4) != ".dae") {
				return false;
			}

			LoadColladaAsset(file, [](const ColladaData& colladaData) {
				for (auto& geometry : colladaData.mGeometryNodes) {
					LOG(LogInfo) << "Model '" << geometry.mName << "' was successfully loaded.";
				}

				});

			return true;
		}

		bool Mesh::Save(std::string_view file) {
			l::filesystem::File f(file);
			f.modeWrite();
			if (!f.open()) {
				return false;
			}

			std::stringstream ostr;
			for (auto coord : mVertices) {
				ostr << coord << ", ";
			}
			f.write(ostr);
			f.flush();

			return true;
		}

		void Mesh::LoadFromSources(
			std::span<uint32_t> order,
			std::span<uint32_t> indices,
			std::span<float> vertices,
			std::optional<std::span<uint32_t>> normals,
			std::optional<std::span<float>> mapUV_T
		) {
			mVertices.clear();
			mIndices.clear();
			mIndices.reserve(indices.size());

			size_t normalOffs = 0;
			size_t uv_t_offs = 0;
			size_t stride = 3;

			mFormat.emplace_back(VertexFormats::POSITION_3);

			if (normals != std::nullopt) {
				mFormat.emplace_back(VertexFormats::NORMAL_3);
				normalOffs = stride;
				stride += 3;

			}
			if (mapUV_T != std::nullopt) {
				mFormat.emplace_back(VertexFormats::UV_MAP0_2);
				uv_t_offs = stride;
				stride += 2;
			}

			auto indexStride = static_cast<int>(order.size());
			size_t indexVertices;
			size_t index;
			size_t offs;
			size_t offs2;
			size_t offs3;

			auto numVertices = vertices.size() / 3;
			mVertices.resize(numVertices * stride);

			for (int j = 0; j < static_cast<int>(indices.size()); j += indexStride) {
				index = indices[j + order[0]];
				offs = index * stride;
				offs3 = index * 3;

				mIndices.emplace_back(static_cast<uint32_t>(index));

				mVertices[offs + 0] = vertices[offs3 + 0];
				mVertices[offs + 1] = vertices[offs3 + 1];
				mVertices[offs + 2] = vertices[offs3 + 2];
			}

			int orderIndex = 1;

			if (normals != std::nullopt) {
				auto& values = *normals;
				for (int j = 0; j < static_cast<int>(indices.size()); j += indexStride) {
					indexVertices = indices[j + order[0]];
					index = indices[j + order[orderIndex]];
					offs = indexVertices * stride + normalOffs;
					offs3 = index * 3;
					mVertices[offs + 0] = static_cast<float>(values[offs3 + 0]);
					mVertices[offs + 1] = static_cast<float>(values[offs3 + 1]);
					mVertices[offs + 2] = static_cast<float>(values[offs3 + 2]);
				}
			}
			if (mapUV_T != std::nullopt) {
				auto& values = *mapUV_T;
				orderIndex++;
				for (int j = 0; j < static_cast<int>(indices.size()); j += indexStride) {
					indexVertices = indices[j + order[0]];
					index = indices[j + order[orderIndex]];
					offs = indexVertices * stride + uv_t_offs;
					offs2 = index * 2;
					mVertices[offs + 0] = values[offs2 + 0];
					mVertices[offs + 1] = 1 - values[offs2 + 1];
				}
			}
			//if (normals != std::nullopt) {
			//	ComputeTangentsInplace(mVertices, numVertices, mIndices, mIndices.size() / 3, stride, 0u, uv_t_offs, normalOffs, tangent_offs);
			//}
		}

		void Mesh::ComputeTangentsInplace(
			std::span<float> vertices, 
			size_t verticeCount,
			std::span<uint32_t> indices,
			size_t indicesCount,
			size_t vertexStride,
			size_t vertexOffs,
			size_t uvOffs,
			size_t normalOffs,
			size_t tangentOffs
		) {
			std::vector<float> tan1, tan2;
			tan1.reserve(3 * verticeCount);
			tan2.reserve(3 * verticeCount);

			for (int i = 0; i < indicesCount; i++)
			{
				size_t i1 = indices[3 * i + 0];
				size_t i2 = indices[3 * i + 1];
				size_t i3 = indices[3 * i + 2];

				float x1 = vertices[vertexStride * i2 + vertexOffs + 0] - vertices[vertexStride * i1 + vertexOffs + 0];
				float x2 = vertices[vertexStride * i3 + vertexOffs + 0] - vertices[vertexStride * i1 + vertexOffs + 0];
				float y1 = vertices[vertexStride * i2 + vertexOffs + 1] - vertices[vertexStride * i1 + vertexOffs + 1];
				float y2 = vertices[vertexStride * i3 + vertexOffs + 1] - vertices[vertexStride * i1 + vertexOffs + 1];
				float z1 = vertices[vertexStride * i2 + vertexOffs + 2] - vertices[vertexStride * i1 + vertexOffs + 2];
				float z2 = vertices[vertexStride * i3 + vertexOffs + 2] - vertices[vertexStride * i1 + vertexOffs + 2];

				float s1 = vertices[vertexStride * i2 + uvOffs + 0] - vertices[vertexStride * i1 + uvOffs + 0];
				float s2 = vertices[vertexStride * i3 + uvOffs + 0] - vertices[vertexStride * i1 + uvOffs + 0];
				float t1 = vertices[vertexStride * i2 + uvOffs + 1] - vertices[vertexStride * i1 + uvOffs + 1];
				float t2 = vertices[vertexStride * i3 + uvOffs + 1] - vertices[vertexStride * i1 + uvOffs + 1];

				float r = 1.0f / (s1 * t2 - s2 * t1);
				float sdirX = (t2 * x1 - t1 * x2) * r;
				float sdirY = (t2 * y1 - t1 * y2) * r;
				float sdirZ = (t2 * z1 - t1 * z2) * r;
				float tdirX = (s1 * x2 - s2 * x1) * r;
				float tdirY = (s1 * y2 - s2 * y1) * r;
				float tdirZ = (s1 * z2 - s2 * z1) * r;

				tan1[3 * i1 + 0] += sdirX;
				tan1[3 * i1 + 1] += sdirY;
				tan1[3 * i1 + 2] += sdirZ;
				tan1[3 * i2 + 0] += sdirX;
				tan1[3 * i2 + 1] += sdirY;
				tan1[3 * i2 + 2] += sdirZ;
				tan1[3 * i3 + 0] += sdirX;
				tan1[3 * i3 + 1] += sdirY;
				tan1[3 * i3 + 2] += sdirZ;

				tan2[3 * i1 + 0] += tdirX;
				tan2[3 * i1 + 1] += tdirY;
				tan2[3 * i1 + 2] += tdirZ;
				tan2[3 * i2 + 0] += tdirX;
				tan2[3 * i2 + 1] += tdirY;
				tan2[3 * i2 + 2] += tdirZ;
				tan2[3 * i3 + 0] += tdirX;
				tan2[3 * i3 + 1] += tdirY;
				tan2[3 * i3 + 2] += tdirZ;
			}

			std::vector<float> bitangent;
			std::vector<float> tangent;
			std::vector<float> normal;

			for (int i = 0; i < verticeCount; i++)
			{
				// Gram-Schmidt orthogonalize
				auto index0 = vertexStride * i + normalOffs;
				auto index1 = vertexStride * i + tangentOffs;

				// Compute tangent
				float dot1 = l::vec::InnerProduct<float>(
					vertices.subspan(index0, 3),
					std::span(tan1.begin() + 3 * i, 3));

				l::vec::Scale<float>(
					std::span(tangent.begin(), 3), 
					vertices.subspan(index0, 3),
					dot1);

				l::vec::Subtract<float>(
					std::span(tangent.begin(), 3), 
					vertices.subspan(index0, 3),
					std::span(tangent.begin(), 3));

				l::vec::Normalize<float>(vertices.subspan(index1, 3), std::span(tangent.begin(), 3));

				// Compute bitangent from tangent
				l::vec::Cross<float>(
					std::span(bitangent.begin(), 3),
					vertices.subspan(index0, 3),
					std::span(tangent.begin(), 3)
					);

				float dot2 = l::vec::InnerProduct<float>(
					std::span(bitangent.begin(), 3),
					std::span(tan2.begin() + 3 * i, 3));

				// Calculate handedness
				if (dot2 == 0) {
					LOG(LogWarning) << "Vertex index " << i << " has an undefined bitangent handedness because:";
					if (l::vec::IsZeroVector<float>(vertices.subspan(index0, 3))) {
						LOG(LogError) << "  Normal is zero";
					}
					if (l::vec::IsZeroVector(vertices.subspan(index1, 3))) {
						LOG(LogWarning) << "  Tangent is zero";
					}
					if (l::vec::IsZeroVector(std::span(tan2.begin() + 3 * i, 3))) {
						LOG(LogWarning) << "  Bitangent is zero";
					}
				}
				if (dot2 < 0) {
					vertices[index0 + 3] = -1;
				}
				else {
					vertices[index0 + 3] = 1;
				}

				// Gram-Schmidt orthogonalize
				//tangent[a] = (t - n * Dot(n, t)).Normalize();

				// Calculate handedness
				//tangent[a].w = (Dot(Cross(n, t), tan2[a]) < 0.0F) ? -1.0F : 1.0F;
			}
		}


	}
}
