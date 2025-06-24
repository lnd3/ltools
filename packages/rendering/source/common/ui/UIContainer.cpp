#include "rendering/ui/UIContainer.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

#include <memory>

namespace l::ui {

    ImVec2 DragMovement(const ImVec2& prevPos, const ImVec2& curPos, float curScale) {
        ImVec2 move = curPos;
        move.x -= prevPos.x;
        move.y -= prevPos.y;
        move.x = move.x / curScale;
        move.y = move.y / curScale;
        return move;
    }

    bool Overlap(const ImVec2& p, const ImVec2& pMin, const ImVec2& pMax) {
        return pMin.x < p.x && pMin.y < p.y && pMax.x > p.x && pMax.y > p.y;
    }

    bool Overlap(const ImVec2& p, const ImVec2& pMin, const ImVec2& pMax, const ContainerArea& area) {
        ImVec2 pMinT = area.Transform(pMin);
        ImVec2 pMaxT = area.Transform(pMax);
        return pMinT.x < p.x && pMinT.y < p.y && pMaxT.x > p.x && pMaxT.y > p.y;
    }

    bool OverlapScreenRect(const ImVec2& p, const ImVec2& pCenter, const ImVec2& screenOffset, const ContainerArea& area) {
        ImVec2 pMinT = area.Transform(pCenter, ImVec2(-screenOffset.x, -screenOffset.y));
        ImVec2 pMaxT = area.Transform(pCenter, ImVec2(screenOffset.x, screenOffset.y));
        return pMinT.x < p.x && pMinT.y < p.y && pMaxT.x > p.x && pMaxT.y > p.y;
    }

    bool OverlapScreenCircle(const ImVec2& p, const ImVec2& pCenter, float radii, const ContainerArea& area) {
        ImVec2 pT = area.Transform(pCenter);
        ImVec2 d = ImVec2(pT.x - p.x, pT.y - p.y);
        return d.x * d.x + d.y * d.y < radii * radii;
    }

    bool OverlapCircle(const ImVec2& p, const ImVec2& pCenter, float radii) {
        ImVec2 d = ImVec2(pCenter.x - p.x, pCenter.y - p.y);
        return d.x * d.x + d.y * d.y < radii * radii;
    }

    void UIContainer::Add(UIContainer* container, int32_t i) {
        if (i < 0) {
            mContent.push_back(container);
        }
        else {
            ASSERT(static_cast<size_t>(i) < mContent.size());
            mContent.insert(mContent.begin() + i, container);
        }
        container->SetParent(this);
    }

    void UIContainer::Remove(int32_t i) {
        ASSERT(i >= 0 && static_cast<size_t>(i) < mContent.size());
        mContent.at(i)->SetParent(nullptr);
        mContent.erase(mContent.begin() + i);
    }

    void UIContainer::Remove(const UIHandle& handle) {
        for (auto it = mContent.begin(); it != mContent.end(); it++) {
            auto containerPtr = *it;
            if (containerPtr == handle.Get()) {
                mContent.erase(it);
                break;
            }
        }
    }

    void UIContainer::Remove(UIContainer* container) {
        for (auto it = mContent.begin(); it != mContent.end(); it++) {
            auto containerPtr = *it;
            if (containerPtr == container) {
                mContent.erase(it);
                break;
            }
        }
    }

    void UIContainer::RemoveAll() {
        mContent.clear();
    }

    void UIContainer::ForEachChild(bool recursive, std::function<void(UIContainer*)> cb) {
        for (auto it : mContent) {
            if (recursive) {
                ForEachChild(recursive, cb);
            }
            cb(it);
        }
    }

    void UIContainer::Move(ImVec2 localChange) {
        mDisplayArea.mPosition.x += localChange.x;
        mDisplayArea.mPosition.y += localChange.y;
    }

    void UIContainer::Resize(ImVec2 localChange) {
        mDisplayArea.mSize.x += localChange.x;
        mDisplayArea.mSize.y += localChange.y;
        if (mDisplayArea.mRender.mType != UIRenderType::LinkH) {
            mDisplayArea.mSize.x = mDisplayArea.mSize.x < 1.0f ? 1.0f : mDisplayArea.mSize.x;
            mDisplayArea.mSize.y = mDisplayArea.mSize.y < 1.0f ? 1.0f : mDisplayArea.mSize.y;
        }
    }

    ImVec2 UIContainer::GetPosition(bool untransformed) const {
        if (untransformed) {
            return mDisplayArea.mPosition;
        }
        return ImVec2(mDisplayArea.mPosition.x * mDisplayArea.mScale, mDisplayArea.mPosition.y * mDisplayArea.mScale);
    }

    ImVec2 UIContainer::GetPositionAtCenter(ImVec2 offset, bool untransformed) {
        if (untransformed) {
            return ImVec2(mDisplayArea.mPosition.x + mDisplayArea.mSize.x * 0.5f + offset.x, mDisplayArea.mPosition.y * 0.5f + mDisplayArea.mSize.y + offset.y);
        }
        return ImVec2((mDisplayArea.mPosition.x + mDisplayArea.mSize.x * 0.5f + offset.x) * mDisplayArea.mScale, (mDisplayArea.mPosition.y * 0.5f + mDisplayArea.mSize.y + offset.y) * mDisplayArea.mScale);
    }

    ImVec2 UIContainer::GetPositionAtSize(ImVec2 offset, bool untransformed) {
        if (untransformed) {
            return ImVec2(mDisplayArea.mPosition.x + mDisplayArea.mSize.x + offset.x, mDisplayArea.mPosition.y + mDisplayArea.mSize.y + offset.y);
        }
        return ImVec2((mDisplayArea.mPosition.x + mDisplayArea.mSize.x + offset.x) * mDisplayArea.mScale, (mDisplayArea.mPosition.y + mDisplayArea.mSize.y + offset.y) * mDisplayArea.mScale);
    }

    ImVec2 UIContainer::GetSize(bool untransformed) {
        if (untransformed) {
            return mDisplayArea.mSize;
        }
        return ImVec2(mDisplayArea.mSize.x * mDisplayArea.mScale, mDisplayArea.mSize.y * mDisplayArea.mScale);
    }

    bool UIContainer::Accept(UIVisitor& visitor, const InputState& input, UITraversalMode mode) {
        if (visitor.ShouldUpdateContainer()) {
            auto& layout = mDisplayArea.mLayout;
            switch (layout.mLayoutH) {
            case UILayoutH::Fixed:
                break;
            case UILayoutH::Scaled:
                break;
            case UILayoutH::Parent:
                mDisplayArea.mSize.x = mLayoutArea.GetLocalSize().x;
                break;
            }
            switch (layout.mLayoutV) {
            case UILayoutV::Fixed:
                break;
            case UILayoutV::Scaled:
                break;
            case UILayoutV::Parent:
                mDisplayArea.mSize.y = mLayoutArea.GetLocalSize().y;
                break;
            }
        }

        if (mode == UITraversalMode::BFS && visitor.Active(*this, input)) {
            if (visitor.Visit(*this, input)) {
                return true;
            }
        }

        size_t i = 0;
        for (auto& content : mContent) {
            if (visitor.ShouldUpdateContainer()) {
                auto contentSize = content->GetSize();
                auto& contentLayout = content->mDisplayArea.mLayout;
                ContainerArea current;
                current.mScale = mDisplayArea.GetWorldScale(mLayoutArea.mScale);
                current.mSize = mDisplayArea.GetWorldSizeLayout(mLayoutArea.mScale);
                current.mPosition = mDisplayArea.GetWorldPosLayout(mLayoutArea.mScale, mLayoutArea.mPosition, contentSize, contentLayout.mAlignH, contentLayout.mAlignV);
                content->SetLayoutArea(current);
            }

            if (content->Accept(visitor, input, mode)) {
                return true;
            }
            i++;
        }

        if (mode == UITraversalMode::DFS && visitor.Active(*this, input)) {
            return visitor.Visit(*this, input);
        }
        return false;
    }

    bool UISplit::Accept(UIVisitor& visitor, const InputState& input, UITraversalMode mode) {
        // Since we can have multiple layouts in a container for different content, it will act as
        // an anchor rather than a container, therefore it has to align within it and size 

        ContainerArea current;

        if (visitor.ShouldUpdateContainer()) {
            auto& layout = mDisplayArea.mLayout;
            switch (layout.mLayoutH) {
            case UILayoutH::Fixed:
                break;
            case UILayoutH::Scaled:
                break;
            case UILayoutH::Parent:
                mDisplayArea.mSize.x = mLayoutArea.GetLocalSize().x;
                break;
            }
            switch (layout.mLayoutV) {
            case UILayoutV::Fixed:
                break;
            case UILayoutV::Scaled:
                break;
            case UILayoutV::Parent:
                mDisplayArea.mSize.y = mLayoutArea.GetLocalSize().y;
                break;
            }

            float contentCount = static_cast<float>(mContent.size());
            current.mScale = mDisplayArea.GetWorldScale(mLayoutArea.mScale);
            current.mSize = mDisplayArea.GetWorldSize(mLayoutArea.mScale);
            current.mPosition = mDisplayArea.GetWorldPos(mLayoutArea.mScale, mLayoutArea.mPosition);

            switch (mSplitMode) {
            case UISplitMode::EqualSplitH:
                current.mSize.x /= contentCount;
                break;
            case UISplitMode::EqualSplitV:
                current.mSize.y /= contentCount;
                break;
            case UISplitMode::AppendH:
                break;
            case UISplitMode::AppendV:
                break;
            case UISplitMode::EqualResizeH:
                break;
            case UISplitMode::EqualResizeV:
                break;
            }
        }

        if (mode == UITraversalMode::BFS && visitor.Active(*this, input)) {
            if (visitor.Visit(*this, input)) {
                return true;
            }
        }

        size_t i = 0;
        for (auto& content : mContent) {
            if (visitor.ShouldUpdateContainer()) {
                content->SetLayoutArea(current);
            }

            if (content->Accept(visitor, input, mode)) {
                return true;
            }

            if (visitor.ShouldUpdateContainer()) {
                switch (mSplitMode) {
                case UISplitMode::EqualSplitH:
                    current.mPosition.x += current.mSize.x;
                    break;
                case UISplitMode::EqualSplitV:
                    current.mPosition.y += current.mSize.y;
                    break;
                case UISplitMode::AppendH:
                    current.mPosition.x += content->GetSize().x * mLayoutArea.mScale;
                    current.mSize.x -= content->GetSize().x * mLayoutArea.mScale;
                    break;
                case UISplitMode::AppendV:
                    current.mPosition.y += content->GetSize().y * mLayoutArea.mScale;
                    current.mSize.y -= content->GetSize().y * mLayoutArea.mScale;
                    break;
                case UISplitMode::EqualResizeH:
                    break;
                case UISplitMode::EqualResizeV:
                    break;
                }
            }
            i++;
        }

        if ((mode == UITraversalMode::DFS) && visitor.Active(*this, input)) {
            return visitor.Visit(*this, input);
        }
        return false;
    }

    void UIManager::Reset() {
        mContainers.clear();
    }

    UIHandle UIManager::Add(std::unique_ptr<UIContainer> container) {
        auto id = mIdCounter++;
        auto stringId = container->GetStringId();
        container->SetId(id);
        mContainers.insert({ id, std::move(container) });
        return UIHandle{ id, stringId, mContainers.at(id).get() };
    }

    void UIManager::Remove(const UIHandle& handle) {
        if (handle.IsValid()) {
            mContainers.erase(handle.GetId());
        }
    }

    void UIManager::Remove(UIContainer* container) {
        if (container != nullptr) {
            mContainers.erase(container->GetId());
        }
    }

    UIContainer* UIManager::FindNodeId(uint32_t flag, int32_t nodeId, int32_t channelId) {
        for (auto& c : mContainers) {
            if (c.second->HasConfigFlag(flag) && c.second->GetNodeId() == nodeId && (channelId == -1 || c.second->GetChannelId() == channelId)) {
                return c.second.get();
            }
        }
        return nullptr;
    }

    UIHandle CreateContainer(UIManager& uiManager, uint32_t flags, UIRenderType renderType, UIAlignH alignH, UIAlignV alignV, UILayoutH layoutH, UILayoutV layoutV) {
        std::unique_ptr<UIContainer> container = std::make_unique<UIContainer>(flags, renderType, alignH, alignV, layoutH, layoutV);

        auto stringId = CreateUniqueStringId<UIContainer>();
        container->SetStringId(stringId);

        return uiManager.Add(std::move(container));
    }

    UIHandle CreateSplit(UIManager& uiManager, uint32_t flags, UIRenderType renderType, UISplitMode splitMode, UILayoutH layoutH, UILayoutV layoutV) {
        std::unique_ptr<UISplit> container = std::make_unique<UISplit>(flags, renderType, splitMode, layoutH, layoutV);

        auto stringId = CreateUniqueStringId<UIContainer>();
        container->SetStringId(stringId);

        return uiManager.Add(std::move(container));
    }

    void DeleteContainer(UIManager& uiManager, UIHandle handle, bool alsoRemoveFromParent) {
        if (handle.IsValid()) {
            DeleteContainer(uiManager, handle.Get(), alsoRemoveFromParent);
        }
    }

    void DeleteContainer(UIManager& uiManager, UIContainer* container, bool alsoRemoveFromParent) {
        if (container != nullptr) {
            if (container->HasConfigFlag(UIContainer_InputFlag)) {
                auto inputContainer = container;

                // check for link and remove it as well
                auto linkContainer = inputContainer->GetCoParent();
                if (linkContainer != nullptr && linkContainer->HasConfigFlag(UIContainer_LinkFlag)) {
                    DeleteContainer(uiManager, linkContainer, true); // manually trigger removal from parent since it is removed externally from hierarchy
                }
            }
            else if (container->HasConfigFlag(UIContainer_LinkFlag)) {
                // check for link and remove it as well
                auto linkContainer = container;

                auto inputContainer = linkContainer->GetCoParent();
                if (inputContainer != nullptr && inputContainer->HasConfigFlag(UIContainer_InputFlag)) {
                    inputContainer->SetCoParent(nullptr);
                }
            }

            // remove all of its children
            container->ForEachChild(false, [&](UIContainer* c) {
                DeleteContainer(uiManager, c, false);
                });

            // We also remove the container from its parent if its the first 
            // DeleteContainer call beacuse otherwise the parent is also being removed 
            // in which case it doesn't matter since its also not around.
            // The only edge case is when a container is being removed externally
            // from its parent hierarchy in which case it is not deleted by its parent.
            // This currently only happens when a link container is being removed or 
            // indirectly by removing an input container
            if (alsoRemoveFromParent) {
                ASSERT(container->GetParent() != nullptr);
                container->GetParent()->Remove(container);
            }

            // finally remove the container itself
            uiManager.Remove(container);
        }
    }

    void ForEachChildOf(uint32_t flag, UIContainer* rootContainer, bool recursive, std::function<bool(UIContainer*)> cb) {
        rootContainer->ForEachChild(false, [&](UIContainer* c) {
            if (c->HasConfigFlag(flag)) {
                if (!cb(c)) {
                    return;
                }
            }
            if (recursive) {
                ForEachChildOf(flag, c, recursive, cb);
            }
            });
    }

}
