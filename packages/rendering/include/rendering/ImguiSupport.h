#pragma once

#include "logging/LoggingAll.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "implot/implot.h"
#include "implot/implot_internal.h"

#include <functional>

namespace l {
namespace rendering {

    class GLFWImguiHandle {
    public:
        using ImguiHandler = std::function<void(GLFWImguiHandle&)>;

        GLFWImguiHandle(
            GLFWwindow* parent = nullptr
        );
        ~GLFWImguiHandle();

        void SetGuiBuilder(ImguiHandler builder);
        void Render();
    protected:
        GLFWwindow* mParent;
        ImguiHandler mBuilder;
    };

    std::unique_ptr<GLFWImguiHandle> CreateImgui(
        GLFWwindow* parent = nullptr
    );
}
}