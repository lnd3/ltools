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
    template<class T, class = std::enable_if_t<std::is_base_of_v<UIContainer, T>>>
    std::string CreateUniqueId() {
        static uint32_t mId = 0;
        std::string id;
        if constexpr (std::is_same_v<T, UIContainer>) {
            id += "UIContainer";
        }
        else if constexpr (std::is_same_v<T, UISplit>) {
            id += "UISplit";
        }
        else {
            id += "Unknown";
        }

        id += l::string::to_hex<uint32_t>(mId++, 4);

        return id;
    }

    class UICreator {
    public:
        UICreator() = default;
        ~UICreator() = default;

        UIHandle<UIContainer> CreateContainer(uint32_t flags, UIRenderType renderType = UIRenderType::Rect, UIAlignH alignH = UIAlignH::Left, UIAlignV alignV = UIAlignV::Top, UILayoutH layoutH = UILayoutH::Fixed, UILayoutV layoutV = UILayoutV::Fixed);
        UIHandle<UISplit> CreateSplit(uint32_t flags, UISplitMode splitMode = UISplitMode::AppendV, UILayoutH layoutH = UILayoutH::Fixed, UILayoutV layoutV = UILayoutV::Fixed);

    protected:
        std::unordered_map<std::string, std::unique_ptr<UIContainer>> mContainers;
        std::vector<UIVisitor> mVisitors;

    };

}
