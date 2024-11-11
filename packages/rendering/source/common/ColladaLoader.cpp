#include "rendering/ColladaLoader.h"

#include "logging/LoggingAll.h"
#include "memory/Containers.h"
#include "physics/VecExt.h"
#include "filesystem/File.h"
#include "logging/String.h"

#include <functional>
#include <mutex>
#include <unordered_map>
#include <atomic>
#include <span>


namespace l {
namespace rendering {

	ColladaData LoadColladaAsset(std::string_view file, std::function<void(const ColladaData& colladaData)> handler) {
		ColladaData colladaData;

		pugi::xml_document doc;
		pugi::xml_parse_result result = doc.load_file(file.data());
		if (result) {
			ColladaLoader walker(colladaData);
			doc.traverse(walker);
		}
		if (handler) {
			handler(colladaData);
		}

		return colladaData;
	}

	ColladaLoader::ColladaLoader(ColladaData& colladaNodes) : mColladaNodes(colladaNodes) {};

	bool ColladaLoader::begin(pugi::xml_node&) {
		return true;
	}

	bool ColladaLoader::for_each(pugi::xml_node& node) {
		if (string::equal(node.name(), "COLLADA")) {
			mFoundError = false;
		}
		else if (string::equal(node.name(), "library_lights")) {
		}
		else if (string::equal(node.name(), "library_effects")) {
		}
		else if (string::equal(node.name(), "library_materials")) {
		}
		else if (string::equal(node.name(), "library_geometries")) {
		}

		// geometry node allocation
		else if (string::equal(node.parent().name(), "library_geometries")
			&& string::equal(node.name(), "geometry")) {

			auto id = node.attribute("id").as_string();
			auto name = node.attribute("name").as_string();
			mColladaNodes.mGeometryNodes.emplace_back(GeometryNode{id, name, {}, {}, {} });
		}

		// source node allocation
		else if (string::equal(node.parent().name(), "mesh")
			&& string::equal(node.name(), "source")) {

			auto& geometryNode = mColladaNodes.mGeometryNodes.back();
			auto id = node.attribute("id").as_string();
			geometryNode.mSourceNodes.emplace_back(SourceNode{ id, {FloatArrayNode{}}, {AccessorNode{}} });
		}
		// source content
		else if (string::equal(node.parent().name(), "source")
			&& string::equal(node.name(), "float_array")) {

			auto& sourceNode = mColladaNodes.mGeometryNodes.back().mSourceNodes.back();
			auto id = node.attribute("id").as_string();
			auto count = node.attribute("count").as_uint();
			auto content = node.text().as_string();

			sourceNode.mFloatArray.mId = id;
			sourceNode.mFloatArray.mCount = count;
			sourceNode.mFloatArray.mFloatArray.reserve(count);
			string::cstring_to_numbers<float>(content, count, ' ', sourceNode.mFloatArray.mFloatArray);
		}
		else if (string::equal(node.parent().parent().name(), "source")
			&& string::equal(node.parent().name(), "technique_common")
			&& string::equal(node.name(), "accessor")) {

			auto& sourceNode = mColladaNodes.mGeometryNodes.back().mSourceNodes.back();
			auto count = node.attribute("count").as_uint();
			auto stride = node.attribute("stride").as_uint();
			sourceNode.mAccessorNode = AccessorNode{ count , stride};
		}

		// vertices input mapping
		else if (string::equal(node.parent().parent().name(), "mesh")
			&& string::equal(node.parent().name(), "vertices")
			&& string::equal(node.name(), "input")) {
			auto& geometryNode = mColladaNodes.mGeometryNodes.back();
			auto id = node.parent().attribute("id").as_string();
			auto semantic = node.attribute("semantic").as_string();
			auto source = node.attribute("source").as_string();
			geometryNode.mVerticesInputNodes.emplace_back(VerticesInputNode{ id, semantic, source });
		}

		// triangles node allocation
		else if (string::equal(node.parent().name(), "mesh")
			&& (string::equal(node.name(), "triangles") || string::equal(node.name(), "polylist"))) {

			auto& geometryNode = mColladaNodes.mGeometryNodes.back();
			auto material = node.attribute("material").as_string();
			auto trianglesCount = node.attribute("count").as_uint();
			auto indicesCount = 3 * 3 * trianglesCount;
			geometryNode.mTrianglesNodes.emplace_back(TrianglesNode{ material, indicesCount, {}, {}, {} });
		}
		// triangles content
		else if ((string::equal(node.parent().name(), "triangles") || string::equal(node.parent().name(), "polylist"))
			&& string::equal(node.name(), "input")) {

			auto& trianglesNode = mColladaNodes.mGeometryNodes.back().mTrianglesNodes.back();
			auto semantic = node.attribute("semantic").as_string();
			auto offset = node.attribute("offset").as_uint();
			auto source = node.attribute("source").as_string();
			trianglesNode.mInputNodes.emplace_back(TrianglesInputNode{ semantic, source, offset });
		}
		else if ((string::equal(node.parent().name(), "triangles") || string::equal(node.parent().name(), "polylist"))
			&& string::equal(node.name(), "vcount")) {

			//auto& trianglesNode = mColladaNodes.mGeometryNodes.back().mTrianglesNodes.back();
			//auto content = node.text().as_string();
			//trianglesNode.mIndiceCountPerPrimitive.reserve(trianglesNode.mIndicesCount);
			//string::cstring_to_numbers<uint32_t>(content, trianglesNode.mIndicesCount, ' ', trianglesNode.mIndiceCountPerPrimitive);
			//for (int i = 0; i < trianglesNode.mIndiceCountPerPrimitive.size();i++) {
			//	ASSERT(trianglesNode.mIndiceCountPerPrimitive[i] == 3) << "Collada mesh contains non-triangles and there is no converter yet";
			//}
			//trianglesNode.mIndiceCountPerPrimitive.clear();
		}
		else if ((string::equal(node.parent().name(), "triangles") || string::equal(node.parent().name(), "polylist"))
			&& string::equal(node.name(), "p")) {

			auto& trianglesNode = mColladaNodes.mGeometryNodes.back().mTrianglesNodes.back();
			auto content = node.text().as_string();
			trianglesNode.mIndices.reserve(trianglesNode.mIndicesCount);
			string::cstring_to_numbers<uint32_t>(content, trianglesNode.mIndicesCount, ' ', trianglesNode.mIndices);
		}

		return true;
	}

	bool ColladaLoader::end(pugi::xml_node&) {
		return true;
	}
}
}
