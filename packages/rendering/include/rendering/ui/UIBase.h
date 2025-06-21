#pragma once

namespace l::ui {

    class UIBase {
    public:
        UIBase() = default;
        virtual ~UIBase() = default;

        virtual void Show() = 0;
        virtual bool IsShowing() = 0;
        virtual void Update(double, float) {}
    };
}
