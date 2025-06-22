#include "rendering/ui/UIBase.h"

#include <mutex>

namespace {
    std::vector<std::unique_ptr<l::ui::UIBase>> mUIs;
    std::vector<std::string_view> mUIDeleted;
    std::mutex mUIsMutex;
}

namespace l::ui {

    void UIAdopt(std::unique_ptr<l::ui::UIBase>&& ui) {
        std::lock_guard<std::mutex> lock(mUIsMutex);
        mUIs.push_back(std::move(ui));
    }

    void UIErase(std::string_view name) {
        std::lock_guard<std::mutex> lock(mUIsMutex);
        mUIDeleted.push_back(name);
    }

    void UIErase(l::ui::UIBase* ui) {
        if (ui) {
            UIErase(ui->GetName());
        }
    }

    void UIHouseKeeping() {
        std::lock_guard<std::mutex> lock(mUIsMutex);
        std::erase_if(mUIs, [&](const std::unique_ptr<l::ui::UIBase>& windowPtr) {
            for (auto& e : mUIDeleted) {
                if (windowPtr->GetName() == e) {
                    return true;
                }
            }
            return false;
            });
        mUIDeleted.clear();
    }

    UIBase* UIGet(std::string_view name) {
        std::lock_guard<std::mutex> lock(mUIsMutex);
        for (auto& ui : mUIs) {
            if (ui) {
                if (ui->GetName() == name) {
                    return ui.get();
                }
            }
        }
        return nullptr;
    }

    bool UIHas(std::string_view name) {
        std::lock_guard<std::mutex> lock(mUIsMutex);
        for (auto& ui : mUIs) {
            if (ui) {
                if (ui->GetName() == name) {
                    return true;
                }
            }
        }
        return false;
    }

    void UIGetAll(std::vector<l::ui::UIBase*>& list) {
        list.clear();
        std::lock_guard<std::mutex> lock(mUIsMutex);
        for (auto& ui : mUIs) {
            if (ui) {
                list.push_back(ui.get());
            }
        }
    }

    void UIForeach(std::function<bool(std::unique_ptr<l::ui::UIBase>&)> cb) {
        std::lock_guard<std::mutex> lock(mUIsMutex);
        for (auto& ui : mUIs) {
            if (ui) {
                if (!cb(ui)) {
                    break;
                }
            }
        }
    }

}
