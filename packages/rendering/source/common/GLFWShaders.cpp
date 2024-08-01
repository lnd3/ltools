#include "rendering/GLFWShaders.h"
#include "rendering/GLFWSupport.h"
#include "logging/LoggingAll.h"

#include "memory/Containers.h"

#include <functional>
#include <unordered_map>
#include <atomic>

namespace l {
    namespace rendering {
        std::unique_ptr<ShaderManager> CreateShaderManager() {
            return std::make_unique<ShaderManager>();
        }

        ShaderManager::ShaderManager()
            : mCurrentProgram({0}) {}

        ShaderManager::~ShaderManager() {
            for (auto it : mShaderPrograms) {
                it.second.unload();
            }
            mShaderPrograms.clear();
            mShaders.clear();
        }

        void ShaderManager::CreateShader(std::string_view name, std::string_view code, bool fragment) {
            if (!code.empty()) {
                Shader shader;
                shader.load(code, fragment);
                mShaders.emplace(name, std::move(shader));
            }
            else {
                mShaders.erase(name.data());
            }
        }

        void ShaderManager::CompileShader(std::string_view name) {
            auto it = mShaders.find(name.data());
            if (it != mShaders.end()) {
                it->second.compile();
                return;
            }
            ASSERT(false) << "Failed to find shader '" << name << "'";
        }

        void ShaderManager::CreateProgram(std::string_view name, std::string_view vertexShader, std::string_view fragmentShader) {
            ShaderProgram program;

            auto itVertex = mShaders.find(vertexShader.data());
            if (itVertex == mShaders.end()) {
                ASSERT(false) << "Failed to find vertex shader '" << vertexShader << "'";
                return;
            }
            auto itFragment = mShaders.find(fragmentShader.data());
            if (itFragment == mShaders.end()) {
                ASSERT(false) << "Failed to find fragment shader '" << fragmentShader << "'";
                return;
            }
            program.load(itVertex->second, itFragment->second);

            mShaderPrograms.emplace(name, std::move(program));
        }

        void ShaderManager::DeleteProgram(std::string_view name) {
            auto it= mShaderPrograms.find(name.data());
            if (it == mShaderPrograms.end()) {
                ASSERT(false) << "Failed to find program '" << name << "'";
                return;
            }
            it->second.unload();
            mShaderPrograms.erase(name.data());
        }

        void ShaderManager::CompileProgram(std::string_view name) {
            auto it = mShaderPrograms.find(name.data());
            if (it == mShaderPrograms.end()) {
                ASSERT(false) << "Failed to find program '" << name << "'";
                return;
            }
            it->second.compile();
        }

        void ShaderManager::UseProgram(std::string_view name) {
            auto it = mShaderPrograms.find(name.data());
            if (it == mShaderPrograms.end()) {
                ASSERT(false) << "Failed to find program '" << name << "'";
                return;
            }
            it->second.use();

            mCurrentProgram = it->second;
        }

        ShaderProgram& ShaderManager::GetCurrentProgram() {
            return mCurrentProgram;
        }

        const GLuint ShaderManager::GetProgramId(std::string_view name) {
            auto it = mShaderPrograms.find(name.data());
            if (it == mShaderPrograms.end()) {
                ASSERT(false) << "Failed to find program '" << name << "'";
                return 0;
            }
            return it->second.mId;
        }

        void ShaderManager::UpdateMatrices4x4(const mat4x4 matrices, size_t count, std::string_view uniformName) {
            auto uModel = mCurrentProgram.GetUniformLocation(uniformName);
            GL_CALL(glUniformMatrix4fv(uModel, static_cast<GLsizei>(count), GL_FALSE, (const GLfloat*)matrices));
        }

        void ShaderManager::UpdateVectors4(const vec4 vectors, size_t count, std::string_view uniformName) {
            auto uModel = mCurrentProgram.GetUniformLocation(uniformName);
            GL_CALL(glUniform4fv(uModel, static_cast<GLsizei>(count), (const GLfloat*)vectors));
        }

        void ShaderManager::UpdateModel(const mat4x4 model) {
            auto uModel = mCurrentProgram.GetUniformLocation("model");
            GL_CALL(glUniformMatrix4fv(uModel, 1, GL_FALSE, (const GLfloat*)model));
        }

        void ShaderManager::UpdateView(const mat4x4 view) {
            auto uView = mCurrentProgram.GetUniformLocation("view");
            GL_CALL(glUniformMatrix4fv(uView, 1, GL_FALSE, (const GLfloat*)view));
        }

        void ShaderManager::UpdateProjection(const mat4x4 projection) {
            auto uProjection = mCurrentProgram.GetUniformLocation("projection");
            GL_CALL(glUniformMatrix4fv(uProjection, 1, GL_FALSE, (const GLfloat*)projection));
        }

        void ShaderManager::UpdateViewPosition(const vec4 position) {
            auto uViewP = mCurrentProgram.GetUniformLocation("viewposition");
            GL_CALL(glUniform3fv(uViewP, 1, (const GLfloat*)position));
        }

        void ShaderManager::UpdateTime(const float time) {
            auto uTime = mCurrentProgram.GetUniformLocation("time");
            GL_CALL(glUniform1fv(uTime, 1, (const GLfloat*)&time));
        }

    }
}
