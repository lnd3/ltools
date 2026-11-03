#pragma once

#include "logging/LoggingAll.h"

#include "memory/Handle.h"
#include "rendering/GLFWSupport.h"
#include "various/linmathext.h"

#include <string_view>
#include <memory>
#include <functional>
#include <optional>
#include <tuple>

#define GLAD_GL_IMPLEMENTATION
#include <glad/glad.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace l {
namespace rendering {

    // Added alongside tessellation-control/tessellation-evaluation
    // stage support (ShaderProgram/ShaderManager below) - replaces the
    // old `bool fragment` (vertex/fragment only) parameter throughout
    // this file.
    enum class ShaderStage {
        Vertex,
        TessControl,
        TessEvaluation,
        Fragment,
    };

    struct Shader {
        GLuint mId;
        ShaderStage mStage;

        void load(std::string_view code, ShaderStage stage) {
            GLenum glStage = GL_VERTEX_SHADER;
            switch (stage) {
                case ShaderStage::Vertex: glStage = GL_VERTEX_SHADER; break;
                case ShaderStage::TessControl: glStage = GL_TESS_CONTROL_SHADER; break;
                case ShaderStage::TessEvaluation: glStage = GL_TESS_EVALUATION_SHADER; break;
                case ShaderStage::Fragment: glStage = GL_FRAGMENT_SHADER; break;
            }
            mId = glCreateShader(glStage);
            mStage = stage;

            const GLchar* codeBytes = static_cast<const GLchar*>(code.data());
            glShaderSource(mId, 1, &codeBytes, NULL);
        }

        void compile() {
            glCompileShader(mId);

            GL_ASSERT();
        }

        void unload() {
            glDeleteShader(mId);
            mId = 0;
        }
    };

    struct ShaderProgram {
        GLuint mId;
        GLuint mVertexShaderId;
        // Tessellation stages, per direct request ("continue with the
        // gpu tessellation" - Phase 0 of the scoped plan, see
        // superplan's P014 Log). 0 doubles as "stage not present" (not
        // a valid GL shader id) - every existing 2-stage program keeps
        // working unchanged, since `load()` below explicitly zeroes
        // both and `compile()`'s new blocks are gated on `!= 0`.
        GLuint mTessControlShaderId = 0;
        GLuint mTessEvaluationShaderId = 0;
        GLuint mFragmentShaderId;

        void load(const Shader& vertexShader, const Shader& fragmentShader) {
            mVertexShaderId = vertexShader.mId;
            mTessControlShaderId = 0;
            mTessEvaluationShaderId = 0;
            mFragmentShaderId = fragmentShader.mId;
        }

        void loadWithTessellation(const Shader& vertexShader, const Shader& tessControlShader, const Shader& tessEvaluationShader, const Shader& fragmentShader) {
            mVertexShaderId = vertexShader.mId;
            mTessControlShaderId = tessControlShader.mId;
            mTessEvaluationShaderId = tessEvaluationShader.mId;
            mFragmentShaderId = fragmentShader.mId;
        }

        void compile() {
            glCompileShader(mVertexShaderId);
            {
                GLint vertexStatus;
                glGetShaderiv(mVertexShaderId, GL_COMPILE_STATUS, &vertexStatus);
                if (vertexStatus != GL_TRUE)
                {
                    GLsizei logLength = 0;
                    GLchar message[1024];
                    glGetShaderInfoLog(mVertexShaderId, 1024, &logLength, message);
                    LLOG(LogError) << "Failed to compile vertex shader. Error: " << message;
                }
                GL_ASSERT();
            }

            if (mTessControlShaderId != 0)
            {
                glCompileShader(mTessControlShaderId);
                GLint tessControlStatus;
                glGetShaderiv(mTessControlShaderId, GL_COMPILE_STATUS, &tessControlStatus);
                if (tessControlStatus != GL_TRUE)
                {
                    GLsizei logLength = 0;
                    GLchar message[1024];
                    glGetShaderInfoLog(mTessControlShaderId, 1024, &logLength, message);
                    LLOG(LogError) << "Failed to compile tessellation control shader. Error: " << message;
                }
                GL_ASSERT();
            }

            if (mTessEvaluationShaderId != 0)
            {
                glCompileShader(mTessEvaluationShaderId);
                GLint tessEvaluationStatus;
                glGetShaderiv(mTessEvaluationShaderId, GL_COMPILE_STATUS, &tessEvaluationStatus);
                if (tessEvaluationStatus != GL_TRUE)
                {
                    GLsizei logLength = 0;
                    GLchar message[1024];
                    glGetShaderInfoLog(mTessEvaluationShaderId, 1024, &logLength, message);
                    LLOG(LogError) << "Failed to compile tessellation evaluation shader. Error: " << message;
                }
                GL_ASSERT();
            }

            glCompileShader(mFragmentShaderId);
            {
                GLint fragmentStatus;
                glGetShaderiv(mFragmentShaderId, GL_COMPILE_STATUS, &fragmentStatus);
                if (fragmentStatus != GL_TRUE)
                {
                    GLsizei logLength = 0;
                    GLchar message[1024];
                    glGetShaderInfoLog(mFragmentShaderId, 1024, &logLength, message);
                    LLOG(LogError) << "Failed to compile fragment shader. Error: " << message;
                }
                GL_ASSERT();
            }

            mId = glCreateProgram();
            glAttachShader(mId, mVertexShaderId);
            if (mTessControlShaderId != 0) glAttachShader(mId, mTessControlShaderId);
            if (mTessEvaluationShaderId != 0) glAttachShader(mId, mTessEvaluationShaderId);
            glAttachShader(mId, mFragmentShaderId);
            glLinkProgram(mId);
            {
                GLint linkStatus;
                glGetProgramiv(mId, GL_LINK_STATUS, &linkStatus);
                if (linkStatus != GL_TRUE)
                {
                    GLsizei logLength = 0;
                    GLchar message[1024];
                    glGetProgramInfoLog(mId, 1024, &logLength, message);

                    LLOG(LogError) << "Failed to link. Error: " << message;
                }
                GL_ASSERT();
            }

            glValidateProgram(mId);
            GL_ASSERT();
        }

        void use() {
            GL_CALL(glUseProgram(mId));
        }

        void unload() {
            glDeleteProgram(mId);
            mId = 0;
            mVertexShaderId = 0;
            mTessControlShaderId = 0;
            mTessEvaluationShaderId = 0;
            mFragmentShaderId = 0;
        }

        GLint GetUniformLocation(std::string_view name) {
            GLint location = glGetUniformLocation(mId, name.data());

            GL_ASSERT();
            
            //GLint uniformCount;
            //glGetProgramiv(mId, GL_ACTIVE_UNIFORMS, &uniformCount);

            ASSERT(location != GL_INVALID_OPERATION && location != GL_INVALID_VALUE) << "Failed to find uniform '" << name;
            return location;
        }

        GLint GetAttributeLocation(std::string_view name) {
            GLint location = glGetAttribLocation(mId, name.data());

            GL_ASSERT();

            //GLint attributeCount;
            //glGetProgramiv(mId, GL_ACTIVE_ATTRIBUTES, &attributeCount);

            ASSERT(location != GL_INVALID_OPERATION) << "Failed to find attribute '" << name;
            return location;
        }

    };

    class ShaderManager {
    public:
        ShaderManager();
        ~ShaderManager();

        void CreateShader(std::string_view name, std::string_view code, ShaderStage stage);
        void CompileShader(std::string_view name);
        void CreateProgram(std::string_view name, std::string_view vertexShader, std::string_view fragmentShader);
        // Per direct request ("continue with the gpu tessellation") -
        // a separate entry point rather than overloading CreateProgram,
        // so the existing 2-stage call sites need no disambiguation.
        void CreateTessellatedProgram(std::string_view name, std::string_view vertexShader, std::string_view tessControlShader, std::string_view tessEvaluationShader, std::string_view fragmentShader);
        void DeleteProgram(std::string_view name);
        void CompileProgram(std::string_view name);
        void UseProgram(std::string_view name);
        ShaderProgram& GetCurrentProgram();
        GLuint GetProgramId(std::string_view name);

        void UpdateMatrices4x4(const mat4x4 matrices, size_t count, std::string_view uniformName);
        void UpdateVectors4(const vec4 vectors, size_t count, std::string_view uniformName);
        void UpdateModel(const mat4x4 model);
        void UpdateView(const mat4x4 view);
        void UpdateProjection(const mat4x4 projection);
        void UpdateViewPosition(const vec4 position);
        void UpdateTime(const float time);

    protected:
        std::unordered_map<std::string, l::rendering::Shader> mShaders;
        std::unordered_map<std::string, l::rendering::ShaderProgram> mShaderPrograms;

        ShaderProgram mCurrentProgram{};
    };

    std::unique_ptr<ShaderManager> CreateShaderManager();
}
}