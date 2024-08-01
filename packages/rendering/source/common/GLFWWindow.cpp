#include "rendering/GLFWWindow.h"
#include "logging/LoggingAll.h"

#include "memory/Containers.h"

#include <functional>
#include <mutex>
#include <unordered_map>
#include <atomic>
#include <memory>


#define GLAD_GL_IMPLEMENTATION
#include <glad/glad.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>


namespace {

    void glfw_error_callback(int error, const char* description) {
        LOG(LogError) << "GLFW Error " << error << ":" << description << "\n";
    }

    struct Callbacks {
        std::function<l::rendering::KeyCB> key;
        std::function<l::rendering::MouseCB> mouse;
        std::function<l::rendering::ScrollCB> scroll;
    };

    std::unordered_map<GLFWwindow*, Callbacks> sCallbacks;
    std::atomic_int sGLFWWindowCount = 0;

    void set_callbacks(GLFWwindow* window, std::function<l::rendering::KeyCB> key = nullptr, 
        std::function<l::rendering::MouseCB> mouse = nullptr, 
        std::function<l::rendering::ScrollCB> scroll = nullptr) {
        if (key || mouse || scroll) {
            Callbacks cb;
            cb.key = std::move(key);
            cb.mouse = std::move(mouse);
            cb.scroll = std::move(scroll);
            sCallbacks.emplace(window, std::move(cb));
        }
        else {
            sCallbacks.erase(window);
        }
    }

    static void invoke_key(GLFWwindow* window, int key, int scancode, int action, int mods) {
        auto it = sCallbacks.find(window);
        if (it != sCallbacks.end() && it->second.key) {
            it->second.key(window, key, scancode, action, mods);
        }
        else {
            if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }
        }
    }

    static void invoke_mouse(GLFWwindow* window, int button, int action, int mods) {
        auto it = sCallbacks.find(window);
        if (it != sCallbacks.end() && it->second.mouse) {
            it->second.mouse(window, button, action, mods);
        }
    }

    static void invoke_scroll(GLFWwindow* window, double xoffset, double yoffset) {
        auto it = sCallbacks.find(window);
        if (it != sCallbacks.end() && it->second.scroll) {
            it->second.scroll(window, static_cast<float>(xoffset), static_cast<float>(yoffset));
        }
    }
}

namespace l {
    namespace rendering {

        std::unique_ptr<GLFWWindowHandle> CreateGLFW(
            std::string_view name, 
            int width, 
            int height, 
            bool fullscreen, 
            bool invisible, 
            GLFWwindow* share
        ) {
            auto instance = std::make_unique<GLFWWindowHandle>(name, width, height, fullscreen, invisible, share);
            if (instance->IsValid()) {
                return instance;
            }
            return nullptr;
        }

        GLFWWindowHandle::GLFWWindowHandle(std::string_view name, 
            int width, 
            int height, 
            bool fullscreen,
            bool invisible,
            GLFWwindow* share) {
            auto construct = [=]() noexcept -> GLFWwindow* {

                sGLFWWindowCount++;

                // Safe to call even if glfw is already initialized
                glfwSetErrorCallback(glfw_error_callback);
                if (!glfwInit()) {
                    LOG(LogError) << "Failed to initialize GLFW";
                    return nullptr;
                }

                glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
                glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
                glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

                GLFWwindow* window = nullptr;

                int monitorCount = 0;
                glfwGetMonitors(&monitorCount);

                if (monitorCount == 0) {
                    return nullptr;
                }

                if (invisible) {
                    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
                }

                if (fullscreen) {
                    auto monitor = glfwGetPrimaryMonitor();

                    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

                    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
                    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
                    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
                    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
                    glfwWindowHint(GLFW_DEPTH_BITS, mode->refreshRate);

                    window = glfwCreateWindow(mode->width, mode->height, name.data(), monitor, share);
                }
                else {
                    window = glfwCreateWindow(width, height, name.data(), nullptr, share);
                }

                if (window == nullptr) {
                    LOG(LogError) << "Failed to create GLFW window";
                    return nullptr;
                }

                glfwMakeContextCurrent(window);

                LOG(LogInfo) << "GLFW version:" << glfwGetVersionString();

                if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
                    LOG(LogInfo) << "Failed to load OpenGL procedures";
                    return nullptr;
                }

                LOG(LogInfo) << "OpenGL version:" << glGetString(GL_VERSION);

                glfwSetKeyCallback(window, invoke_key);
                glfwSetMouseButtonCallback(window, invoke_mouse);
                glfwSetScrollCallback(window, invoke_scroll);

                glfwSwapInterval(1); // Enable vsync

                return window;
            };

            auto destruct = [](GLFWwindow* handle) noexcept {
                if (handle) {
                    glfwDestroyWindow(handle);
                    if (--sGLFWWindowCount == 0) {
                        glfwTerminate();
                    }
                }
            };
            mHandle = memory::Handle<GLFWwindow>(construct, destruct);
        }

        GLFWWindowHandle::~GLFWWindowHandle() {
            set_callbacks(mHandle.get());
        }

        bool GLFWWindowHandle::IsValid() {
            return mHandle();
        }

        GLFWwindow* GLFWWindowHandle::get() {
            return mHandle.get();
        }

        // setup
        void GLFWWindowHandle::SetInput(
            std::function<KeyCB> keyCallback, 
            std::function<MouseCB> mouseCallback, 
            std::function<ScrollCB> scrollCallback) {
            if (!IsValid()) {
                return;
            }
            set_callbacks(mHandle.get(), std::move(keyCallback), std::move(mouseCallback), std::move(scrollCallback));

            SetMouseMode();
            
            auto [w, h] = GetFrameBufferSize();
            glfwSetCursorPos(mHandle.get(), w / 2, h / 2);
        }

        void GLFWWindowHandle::SetOpacity(float opacity) {
            if (!IsValid()) {
                return;
            }
            glfwSetWindowOpacity(mHandle.get(), opacity);
        }

        // control
        bool GLFWWindowHandle::ShouldClose() {
            if (!IsValid()) {
                return true;
            }
            return glfwWindowShouldClose(mHandle.get());
        }

        void GLFWWindowHandle::SwapBuffers() {
            if (!IsValid()) {
                return;
            }
            glfwSwapBuffers(mHandle.get());
        }

        void GLFWWindowHandle::PollEvents() {
            glfwPollEvents();
        }

        // state
        std::tuple<int, int> GLFWWindowHandle::GetFrameBufferSize() {
            int w = 0;
            int h = 0;
            if (IsValid()) {
                glfwGetFramebufferSize(mHandle.get(), &w, &h);
            }
            return { w, h };
        }

        float GLFWWindowHandle::GetFrameBufferRatio() {
            auto [w, h] = GetFrameBufferSize();
            if (h > 0) {
                return w / (float)h;
            }
            return 0.0f;
        }

        void GLFWWindowHandle::Update() {
            double x, y;
            glfwGetCursorPos(mHandle.get(), &x, &y);

            if (mMouseAbsoluteCoords) {
                mMouseCoords[0] = static_cast<float>(x);
                mMouseCoords[1] = static_cast<float>(y);
            }
            else {
                auto [w, h] = GetFrameBufferSize();

                mMouseChange[0] = w / 2 - static_cast<float>(x);
                mMouseChange[1] = h / 2 - static_cast<float>(y);

                glfwSetCursorPos(mHandle.get(), w / 2, h / 2);
            }
        }

        std::tuple<float, float> GLFWWindowHandle::GetMouseChange() {
            if (mMouseAbsoluteCoords) {
                return { mMouseCoords[0], mMouseCoords[1] };
            }
            else {
                return { mMouseChange[0], mMouseChange[1] };
            }
        }

        void GLFWWindowHandle::SetMouseMode(bool absoluteCoords, bool visibility) {
            mMouseAbsoluteCoords = absoluteCoords;
            mMouseVisible = visibility;
            glfwSetInputMode(mHandle.get(), GLFW_CURSOR, visibility ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);

            if (absoluteCoords) {
                glfwSetCursorPos(mHandle.get(), mMouseCoords[0], mMouseCoords[1]);
            }
            else {
                auto [w, h] = GetFrameBufferSize();
                glfwSetCursorPos(mHandle.get(), w / 2, h / 2);
                vec2_clear(mMouseChange);
            }

            // reset state
            GetMouseChange();
        }

        // rendering
        void GLFWWindowHandle::Clear(int bufferBits) {
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(bufferBits);
        }

        void GLFWWindowHandle::SetViewPort(int x, int y, int width, int height) {
            if (width - x <= 0 || height - y <= 0) {
                auto [w, h] = GetFrameBufferSize();
                glViewport(0, 0, w, h);
                return;
            }
            glViewport(x, y, width, height);
        }
    }
}
