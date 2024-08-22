#include "rendering/ui/UICreator.h"

#include <memory>

namespace l::ui {

    UIHandle<UIContainer> UICreator::CreateContainer(uint32_t flags, UIRenderType renderType, UIAlignH alignH, UIAlignV alignV, UILayoutH layoutH, UILayoutV layoutV) {
        std::unique_ptr<UIContainer> container = std::make_unique<UIContainer>(flags, renderType, alignH, alignV, layoutH, layoutV);

        std::string id = CreateUniqueId<UIContainer>();
        container->SetId(id);
        mContainers.insert({ id, std::move(container)});

        return UIHandle<UIContainer>{ id, mContainers.at(id).get() };
    }

    UIHandle<UISplit> UICreator::CreateSplit(uint32_t flags, bool horizontalSplit) {
        std::unique_ptr<UISplit> container = std::make_unique<UISplit>(flags, horizontalSplit);

        std::string id = CreateUniqueId<UISplit>();
        container->SetId(id);
        mContainers.insert({ id, std::move(container) });

        return UIHandle<UISplit>{ id, mContainers.at(id).get() };
    }
}
