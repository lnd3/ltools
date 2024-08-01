#pragma once

#include <memory>

#include "rendering/GLFWSupport.h"
#include "rendering/GLFWShaders.h"
#include "various/linmathext.h"
#include "filesystem/File.h"

#include "ufbx/ufbx.h"
#include "openfbx/src/ofbx.h"
#include "rendering/ColladaLoader.h"

#include <type_traits>
#include <memory>


//
// 
// When Mesh is Constructed:
// -Generate VAO, VBO, IBO
//    - Bind     VAO, VBO, IBO
//
//    ->Upload Vertex Data to VBO
//    ->Upload Index Array to IBO
//
//    Foreach Attribute <n>
//    -Setup  Attrib Pointer(n)
//    - Enable Attrib Array(n)
//    End Foreach
//    Stage 2: Drawing a Mesh Instance
//
//    When an Object(instance of Mesh) is Rendered :
//
// -Bind Mesh's VAO
//
//    - Bind program / shader(id)
//    ->send uniforms
//
//    ->glDrawElements
//
// https://stackoverflow.com/questions/18403708/crash-on-gldrawelements


namespace l {
namespace rendering {

    struct Attribute {
        void* mData = nullptr; // attribute pointer to attribute array
        size_t mSize = 0; // number of innertypes in attribute, usually 1,2,3 or 4.
        size_t mStride = 0; // byte offset between consecutive vertex attributes. distance in bytes to next attribute
        std::string mAttributeName = "";
        size_t mTotalSizeInBytes = 0; // size of attribute array in bytes
    };

    template<class InnerType, size_t InnerTypeCount>
    Attribute CreateAttribute(InnerType* data = nullptr, size_t size = 0, std::string_view mAttributeName = "") {
        Attribute attribute;
        attribute.mData = data; // attribute pointer to attribute array
        attribute.mSize = InnerTypeCount;
        attribute.mAttributeName = mAttributeName;
        attribute.mTotalSizeInBytes = size * sizeof(InnerType); // size of attribute array in bytes
        attribute.mStride = InnerTypeCount * sizeof(InnerType);
        return attribute;
    }

    size_t GetAttributeTotalStride(const std::vector<Attribute>& attributes);
    size_t GetAttributeTotalSizeInBytes(const std::vector<Attribute>& attributes);

    class IRender {
    public:
        virtual ~IRender() {}

        virtual void UpdateModel(mat4x4 transform) = 0;
        virtual void Upload() = 0;
        virtual void Configure(ShaderProgram& program) = 0;
        virtual void SetTexture(uint32_t index, GLuint texture) = 0;
        virtual void Draw(ShaderProgram& program) = 0;
    };

    class Render : public IRender {
    public:
        Render();
        virtual ~Render();

        virtual void UpdateModel(mat4x4 transform);
        virtual void Upload();
        virtual void Configure(ShaderProgram& program);
        virtual void SetTexture(uint32_t index, GLuint texture);
        virtual void Draw(ShaderProgram& program);

    protected:
        void Create();

        uint32_t mVAO = GL_INVALID_VALUE;
        uint32_t mVBO = GL_INVALID_VALUE;
        uint32_t mEBO = GL_INVALID_VALUE;

        bool mDirty = true;
        mat4x4 mTransform{0};

        void* mIndicesBase = nullptr;
        void* mIndicesBase16 = nullptr;
        void* mIndicesBase32 = nullptr;
        uint32_t mIndicesCount = 0;
        int32_t mIndicesAttribSize = 0;
        bool mIndicesBuffered = true;
        bool mInterleaved = false;
        Attribute mInterleavedAttribute{0};

        std::vector<Attribute> mAttributes;

        GLuint mTexture0 = UINT32_MAX;
        GLuint mTexture1 = UINT32_MAX;
        GLuint mTexture2 = UINT32_MAX;
        GLuint mTexture3 = UINT32_MAX;
    };

    class RenderVAO : public Render {
    public:

        RenderVAO() : Render() {}

        virtual ~RenderVAO() = default;

        template<class T>
        void SetIndices(bool buffered, T* indices, size_t count) {
            if (indices == nullptr || count == 0) {
                return;
            }

            mIndicesBase = &indices[0];
            mIndicesCount = static_cast<GLuint>(count);
            mIndicesAttribSize = sizeof(T);
            mIndicesBuffered = buffered;

            if (!mIndicesBuffered) {
                if constexpr (sizeof(T) == 2) {
                    mIndicesBase16 = mIndicesBase;
                }
                else if constexpr (sizeof(T) == 4) {
                    mIndicesBase32 = mIndicesBase;
                }
            }
        }

        template<class T, size_t stride = 3>
        void SetInterleaved(T* vertices, size_t count) {
            if (vertices == nullptr || count == 0) {
                return;
            }

            mInterleaved = true;
            mInterleavedAttribute = CreateAttribute<T, stride>(&vertices[0], count);
        }

        template<class T = float, size_t stride = 3>
        void SetPositions3D(T* vertices = nullptr, size_t count = 0) {
            if (!mInterleaved && vertices == nullptr || count == 0) {
                return;
            }
            mAttributes.emplace_back(CreateAttribute<T, stride>(&vertices[0], count, "vPosition"));
        }

        template<class T = float, size_t stride = 3>
        void SetNormals3D(T* normals = nullptr, size_t count = 0) {
            if (normals != nullptr && count > 0) {
                mAttributes.emplace_back(CreateAttribute<T, stride>(&normals[0], count, "vNormal"));
            }
        }

        template<class T = float, size_t stride = 4>
        void SetColors(T* colors = nullptr, size_t count = 0) {
            if (colors != nullptr && count > 0) {
                mAttributes.emplace_back(CreateAttribute<T, stride>(&colors[0], count, "vColor"));
            }
        }

        template<class T = float, size_t stride = 2>
        void SetUVs2D(T* uvs = nullptr, size_t count = 0) {
            if (uvs != nullptr && count > 0) {
                mAttributes.emplace_back(CreateAttribute<T, stride>(&uvs[0], count, "vUV"));
            }
        }

        template<class T = float, size_t stride = 3>
        void SetTangents3D(T* tangents = nullptr, size_t count = 0) {
            if(tangents != nullptr && count > 0){
                mAttributes.emplace_back(CreateAttribute<T, stride>(&tangents[0], count, "vTangent"));
            }
        }

    };

    std::unique_ptr<RenderVAO> CreateRenderVAO();

    std::unique_ptr<RenderVAO> CreateUFBXMesh(l::rendering::ShaderProgram& program,
        const ufbx_mesh& mesh,
        bool saveData = false,
        bool useNormals = true,
        bool useColors = true,
        bool useUVs = true,
        bool useTangents = false
    );

    std::unique_ptr<RenderVAO> CreateOFBXMesh(l::rendering::ShaderProgram& program,
        const ofbx::Mesh& geometry,
        bool saveData = false,
        bool useNormals = true,
        bool useColors = true,
        bool useUVs = true
    );

    std::unique_ptr<RenderVAO> CreateColladaMesh(l::rendering::ShaderProgram& program,
        const l::rendering::GeometryNode& geometry,
        bool saveData = false,
        bool useNormals = true,
        bool useColors = true,
        bool useUVs = true
    );

    std::unique_ptr<ufbx_scene> LoadUFBXMesh(std::string_view filename);

    template<class T>
    void SaveGeometryData(std::string filename, std::string namepostfix, std::vector<T> data, size_t stride) {
        l::filesystem::File f0("./" + filename + "_" + namepostfix + ".txt");
        f0.modeWriteTrunc();
        if (f0.open()) {
            std::stringstream stream;
            for (int i = 0; i < data.size() / stride; i++) {
                stream << "{";
                for (int j = 0; j < stride; j++) {
                    stream << std::to_string(data[i * stride + j]);
                    if (j < stride - 1) {
                        stream << ", ";
                    }
                }
                stream << "}";
                if (i % 3 == 2) {
                    stream << "\n";
                }
            }
            f0.write(stream);
        }
        };

}
}