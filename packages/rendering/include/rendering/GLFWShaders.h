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

    struct Shader {
        GLuint mId;
        bool mFragment;

        void load(std::string_view code, bool fragment) {
            mId = fragment ? glCreateShader(GL_FRAGMENT_SHADER) : glCreateShader(GL_VERTEX_SHADER);
            mFragment = fragment;

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
        GLuint mFragmentShaderId;

        void load(const Shader& vertexShader, const Shader& fragmentShader) {
            mVertexShaderId = vertexShader.mId;
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
                    LOG(LogError) << "Failed to compile vertex shader. Error: " << message;
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
                    LOG(LogError) << "Failed to compile fragment shader. Error: " << message;
                }
                GL_ASSERT();
            }

            mId = glCreateProgram();
            glAttachShader(mId, mVertexShaderId);
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

                    LOG(LogError) << "Failed to link. Error: " << message;
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

        void CreateShader(std::string_view name, std::string_view code, bool fragment);
        void CompileShader(std::string_view name);
        void CreateProgram(std::string_view name, std::string_view vertexShader, std::string_view fragmentShader);
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