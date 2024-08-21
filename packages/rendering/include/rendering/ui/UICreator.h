#pragma once

#include "logging/LoggingAll.h"
#include "rendering/ui/UIContainer.h"
#include "rendering/ui/UIVisitors.h"
#include "rendering/ui/UIWindow.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "implot/implot.h"
#include "implot/implot_internal.h"

#include <functional>

namespace l::ui {

    struct UIHandle {
        std::string mId;
        UIContainer* mContainer;
    };

    class UI {
    public:
        UI() = default;
        ~UI() = default;

        UIHandle CreateContainer(std::string_view name, uint32_t flags);
        UIHandle CreateSplit(std::string_view name, uint32_t flags, bool horizontalSplit = true);
        UIHandle CreateLayout(std::string_view name, uint32_t flags, UIAlignH alignH = UIAlignH::Left, UIAlignV alignV = UIAlignV::Top);

    protected:
        std::unordered_map<std::string, std::unique_ptr<UIContainer>> mContainers;
        std::vector<UIVisitor> mVisitors;

    };

}
