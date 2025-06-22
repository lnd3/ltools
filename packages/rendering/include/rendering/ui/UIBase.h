#pragma once

#include <string_view>
#include <memory>
#include <vector>
#include <functional>

namespace l::ui {

    class UIBase {
    public:
        UIBase() = default;
        virtual ~UIBase() = default;

        virtual void Open() = 0;
        virtual void Show() = 0;
        virtual bool IsShowing() = 0;
        virtual void Update(double, float) {}
        virtual std::string_view GetName() const = 0;
    };

    void UIAdopt(std::unique_ptr<l::ui::UIBase>&& ui);
    void UIErase(l::ui::UIBase* ui);
    void UIErase(std::string_view name);
    void UIHouseKeeping();
    UIBase* UIGet(std::string_view name);
    bool UIHas(std::string_view name);
    void UIGetAll(std::vector<l::ui::UIBase*>& list);
    void UIForeach(std::function<bool(std::unique_ptr<l::ui::UIBase>&)> cb);

    template<class T>
    T* UIGetType() {
        T* theUI = nullptr;
        UIForeach([&](std::unique_ptr<l::ui::UIBase>& ui) {
            T* p = dynamic_cast<T*>(ui.get());
            if (p != nullptr) {
                theUI = p;
                return false;
            }
            });
        return theUI;
    }

}
