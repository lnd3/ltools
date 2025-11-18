#pragma once

#include <string_view>

namespace l::ui {

    class UIModule {
    public:
        UIModule() = default;
        virtual ~UIModule() = default;

        virtual void UIModuleRenderControls(bool widget) = 0;
        virtual void UIModuleRenderChart(std::string_view plotKey) = 0;
    };
}
