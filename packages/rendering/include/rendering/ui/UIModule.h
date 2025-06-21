#pragma once

#include <string_view>

#include <rendering/ui/UIBase.h>

namespace l::ui {

    class UIModule : public UIBase {
    public:
        UIModule() = default;
        virtual ~UIModule() = default;

        virtual void UIModuleRenderControls(bool widget) = 0;
        virtual void UIModuleRenderChart(std::string_view plotKey, float candleWidth) = 0;
    };
}
