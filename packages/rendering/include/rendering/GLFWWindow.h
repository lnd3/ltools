#pragma once

#include "logging/LoggingAll.h"
#include "memory/Handle.h"
#include "various/linmathext.h"

#include <string_view>
#include <memory>
#include <functional>
#include <optional>
#include <tuple>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace l {
namespace rendering {

    using KeyCB = void(GLFWwindow* window, int key, int scancode, int action, int mods);
    using MouseCB = void(GLFWwindow* window, int button, int action, int mods);
    using ScrollCB = void(GLFWwindow* window, float xoffset, float yoffset);

    class GLFWWindowHandle {
    public:
        GLFWWindowHandle(
            std::string_view name, 
            int width, 
            int height, 
            bool fullscreen = false, 
            bool invisible = false,
            GLFWwindow* parent = nullptr
        );
        ~GLFWWindowHandle();

        bool IsValid();
        GLFWwindow* get();

        // setup
        void SetInput(std::function<KeyCB> keyCallback = nullptr, 
            std::function<MouseCB> mouseCallback = nullptr, 
            std::function<ScrollCB> scrollCallback = nullptr);
        void SetOpacity(float opacity);

        // control
        bool ShouldClose();
        void SwapBuffers();
        void PollEvents();

        // state
        std::tuple<int, int> GetFrameBufferSize();
        float GetFrameBufferRatio();
        void Update();

        std::tuple<float, float> GetMouseChange();
        void SetMouseMode(bool absoluteCoords = true, bool visible = true);

        // rendering
        void SetViewPort(int x = 0, int y = 0, int w = 0, int h = 0);
        void Clear(int bufferBits);
    protected:
        memory::Handle<GLFWwindow> mHandle;
        vec2 mMouseChange{ 0.0f, 0.0f };
        vec2 mMouseCoords{ 0.0f, 0.0f };
        bool mMouseAbsoluteCoords = true;
        bool mMouseVisible = true;
    };

    std::unique_ptr<GLFWWindowHandle> CreateGLFW(
        std::string_view name, 
        int width, 
        int height, 
        bool fullscreen = false, 
        bool invisible = false, 
        GLFWwindow* share = nullptr
    );
}
}