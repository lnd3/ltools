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

#include "rendering/ui/UIContainer.h"
#include "nodegraph/core/NodeGraphBase.h"

#include <functional>

namespace l::ui {

    UIHandle CreateUINode(UIManager& uiManager, l::nodegraph::NodeGraphBase& node, ImVec2 p);
}
