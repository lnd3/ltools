#include "rendering/GLFWRenderVAO.h"
#include "rendering/GLFWWindow.h"
#include "rendering/GLFWSupport.h"
#include "logging/LoggingAll.h"
#include "physics/GridMap.h"
#include "rendering/GeometryManip.h"
#include "rendering/DataConversion.h"

#include "ufbx/ufbx.h"
#include "openfbx/src/ofbx.h"


namespace l {
    namespace rendering {

        size_t GetAttributeTotalStride(const std::vector<Attribute>& attributes) {
            size_t stride = 0;
            for (const auto& attribute : attributes) {
                stride += attribute.mStride;
            }
            return stride;
        }

        size_t GetAttributeTotalSizeInBytes(const std::vector<Attribute>& attributes) {
            size_t bytes = 0;
            for (const auto& attribute : attributes) {
                if (attribute.mData != nullptr && attribute.mTotalSizeInBytes > 0 && !attribute.mAttributeName.empty()) {
                    bytes += attribute.mTotalSizeInBytes;
                }
            }
            return bytes;
        }

        std::unique_ptr<ufbx_scene, void(*)(ufbx_scene*)> LoadUFBXMesh(std::string_view filename) {
            l::filesystem::File f(filename);
            f.modeRead().modeBinary();
            if (f.open()) {
                auto count = f.fileSize();

                ofbx::u8* buffer = new ofbx::u8[count];
                auto actuallyRead = f.read(buffer, count);

                ASSERT(actuallyRead == count) << "Failed to read all bytes";

                ufbx_load_opts flags{ };
                ufbx_error error;
                auto mesh = std::unique_ptr<ufbx_scene, void(*)(ufbx_scene*)>(ufbx_load_memory(buffer, count, &flags, &error), ufbx_free_scene);
                //oldTruckScene2 = std::unique_ptr<ofbx::IScene>(ofbx::load(buffer, static_cast<int>(count), 0));

                delete[] buffer;

                return mesh;
            }
            return {nullptr, ufbx_free_scene };
        }

        std::unique_ptr<RenderVAO> CreateRenderVAO() {
            return std::make_unique<RenderVAO>();
        }

        std::unique_ptr<RenderVAO> CreateUFBXMesh(l::rendering::ShaderProgram& program, 
            const ufbx_mesh& mesh, 
            bool saveData,
            bool useNormals, 
            bool useColors, 
            bool useUVs,
            bool useTangents
            ) {
            auto r = l::rendering::CreateRenderVAO();

            auto vertices = l::rendering::ConvertInnerType<float, double>(mesh.vertex_position.values.data, mesh.vertex_position.values.count);
            r->SetPositions3D(vertices.data(), vertices.size());

            auto indices = l::rendering::ConvertInnerType<uint32_t, uint32_t>(mesh.vertex_indices.data, mesh.vertex_indices.count);
            r->SetIndices(true, indices.data(), indices.size());

            std::vector<float> normals;
            if (useNormals) {
                if (mesh.vertex_normal.indices.data != nullptr) {
                    normals = l::rendering::ConvertIndexedToIndexed<float, double>(
                        mesh.vertex_indices.data,
                        mesh.vertex_indices.count,
                        mesh.vertex_normal.indices.data,
                        mesh.vertex_normal.indices.count,
                        mesh.vertex_normal.values.data,
                        mesh.vertex_normal.values.count);
                }
                else {
                    normals = l::rendering::ConvertInnerType<float, double>(
                        mesh.vertex_normal.values.data,
                        mesh.vertex_normal.values.count);
                }
                r->SetNormals3D(normals.data(), normals.size());
            }
            std::vector<float> colors;
            if (useColors) {
                if (mesh.vertex_color.indices.data != nullptr) {
                    colors = l::rendering::ConvertIndexedToIndexed<float, double>(
                        mesh.vertex_indices.data,
                        mesh.vertex_indices.count,
                        mesh.vertex_color.indices.data,
                        mesh.vertex_color.indices.count,
                        mesh.vertex_color.values.data,
                        mesh.vertex_color.values.count);
                }
                else {
                    colors = l::rendering::ConvertInnerType<float, double>(
                        mesh.vertex_color.values.data,
                        mesh.vertex_color.values.count);
                }
                r->SetColors(colors.data(), colors.size());
            }
            std::vector<float> uvs;
            if (useUVs) {
                if (mesh.vertex_uv.indices.data != nullptr) {
                    uvs = l::rendering::ConvertIndexedToIndexed<float, double>(
                        mesh.vertex_indices.data,
                        mesh.vertex_indices.count,
                        mesh.vertex_uv.indices.data,
                        mesh.vertex_uv.indices.count,
                        mesh.vertex_uv.values.data,
                        mesh.vertex_uv.values.count);

                    /*
                    uvs.resize(mesh.vertex_position.values.count * 3);
                    for (int i = 0; i < mesh.vertex_indices.count; i++) {

                        //uv_index = vertex_uv.indices[index];
                        //vertex_uv.data[uv_index];

                        uint32_t index = mesh.vertex_indices.data[i];
                        const ufbx_vec2& value = mesh.vertex_uv[i];
                        if (uvs[index*2] != static_cast<float>(value.x)) {
                            LOG(LogError) << "";
                        }
                        if (uvs[index * 2+1] < static_cast<float>(value.y)) {
                            LOG(LogError) << "";
                        }
                        uvs[index * 2 + 0] = static_cast<float>(value.x);
                        uvs[index * 2 + 1] = static_cast<float>(value.y);
                    }
                    */
                }
                else {
                    uvs = l::rendering::ConvertIndexedInnerType<float, double>(
                        mesh.vertex_indices.data,
                        mesh.vertex_indices.count,
                        mesh.vertex_uv.values.data);
                }
                r->SetUVs2D(uvs.data(), uvs.size());
            }
            std::vector<float> tangents;
            if (useTangents) {
                if (mesh.vertex_tangent.indices.data != nullptr) {
                    tangents = l::rendering::ConvertIndexedToIndexed<float, double>(
                        mesh.vertex_indices.data,
                        mesh.vertex_indices.count,
                        mesh.vertex_tangent.indices.data,
                        mesh.vertex_tangent.indices.count,
                        mesh.vertex_tangent.values.data,
                        mesh.vertex_tangent.values.count);
                }
                else {
                    tangents = l::rendering::ConvertInnerType<float, double>(
                        mesh.vertex_tangent.values.data,
                        mesh.vertex_tangent.values.count);
                }
                r->SetTangents3D(tangents.data(), tangents.size());
            }

            r->Configure(program);
            r->Upload();

            if (saveData) {
                SaveGeometryData(mesh.name.data, "uvertices", vertices, 3);
                SaveGeometryData(mesh.name.data, "uindices", indices, 3);
                SaveGeometryData(mesh.name.data, "unormals", normals, 3);
                SaveGeometryData(mesh.name.data, "ucolors", colors, 4);
                SaveGeometryData(mesh.name.data, "uuvs", uvs, 2);
                SaveGeometryData(mesh.name.data, "utangents", tangents, 3);
            }
            return r;
        };

        std::unique_ptr<RenderVAO> CreateOFBXMesh(l::rendering::ShaderProgram& program,
            const ofbx::Mesh& mesh,
            bool saveData,
            bool useNormals,
            bool useColors,
            bool useUVs
        ) {

            auto r = l::rendering::CreateRenderVAO();
            auto& geometryData = mesh.getGeometryData();
            auto indices = l::rendering::ConvertInnerType<uint32_t, int>(geometryData.getPositions().indices, geometryData.getPositions().count);
            r->SetIndices(true, indices.data(), indices.size());

            auto vertices = l::rendering::ConvertInnerType<float, float>(geometryData.getPositions().values, geometryData.getPositions().values_count);
            r->SetPositions3D(vertices.data(), vertices.size());

            std::vector<float> normals;
            if (useNormals) {
                if (geometryData.getPositions().indices != nullptr) {
                    normals = l::rendering::ConvertIndexedToIndexed<float, float>(
                        geometryData.getPositions().indices,
                        geometryData.getPositions().count,
                        geometryData.getPositions().indices,
                        geometryData.getPositions().count,
                        geometryData.getNormals().values,
                        geometryData.getNormals().values_count);
                }
                else {
                    normals = l::rendering::ConvertInnerType<float, float>(
                        geometryData.getNormals().values,
                        geometryData.getNormals().count);
                }
                r->SetNormals3D(normals.data(), normals.size());
            }
            std::vector<float> colors;
            if (useColors) {
                if (geometryData.getColors().indices != nullptr) {
                    colors = l::rendering::ConvertIndexedToIndexed<float, float>(
                        geometryData.getPositions().indices,
                        geometryData.getPositions().count,
                        geometryData.getColors().indices,
                        geometryData.getColors().count,
                        geometryData.getColors().values,
                        geometryData.getColors().count);
                }
                else {
                    colors = l::rendering::ConvertInnerType<float, float>(
                        geometryData.getColors().values,
                        geometryData.getColors().count);
                }
                r->SetColors(colors.data(), colors.size());
            }
            std::vector<float> uvs;
            if (useUVs) {
                if (geometryData.getUVs().indices != nullptr) {
                    uvs = l::rendering::ConvertIndexedToIndexed<float, float>(
                        geometryData.getPositions().indices,
                        geometryData.getPositions().count,
                        geometryData.getUVs().indices,
                        geometryData.getUVs().count,
                        geometryData.getUVs().values,
                        geometryData.getUVs().count);
                }
                else {
                    uvs = l::rendering::ConvertInnerType<float, float>(
                        geometryData.getUVs().values,
                        geometryData.getUVs().count);
                }
                r->SetUVs2D(uvs.data(), uvs.size());
            }
            if (saveData) {
                SaveGeometryData(mesh.name, "vertices", vertices, 3);
                SaveGeometryData(mesh.name, "indices", indices, 3);
                SaveGeometryData(mesh.name, "normals", normals, 3);
                SaveGeometryData(mesh.name, "colors", colors, 4);
                SaveGeometryData(mesh.name, "uvs", uvs, 2);
            }

            r->Configure(program);
            r->Upload();

            return r;
        }

        std::unique_ptr<RenderVAO> CreateColladaMesh(l::rendering::ShaderProgram& program,
            const l::rendering::GeometryNode& geometry,
            bool saveData,
            bool useNormals,
            bool useColors,
            bool useUVs
        ) {
            uint32_t geometryIndex = 0;

            std::vector<uint32_t> indices2;
            std::vector<float> vertices2;
            std::vector<float> normals2;
            std::vector<float> colors2;
            std::vector<float> uvs2;
            std::vector<float> tangents2;

            auto order = geometry.GetOrder(geometryIndex);
            auto indices = geometry.GetIndices(geometryIndex);
            auto vertices = geometry.GetVertices(geometryIndex);
            auto normals = geometry.GetNormals(geometryIndex);
            auto colors = geometry.GetColors(geometryIndex);
            auto uvs = geometry.GetUVs(geometryIndex);

            {
                indices2 = l::rendering::UnwrapIndexData(indices, order.size());
            }
            uint32_t orderIndex = 0;
            {
                vertices2 = l::rendering::UnwrapSourceData<3>(vertices, indices, order, orderIndex);
                orderIndex++;
            }

            if (useNormals && !normals.empty()) {
                normals2 = l::rendering::UnwrapSourceData<3>(normals, indices, order, orderIndex);
                orderIndex++;
            }
            if (useColors && !colors.empty()) {
                colors2 = l::rendering::UnwrapSourceData<4>(colors, indices, order, orderIndex);
                orderIndex++;
            }
            if (useUVs && !uvs.empty()) {
                uvs2 = l::rendering::UnwrapSourceData<2>(uvs, indices, order, orderIndex);
                orderIndex++;

                if (useNormals) {
                    tangents2 = l::rendering::ComputeTangents(indices2, vertices2, normals2, uvs2);
                }
            }

            //Mesh mesh;
            //mesh.SetIndices(std::move(indices2));
            //mesh.Set(ATTRIB_TYPE_COORD, std::move(vertices2));
            //mesh.Set(ATTRIB_TYPE_NORMAL, std::move(normals2));
            //mesh.Set(ATTRIB_TYPE_COLOR, std::move(colors2));
            //mesh.Set(ATTRIB_TYPE_UV, std::move(uvs2));
            //mesh.Set(ATTRIB_TYPE_TANGENT, std::move(tangents2));

            std::set<uint64_t> verticePairs;

            l::rendering::FindMergableIndices<3>(vertices2, 0.000001f, 
                [&](uint32_t i, uint32_t j) {
                    auto id = l::algorithm::pairIndex32(i, j);
                    verticePairs.emplace(id);
                });


            auto AccumulatePairs = [](const std::set<uint64_t>& pairs, std::vector<float>& data, uint32_t stride) {
                uint32_t index = INT32_MAX;
                float count = 0;
                float tmp[4];

                for (auto it : pairs) {
                    auto i = stride * static_cast<uint32_t>(it & INT32_MAX);
                    auto j = stride * static_cast<uint32_t>(it >> 32);

                    if (index == i) { // accumulate normals in junction
                        for (uint32_t k = 0; k < stride; k++) {
                            tmp[k] += data[j + k];
                        }
                        count++;
                    }
                    else {
                        if (index != INT32_MAX) {
                            for (uint32_t k = 0; k < stride; k++) {
                                data[index + k] = tmp[k] / count;
                            }
                        }
                        for (uint32_t k = 0; k < stride; k++) {
                            tmp[k] = data[i + k] + data[j + k];
                        }
                        index = i;
                        count = 2;
                    }
                }
            };

            auto CopyPairs = [](const std::set<uint64_t>& pairs, std::vector<float>& data, uint32_t stride) {
                uint32_t index = INT32_MAX;
                float tmp[4] = {};

                for (auto it : pairs) {
                    auto i = stride * static_cast<uint32_t>(it & INT32_MAX);
                    auto j = stride * static_cast<uint32_t>(it >> 32);

                    if (index == i) {
                        for (uint32_t k = 0; k < stride; k++) {
                            data[j + k] = tmp[k];
                        }
                    }
                    else {
                        for (uint32_t k = 0; k < stride; k++) {
                            tmp[k] = data[i + k];
                            data[j + k] = tmp[k];
                        }
                        index = i;
                    }
                }
            };

            //AccumulatePairs(verticePairs, vertices2, 3);
            AccumulatePairs(verticePairs, normals2, 3);
            //AccumulatePairs(verticePairs, tangents2, 3);

            //CopyPairs(verticePairs, vertices2, 3);
            CopyPairs(verticePairs, normals2, 3);
            //CopyPairs(verticePairs, tangents2, 3);


            auto r = l::rendering::CreateRenderVAO();
            r->SetPositions3D(vertices2.data(), vertices2.size());
            r->SetIndices(true, indices2.data(), indices2.size());
            if (!normals2.empty()) {
                r->SetNormals3D(normals2.data(), normals2.size());
            }
            if (!colors2.empty()) {
                r->SetColors(colors2.data(), colors2.size());
            }
            if (!uvs2.empty()) {
                r->SetUVs2D(uvs2.data(), uvs2.size());

                if (useNormals && !tangents2.empty()) {
                    r->SetTangents3D<float, 4>(tangents2.data(), tangents2.size());
                }
            }

            if (saveData) {
                SaveGeometryData(geometry.mName, "collada_vertices", vertices2, 3);
                SaveGeometryData(geometry.mName, "collada_indices", indices2, 3);
                SaveGeometryData(geometry.mName, "collada_normals", normals2, 3);
                SaveGeometryData(geometry.mName, "collada_colors", colors2, 4);
                SaveGeometryData(geometry.mName, "collada_uvs", uvs2, 2);
            }

            r->Configure(program);
            r->Upload();

            return r;
        }

        Render::Render() {
        }

        Render::~Render() {
            auto deleteBuffer = [](GLuint& bufferId) {
                if (bufferId != GL_INVALID_INDEX) {
                    GL_CALL(glDeleteBuffers(1, &bufferId));
                }
            };
            deleteBuffer(mVBO);
            deleteBuffer(mEBO);
            GL_CALL(glDeleteVertexArrays(1, &mVAO));

        }

        void Render::UpdateModel(mat4x4 transform) {
            mat4x4_dup(mTransform, transform);
        }

        void Render::Upload() {
            if (!mDirty) {
                return;
            }
            mDirty = false;
            Create();

            GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(mVBO)));
            if (mInterleaved) {
                GL_CALL(glNamedBufferData(
                    mVBO,
                    static_cast<GLsizeiptr>(mInterleavedAttribute.mTotalSizeInBytes),
                    mInterleavedAttribute.mData,
                    GL_STATIC_DRAW));
            }
            else {
                size_t offsetInBytes = 0;
                for (auto& attribute : mAttributes) {
                    if (attribute.mData != nullptr) {
                        GL_CALL(glNamedBufferSubData(mVBO,
                            static_cast<GLintptr>(offsetInBytes),
                            static_cast<GLsizeiptr>(attribute.mTotalSizeInBytes),
                            attribute.mData));
                        offsetInBytes += attribute.mTotalSizeInBytes;
                    }
                }
            }

            if (mIndicesBuffered) {
                if (mEBO != GL_INVALID_VALUE) {
                    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLuint>(mEBO)));
                    GL_CALL(glNamedBufferData(
                        mEBO,
                        static_cast<GLsizeiptr>(mIndicesCount * mIndicesAttribSize),
                        mIndicesBase,
                        GL_STATIC_DRAW));
                }
            }
        }

        void Render::Configure(ShaderProgram& program) {
            if (!mDirty) {
                return;
            }
            Create();

            GL_CALL(glBindVertexArray(mVAO));
            GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(mVBO)));
            if (mIndicesBuffered) {
                GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLuint>(mEBO)));
            }
            for (auto& attribute : mAttributes) {
                auto attributeId = program.GetAttributeLocation(attribute.mAttributeName);
                if (attributeId >= 0) {
                    GL_CALL(glEnableVertexAttribArray(attributeId));
                }
            }
            if (mInterleaved) {
                size_t strideOffset = 0;
                auto totalStride = GetAttributeTotalStride(mAttributes);
                for (auto& attribute : mAttributes) {
                    auto attributeId = program.GetAttributeLocation(attribute.mAttributeName);
                    if (attributeId >= 0) {
                        GL_CALL(glVertexAttribPointer(
                            static_cast<GLuint>(attributeId),
                            static_cast<GLint>(attribute.mSize),
                            GL_FLOAT, GL_FALSE,
                            static_cast<GLsizei>(totalStride), (void*)(strideOffset)));
                        strideOffset += attribute.mStride;
                    }
                }
            }
            else {
                auto totalBytes = GetAttributeTotalSizeInBytes(mAttributes);
                GL_CALL(glNamedBufferData(mVBO, static_cast<GLsizeiptr>(totalBytes), NULL, GL_STATIC_DRAW));

                size_t dataOffset = 0;
                for (auto& attribute : mAttributes) {
                    auto attributeId = program.GetAttributeLocation(attribute.mAttributeName);
                    if (attributeId >= 0) {
                        GL_CALL(glVertexAttribPointer(
                            static_cast<GLuint>(attributeId), static_cast<GLint>(attribute.mSize),
                            GL_FLOAT, GL_FALSE,
                            static_cast<GLsizei>(attribute.mStride), (void*)(dataOffset)));
                        dataOffset += attribute.mTotalSizeInBytes;
                    }
                }
            }
        }

        void Render::SetTexture(uint32_t index, GLuint texture) {
            switch (index) {
            case 0:
                mTexture0 = texture;
                break;
            case 1:
                mTexture1 = texture;
                break;
            case 2:
                mTexture2 = texture;
                break;
            case 3:
                mTexture3 = texture;
                break;
            }
        }

        void Render::Draw(ShaderProgram& program) {
            if (mDirty) {
                Upload();
                mDirty = false;
            }

            if (mTexture0 != UINT32_MAX) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, mTexture0);
            }
            else {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, 0);
            }
            if (mTexture1 != UINT32_MAX) {
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, mTexture1);
            }
            else {
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, 0);
            }
            if (mTexture1 != UINT32_MAX) {
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, mTexture2);
            }
            else {
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, 0);
            }
            if (mTexture1 != UINT32_MAX) {
                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, mTexture3);
            }
            else {
                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, 0);
            }

            auto uModel = program.GetUniformLocation("model");
            GL_CALL(glUniformMatrix4fv(uModel, 1, GL_FALSE, (const GLfloat*)&mTransform));

            GL_CALL(glBindVertexArray(mVAO));

            if (mIndicesBuffered) {
                GL_CALL(glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mIndicesCount), GL_UNSIGNED_INT, (void*)(0)));
            }
            else if (mIndicesBase32) {
                GL_CALL(glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mIndicesCount), GL_UNSIGNED_INT, mIndicesBase32));
            }
            else if (mIndicesBase16) {
                GL_CALL(glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mIndicesCount), GL_UNSIGNED_SHORT, mIndicesBase16));
            }
            GL_CALL(glBindVertexArray(0));
        }

        void Render::Create() {
            if (mVAO == GL_INVALID_VALUE) {
                glGenVertexArrays(1, &mVAO);
                GL_CALL(glGenBuffers(1, &mVBO));
                if (mIndicesBuffered) {
                    GL_CALL(glGenBuffers(1, &mEBO));
                }
            }
        }
    }
}



