#pragma once

#include "logging/LoggingAll.h"

#include "rendering/ui/UIContainer.h"
#include "nodegraph/core/NodeGraphBase.h"

#include <functional>

namespace l::ui {

    UIHandle CreateUINode(UIManager& uiManager, l::nodegraph::NodeGraphBase& node, ImVec2 p);
}
