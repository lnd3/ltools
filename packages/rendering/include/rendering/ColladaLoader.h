#pragma once

#include "logging/LoggingAll.h"

#include "pugixml/src/pugixml.hpp"

#include <string_view>
#include <memory>
#include <functional>
#include <optional>
#include <tuple>
#include <span>
#include <array>
#include <vector>

namespace l {
namespace rendering {

	struct FloatArrayNode {
		std::string mId;
		uint32_t mCount = 0;
		std::vector<float> mFloatArray;
	};

	struct AccessorNode {
		uint32_t mCount = 0;
		uint32_t mStride = 0;
	};

	struct VerticesInputNode {
		std::string mVerticesId;
		std::string mSemantic;
		std::string mSource;
	};

	struct SourceNode {
		std::string mId;

		FloatArrayNode mFloatArray;
		AccessorNode mAccessorNode; // In technique_common node
	};

	struct TrianglesInputNode {
		std::string mSemantic;
		std::string mSource;
		uint32_t mOffset = 0;
	};

	struct TrianglesNode {
		std::string mMaterial;
		uint32_t mIndicesCount = 0;

		std::vector<TrianglesInputNode> mInputNodes;
		std::vector<uint32_t> mIndiceCountPerPrimitive; // Only used when polylists are defined 
		std::vector<uint32_t> mIndices;
	};

	struct GeometryNode {
		std::string mId;
		std::string mName;

		std::vector<SourceNode> mSourceNodes; // Nested in mesh node
		std::vector<VerticesInputNode> mVerticesInputNodes;
		std::vector<TrianglesNode> mTrianglesNodes; // Nested in mesh node

		std::vector<float> GetSource(std::string_view sourceId) const {
			for (auto& source : mSourceNodes) {
				if (source.mId == sourceId) {
					return source.mFloatArray.mFloatArray;
				}
			}
			return {};
		}

		std::string GetSourceId(uint32_t index, std::string_view type) const {
			auto polylistNode = mTrianglesNodes.at(index);

			std::string verticesInputNodeId = "";
			for (auto& trianglesInputNode : polylistNode.mInputNodes) {
				if (trianglesInputNode.mSemantic == type) {
					verticesInputNodeId = trianglesInputNode.mSource.substr(1);
				}
			}

			if (verticesInputNodeId.empty()) {
				return "";
			}

			std::string trianglesInputNodeSourceId = verticesInputNodeId;
			if (type == "VERTEX") {
				for (auto& inputNode : mVerticesInputNodes) {
					if (inputNode.mSemantic == "POSITION" && inputNode.mVerticesId == verticesInputNodeId) {
						trianglesInputNodeSourceId = inputNode.mSource.substr(1);
					}
				}
			}
			return trianglesInputNodeSourceId;
		}

		std::vector<uint32_t> GetOrder(uint32_t index = 0) const {
			auto& triangleNode = mTrianglesNodes.at(index);

			std::vector<uint32_t> order;
			for (auto& inputNode : triangleNode.mInputNodes) {
				order.emplace_back(inputNode.mOffset);
			}
			return order;
		}

		std::vector<float> GetVertices(uint32_t index = 0) const {
			auto sourceId = GetSourceId(index, "VERTEX");
			return GetSource(sourceId);
		}

		std::vector<uint32_t> GetIndices(uint32_t index = 0) const {
			return mTrianglesNodes.at(index).mIndices;
		}

		std::vector<float> GetNormals(uint32_t index = 0) const {
			auto sourceId = GetSourceId(index, "NORMAL");
			return GetSource(sourceId);
		}

		std::vector<float> GetColors(uint32_t index = 0) const {
			auto sourceId = GetSourceId(index, "COLOR");
			return GetSource(sourceId);
		}

		std::vector<float> GetUVs(uint32_t index = 0) const {
			auto sourceId = GetSourceId(index, "TEXCOORD");
			return GetSource(sourceId);
		}
	};

	struct ColladaData {
		std::vector<GeometryNode> mGeometryNodes;
	};

	ColladaData LoadColladaAsset(std::string_view file, std::function<void(const ColladaData& colladaData)> handler = nullptr);

	class ColladaLoader : public pugi::xml_tree_walker {
	public:
		ColladaLoader(ColladaData& colladaNodes);
		virtual ~ColladaLoader() = default;

		virtual bool begin(pugi::xml_node&);
		virtual bool for_each(pugi::xml_node& node);
		virtual bool end(pugi::xml_node&);

	private:
		bool mFoundError = true;
		std::string mErrorMessage = "";
		ColladaData& mColladaNodes;
	};


}
}